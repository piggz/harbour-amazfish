#include "alertnotificationservice.h"

const char* AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION  = "{00001811-0000-1000-8000-00805f9b34fb}";
const char* AlertNotificationService::UUID_CHARACTERISTIC_ALERT_NOTIFICATION_NEW_ALERT = "{00002a46-0000-1000-8000-00805f9b34fb}";
const char* AlertNotificationService::UUID_CHARACTERISTIC_ALERT_NOTIFICATION_CONTROL = "{00002a44-0000-1000-8000-00805f9b34fb}";

AlertNotificationService::AlertNotificationService(QObject *parent) : BipService(UUID_SERVICE_ALERT_NOTIFICATION, parent)
{

}

void AlertNotificationService::sendAlert(const QString &sender, const QString &subject, const QString &message, int category, int icon)
{
    qDebug() << "Alert:" << sender << subject << message << category << icon;
    QString msg;
    msg += sender.left(32).toLocal8Bit() + "\0";

    if (!subject.isEmpty()) {
        msg += subject.left(128).toLocal8Bit() + "\n\n";
    }

    if (!message.isEmpty()) {
        msg += message.left(128).toLocal8Bit();
    }

    msg.truncate(230);

    QByteArray send = QByteArray::fromHex("fa0119");
    send += msg.toUtf8();

    qDebug() << (-6 & 0xff) << (char)(-6 & 0xff);
    qDebug() << send.toHex();

    service()->writeCharacteristic(service()->characteristic(QBluetoothUuid(QString(UUID_CHARACTERISTIC_ALERT_NOTIFICATION_NEW_ALERT))), send);
}
