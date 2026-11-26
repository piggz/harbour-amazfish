#ifndef PEBBLEPPOGATTSERVICE_H
#define PEBBLEPPOGATTSERVICE_H

#include "qble/qblelocalservice.h"

#include <QByteArray>
#include <QList>

class PebbleDataCharacteristic;

// Extra service with one readable characteristic, served next to PPoGATT like the official app does.
class PebbleFakeService : public QBLELocalService {
public:
    PebbleFakeService(QDBusConnection bus, const QString& appPath, QObject* parent = nullptr);

    static const char* UUID_SERVICE_FAKE;
};

// PPoGATT (Pebble Protocol over GATT) served on the phone side.
// The Pebble Time is the GATT client: it reads the meta characteristic, writes packets to the
// data characteristic, and gets packets from the phone as notifications on the same characteristic.
// Each packet starts with one byte: serial << 3 | type. The packets carry a byte stream of
// Pebble Protocol messages: length (u16 BE), endpoint (u16 BE), payload.
class PebblePPoGATTService : public QBLELocalService {
    Q_OBJECT
public:
    PebblePPoGATTService(QDBusConnection bus, const QString& appPath, QObject* parent = nullptr);

    static const char* UUID_SERVICE_PPOGATT;
    static const char* UUID_CHARACTERISTIC_DATA;
    static const char* UUID_CHARACTERISTIC_META;

    bool isOpen() const;
    void sendMessage(quint16 endpoint, const QByteArray& payload);

Q_SIGNALS:
    void sessionOpened();
    void sessionClosed();
    void messageReceived(quint16 endpoint, const QByteArray& payload);

private:
    enum PacketType { PacketData = 0,
        PacketAck = 1,
        PacketResetRequest = 2,
        PacketResetComplete = 3 };

    void onPacket(const QByteArray& packet);
    void onNotifyingChanged(bool notifying);
    void resetSession();
    void sendAck(int serial);
    void sendQueuedPackets();
    void parseMessages();

    PebbleDataCharacteristic* m_data = nullptr;
    bool m_open = false;
    int m_window = 1;
    int m_nextSerial = 0;
    int m_expectedSerial = 0;
    int m_unacked = 0;
    QList<QByteArray> m_queue;
    QByteArray m_stream;
};

#endif // PEBBLEPPOGATTSERVICE_H
