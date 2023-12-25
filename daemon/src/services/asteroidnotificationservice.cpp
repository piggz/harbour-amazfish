#include "asteroidnotificationservice.h"

const char* AsteroidNotificationService::UUID_SERVICE_NOTIFICATION = "00009071-0000-0000-0000-00a57e401d05";
const char* AsteroidNotificationService::UUID_CHARACTERISTIC_NOTIFICATION_UPDATE   = "00009001-0000-0000-0000-00a57e401d05";
const char* AsteroidNotificationService::UUID_CHARACTERISTIC_NOTIFICATION_FEEDBACK = "00009002-0000-0000-0000-00a57e401d05";


AsteroidNotificationService::AsteroidNotificationService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_NOTIFICATION, path, parent)
{
    qDebug() << Q_FUNC_INFO;
    connect(this, &QBLEService::characteristicChanged, this, &AsteroidNotificationService::characteristicChanged);
}

void AsteroidNotificationService::sendAlert(const QString &sender, const QString &subject, const QString &message)
{

    unsigned int id = 0;
    QString icon = sender;

    // for vibrate, valid options are { "ringtone", "strong", "normal", "none" }
    QString vibrate = "normal";

    QByteArray data = QString("<insert><pn>%1</pn><id>%2</id><an>%3</an><ai>%4</ai><su>%5</su><bo>%6</bo><vb>%7</vb></insert>")
                .arg(sender, QString::number(id), sender, icon, subject, message, vibrate).toUtf8();


    qDebug() << Q_FUNC_INFO << sender << subject << message << data;

    writeValue(UUID_CHARACTERISTIC_NOTIFICATION_UPDATE, data);
}

//void AlertNotificationService::incomingCall(const QByteArray header, const QString &caller)
//{
//    qDebug() << Q_FUNC_INFO << caller;
//    QByteArray send = header + caller.toUtf8();
//    writeValue(UUID_CHARACTERISTIC_ALERT_NOTIFICATION_NEW_ALERT, send);
//}



void AsteroidNotificationService::removeNotification(unsigned int id)
{
    QByteArray data = QString("<removed><id>%1</id></removed>").arg(id).toUtf8();
    writeValue(UUID_CHARACTERISTIC_NOTIFICATION_UPDATE, data);
}


void AsteroidNotificationService::characteristicChanged(const QString &c, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << c << value;
    emit serviceEvent(c, value[0]);
}
