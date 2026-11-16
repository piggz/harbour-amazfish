#include "pebbleppogattservice.h"
#include "qble/qblelocalcharacteristic.h"

#include <QDebug>
#include <QtEndian>

const char* PebblePPoGATTService::UUID_SERVICE_PPOGATT = "10000000-328e-0fbb-c642-1aa6699bdada";
const char* PebblePPoGATTService::UUID_CHARACTERISTIC_DATA = "10000001-328e-0fbb-c642-1aa6699bdada";
const char* PebblePPoGATTService::UUID_CHARACTERISTIC_META = "10000002-328e-0fbb-c642-1aa6699bdada";

// The watch writes packets without response; the phone sends packets as notifications.
class PebbleDataCharacteristic : public QBLELocalCharacteristic {
public:
    PebbleDataCharacteristic(QDBusConnection bus, unsigned int index, QBLELocalService* service)
        : QBLELocalCharacteristic(bus, index, QString::fromLatin1(PebblePPoGATTService::UUID_CHARACTERISTIC_DATA),
              { QStringLiteral("write-without-response"), QStringLiteral("notify") }, service, service)
    {
    }

    void WriteValue(const QByteArray& value, const QVariantMap& options) override
    {
        // BlueZ passes the ATT MTU of the link with each write
        if (options.contains(QStringLiteral("mtu"))) {
            mtu = options.value(QStringLiteral("mtu")).toInt();
        }
        QBLELocalCharacteristic::WriteValue(value, options);
    }

    int mtu = 23;
};

// Session parameters the watch reads before it opens PPoGATT.
class PebbleMetaCharacteristic : public QBLELocalCharacteristic {
public:
    PebbleMetaCharacteristic(QDBusConnection bus, unsigned int index, QBLELocalService* service)
        : QBLELocalCharacteristic(bus, index, QString::fromLatin1(PebblePPoGATTService::UUID_CHARACTERISTIC_META),
              { QStringLiteral("read") }, service, service)
    {
    }

    QByteArray ReadValue(const QVariantMap&) override
    {
        // Minimum PPoGATT version 0, maximum version 1, app UUID all zero (system session), session type 1
        QByteArray meta(19, '\0');
        meta[1] = 1;
        meta[18] = 1;
        return meta;
    }
};

// The official Android app and cobble add this service; cobble says the watch expects it.
class PebbleFakeCharacteristic : public QBLELocalCharacteristic {
public:
    PebbleFakeCharacteristic(QDBusConnection bus, QBLELocalService* service)
        : QBLELocalCharacteristic(bus, 0, QString::fromLatin1(PebbleFakeService::UUID_SERVICE_FAKE),
              { QStringLiteral("read") }, service, service)
    {
    }

    QByteArray ReadValue(const QVariantMap&) override
    {
        return QByteArray(1, '\0');
    }
};

const char* PebbleFakeService::UUID_SERVICE_FAKE = "badbadba-dbad-badb-adba-badbadbadbad";

PebbleFakeService::PebbleFakeService(QDBusConnection bus, const QString& appPath, QObject* parent)
    : QBLELocalService(bus, 1, QString::fromLatin1(UUID_SERVICE_FAKE), appPath, parent)
{
    addCharacteristic(new PebbleFakeCharacteristic(bus, this));
}

PebblePPoGATTService::PebblePPoGATTService(QDBusConnection bus, const QString& appPath, QObject* parent)
    : QBLELocalService(bus, 0, QString::fromLatin1(UUID_SERVICE_PPOGATT), appPath, parent)
{
    m_data = new PebbleDataCharacteristic(bus, 0, this);
    addCharacteristic(m_data);
    addCharacteristic(new PebbleMetaCharacteristic(bus, 1, this));

    connect(m_data, &QBLELocalCharacteristic::valueWritten, this, &PebblePPoGATTService::onPacket);
    connect(m_data, &QBLELocalCharacteristic::notifyingChanged, this, &PebblePPoGATTService::onNotifyingChanged);
}

bool PebblePPoGATTService::isOpen() const
{
    return m_open;
}

