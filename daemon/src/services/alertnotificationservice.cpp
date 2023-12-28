#include "alertnotificationservice.h"

const char* AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION  = "00001811-0000-1000-8000-00805f9b34fb";
const char* AlertNotificationService::UUID_CHARACTERISTIC_ALERT_NOTIFICATION_NEW_ALERT = "00002a46-0000-1000-8000-00805f9b34fb";
const char* AlertNotificationService::UUID_CHARACTERISTIC_ALERT_NOTIFICATION_CONTROL = "00002a44-0000-1000-8000-00805f9b34fb";
const char* AlertNotificationService::UUID_CHARACTERISTIC_ALERT_NOTIFICATION_EVENT = "00020001-78fc-48fe-8e23-433b3a1942d0";

AlertNotificationService::AlertNotificationService(const QString &path, QObject *parent, uint8_t seperatorChar) : QBLEService(UUID_SERVICE_ALERT_NOTIFICATION, path, parent)
{
 qDebug() << Q_FUNC_INFO;
 m_seperatorChar = seperatorChar;
 connect(this, &QBLEService::characteristicChanged, this, &AlertNotificationService::characteristicChanged);

}

void AlertNotificationService::sendAlert(const QString &sender, const QString &subject, const QString &message)
{
    qDebug() << "Alert:" << sender << subject << message;

    int category = 0xfa; //Custom Huami icon
    int icon = mapSenderToIcon(sender);

    if (sender == "Messages") { //SMS must use category, not icon
        category = AlertCategory::SMS;
    }

    if (icon == HuamiIcon::EMAIL) { //Email icon doesnt work, so use category
        category = AlertCategory::Email;
    }

    QByteArray send = QByteArray(1, category) + QByteArray(1, 1); //1 alert

    send += QByteArray(1, mapSenderToIcon(sender));
    send += sender.left(32).toUtf8() + QByteArray(1, m_seperatorChar); //Bip needs 0x00 for seperator, others may be different

    if (!subject.isEmpty()) {
        send += subject.left(128).toUtf8() + QByteArray(2, 0x0a);
    }

    if (!message.isEmpty()) {
        send += message.left(128).toUtf8();
    }

    send.truncate(230); //!TODO is 230 is the max?
    writeValue(UUID_CHARACTERISTIC_ALERT_NOTIFICATION_NEW_ALERT, send);
}

void AlertNotificationService::incomingCall(const QByteArray header, const QString &caller)
{
    qDebug() << Q_FUNC_INFO << caller;
    QByteArray send = header + caller.toUtf8();
    writeValue(UUID_CHARACTERISTIC_ALERT_NOTIFICATION_NEW_ALERT, send);
}

int AlertNotificationService::mapSenderToIcon(const QString &sender)
{
    QString s = sender.toLower();

    QRegExp mailREX("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b");
    mailREX.setCaseSensitivity(Qt::CaseInsensitive);
    mailREX.setPatternSyntax(QRegExp::RegExp);

    if (mailREX.exactMatch(s)) {
            return HuamiIcon::EMAIL;
    }

    auto icon = AppToIconMap.find(s);

    if ( icon == AppToIconMap.end() )
        return HuamiIcon::APP_11;

    return icon->second;
}

void AlertNotificationService::characteristicChanged(const QString &c, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << c << value;
    emit serviceEvent(c, value[0]);
}
