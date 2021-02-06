#include "uartservice.h"

const char* UARTService::UUID_SERVICE_UART = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const char* UARTService::UUID_CHARACTERISTIC_UART_TX = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
const char* UARTService::UUID_CHARACTERISTIC_UART_RX = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

UARTService::UARTService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_UART, path, parent)
{
    qDebug() << Q_FUNC_INFO;
    connect(this, &QBLEService::characteristicChanged, this, &UARTService::characteristicChanged);
}

void UARTService::tx(const QByteArray &bytes)
{
    qDebug() << Q_FUNC_INFO << bytes;

    for (int i=0; i<bytes.length(); i+=20) {
        int l = bytes.length() - i;
        if (l > 20) {
            l = 20;
        }

        QByteArray packet = bytes.mid(i, l);
        writeValue(UUID_CHARACTERISTIC_UART_TX, packet);
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
    qDebug() << Q_FUNC_INFO << c << value;
}

