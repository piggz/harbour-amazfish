#ifndef UARTSERVICE_H
#define UARTSERVICE_H

#include <QObject>
#include "qble/qbleservice.h"
#include "abstractdevice.h"

/** 6e400001-b5a3-f393-e0a9-e50e24dcca9e --UART Service
 * "6e400002-b5a3-f393-e0a9-e50e24dcca9e", --TX
 * "6e400003-b5a3-f393-e0a9-e50e24dcca9e", --RX
 */
class UARTService : public QBLEService
{
    Q_OBJECT
public:
    UARTService(const QString &path, QObject *parent);

    static const char *UUID_SERVICE_UART;
    static const char *UUID_CHARACTERISTIC_UART_TX;
    static const char *UUID_CHARACTERISTIC_UART_RX;

    void tx(const QByteArray& str);
    void txJson(const QJsonObject &json);

    Q_SIGNAL void serviceEvent(char event);
    Q_SIGNAL void jsonRx(const QJsonObject &json);

private:
    void characteristicChanged(const QString &c, const QByteArray &value);
    void handleRx(const QString &json);
    QJsonObject ObjectFromString(const QString& in);

    QByteArray m_incomingJson;
};

#endif // UARTSERVICE_H
