#include "uartservice.h"

const char* UARTService::UUID_SERVICE_UART = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const char* UARTService::UUID_CHARACTERISTIC_UART_TX = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
const char* UARTService::UUID_CHARACTERISTIC_UART_RX = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

UARTService::UARTService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_UART, path, parent)
{
    qDebug() << Q_FUNC_INFO;
    connect(this, &QBLEService::characteristicChanged, this, &UARTService::characteristicChanged);

    m_txTimer.setSingleShot(true);
    connect(&m_txTimer, &QTimer::timeout, this, &UARTService::sendNextPacket);
}

// Queue the data in 20 byte packets. One packet is written per event loop
// iteration, so that incoming XOFF is not missed while a long message is sent.
void UARTService::tx(const QByteArray &bytes)
{
    qDebug() << Q_FUNC_INFO << bytes;

    for (int i=0; i<bytes.length(); i+=20) {
        int l = bytes.length() - i;
        if (l > 20) {
            l = 20;
        }

        m_txQueue.enqueue(bytes.mid(i, l));
    }

    if (!m_txTimer.isActive()) {
        m_txTimer.start(0);
    }
}

void UARTService::sendNextPacket()
{
    if (m_txQueue.isEmpty()) {
        return;
    }

    if (m_txPaused) { // wait for XON
        m_txTimer.start(100);
        return;
    }

    if (!writeValue(UUID_CHARACTERISTIC_UART_TX, m_txQueue.dequeue())) {
        qWarning() << Q_FUNC_INFO << "write failed, dropping" << m_txQueue.count() << "queued packets";
        m_txQueue.clear();
        return;
    }

    if (!m_txQueue.isEmpty()) {
        m_txTimer.start(0);
    }
}

void UARTService::txJson(const QJsonObject &json)
{
    qDebug() << Q_FUNC_INFO << json;

    QJsonDocument doc(json);
    QByteArray strJson(doc.toJson(QJsonDocument::Compact));

    tx(QByteArray(1, 0x10) + "GB(" + strJson + ")\n");
}

void UARTService::characteristicChanged(const QString &c, const QByteArray &value)
{
    // qDebug() << Q_FUNC_INFO << c << value;

    if (c == UUID_CHARACTERISTIC_UART_RX) {
        // Espruino throttles us with XOFF/XON when its input buffer fills up
        QByteArray data;
        data.reserve(value.length());
        for (int i = 0; i < value.length(); ++i) {
            const char ch = value.at(i);
            if (ch == 0x13) {        // XOFF
                m_txPaused = true;
            } else if (ch == 0x11) { // XON
                m_txPaused = false;
            } else {
                data.append(ch);
            }
        }

        m_incomingJson += data;
        while (m_incomingJson.contains("\n")) {
            int p = m_incomingJson.indexOf("\n");
            QString json =  m_incomingJson.mid(0,p-1);
            m_incomingJson = m_incomingJson.mid(p+1);
            handleRx(json);
        }
    }
}

void UARTService::handleRx(const QString &json)
{
    // qDebug() << Q_FUNC_INFO << json;

    if (json.contains( "Uncaught ReferenceError: \"GB\" is not defined")) {
        emit message(tr("Android Integration plugin not installed on Bangle.js"));
    } else if ((json.length() > 0) && (json.at(0)=='{')) {
        // JSON - we hope!
        QJsonObject obj = ObjectFromString(json);
        emit jsonRx(obj);
    }
}

QJsonObject UARTService::ObjectFromString(const QString& in)
{
    QJsonObject obj;
    QJsonDocument doc = QJsonDocument::fromJson(in.toUtf8());

    // check validity of the document
    if(!doc.isNull()) {
        if(doc.isObject()) {
            obj = doc.object();
        } else {
            qDebug() << Q_FUNC_INFO << "Document is not an object";
        }
    } else {
        qDebug() << Q_FUNC_INFO << "Invalid JSON.";
    }
    return obj;
}