void PebblePPoGATTService::sendMessage(quint16 endpoint, const QByteArray& payload)
{
    if (!m_open) {
        qWarning() << Q_FUNC_INFO << "Session is not open, dropping message for endpoint" << endpoint;
        return;
    }

    QByteArray message(4, '\0');
    qToBigEndian<quint16>(payload.size(), reinterpret_cast<uchar*>(message.data()));
    qToBigEndian<quint16>(endpoint, reinterpret_cast<uchar*>(message.data() + 2));
    message += payload;
    qDebug() << Q_FUNC_INFO << "endpoint" << endpoint << message.toHex();

    // One header byte per packet, 3 bytes of ATT header per notification
    int chunkSize = m_data->mtu - 4;
    for (int pos = 0; pos < message.size(); pos += chunkSize) {
        m_queue.append(message.mid(pos, chunkSize));
    }
    sendQueuedPackets();
}

void PebblePPoGATTService::onPacket(const QByteArray& packet)
{
    if (packet.isEmpty()) {
        return;
    }

    int serial = static_cast<quint8>(packet.at(0)) >> 3;
    int type = packet.at(0) & 7;

    switch (type) {
    case PacketData:
        if (serial == m_expectedSerial) {
            m_expectedSerial = (m_expectedSerial + 1) % 32;
            sendAck(serial);
            m_stream += packet.mid(1);
            parseMessages();
        } else if (serial == (m_expectedSerial + 31) % 32) {
            // Our ACK got lost and the watch sent the packet again
            sendAck(serial);
        } else {
            qWarning() << Q_FUNC_INFO << "Unexpected serial" << serial << "expected" << m_expectedSerial;
        }
        break;
    case PacketAck:
        // An ACK confirms this packet and all packets before it
        m_unacked = (m_nextSerial - serial - 1 + 32) % 32;
        sendQueuedPackets();
        break;
    case PacketResetRequest:
        qDebug() << Q_FUNC_INFO << "Reset request" << packet.toHex();
        resetSession();
        // Payload: the PPoGATT version the watch wants
        if (packet.size() > 1 && packet.at(1) >= 1) {
            // PPoGATT version 1: our receive and send window is 25 packets; we send nothing until the watch
            // answers with its own Reset complete and its windows
            m_window = 0;
            m_data->notify(QByteArray::fromHex("031919"));
        } else {
            // PPoGATT version 0 has a fixed window of 4 packets
            m_window = 4;
            m_data->notify(QByteArray::fromHex("03"));
        }
        m_open = true;
        emit sessionOpened();
        break;
    case PacketResetComplete:
        // Version 1: the watch's receive and transmit windows; we send up to its receive window
        if (packet.size() >= 3) {
            m_window = qMin(25, int(static_cast<quint8>(packet.at(1))));
        }
        qDebug() << Q_FUNC_INFO << "Reset complete from the watch" << packet.toHex() << "window" << m_window;
        sendQueuedPackets();
        break;
    default:
        qWarning() << Q_FUNC_INFO << "Unknown packet" << packet.toHex();
        break;
    }
}

void PebblePPoGATTService::onNotifyingChanged(bool notifying)
{
    qDebug() << Q_FUNC_INFO << notifying;
    if (!notifying && m_open) {
        resetSession();
        emit sessionClosed();
    }
}

void PebblePPoGATTService::resetSession()
{
    m_open = false;
    m_nextSerial = 0;
    m_expectedSerial = 0;
    m_unacked = 0;
    m_queue.clear();
    m_stream.clear();
}

void PebblePPoGATTService::sendAck(int serial)
{
    m_data->notify(QByteArray(1, static_cast<char>(serial << 3 | PacketAck)));
}

void PebblePPoGATTService::sendQueuedPackets()
{
    while (!m_queue.isEmpty() && m_unacked < m_window) {
        QByteArray packet(1, static_cast<char>(m_nextSerial << 3 | PacketData));
        packet += m_queue.takeFirst();
        m_data->notify(packet);
        m_nextSerial = (m_nextSerial + 1) % 32;
        m_unacked++;
    }
}

void PebblePPoGATTService::parseMessages()
{
    while (m_stream.size() >= 4) {
        const uchar* header = reinterpret_cast<const uchar*>(m_stream.constData());
        int length = qFromBigEndian<quint16>(header);
        quint16 endpoint = qFromBigEndian<quint16>(header + 2);
        if (m_stream.size() < 4 + length) {
            return;
        }
        QByteArray payload = m_stream.mid(4, length);
        m_stream.remove(0, 4 + length);
        qDebug() << Q_FUNC_INFO << "endpoint" << endpoint << payload.toHex();
        emit messageReceived(endpoint, payload);
    }
}
