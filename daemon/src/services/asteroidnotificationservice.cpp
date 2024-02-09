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
//    qDebug() << Q_FUNC_INFO << sender << subject << message;

    QString icon = mapSenderToIcon(sender);

    // for vibrate, valid options are { "ringtone", "strong", "normal", "none" }
    QString vibrate = "normal";

    QByteArray data = QString("<insert><pn>%1</pn><id>%2</id><an>%3</an><ai>%4</ai><su>%5</su><bo>%6</bo><vb>%7</vb></insert>")
                .arg(sender, QString::number(m_lastNotificationId), sender, icon, subject, message, vibrate).toUtf8();

    m_lastNotificationId++;

    writeValue(UUID_CHARACTERISTIC_NOTIFICATION_UPDATE, data);
}

void AsteroidNotificationService::incomingCall(const QString &caller)
{
    qDebug() << Q_FUNC_INFO << caller;
    m_lastVoiceCallNotification = m_lastNotificationId;
    sendAlert("incoming-call", caller, "");
}

void AsteroidNotificationService::incomingCallEnded()
{
    removeNotification(m_lastVoiceCallNotification);
}

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

QString AsteroidNotificationService::mapSenderToIcon(const QString &sender) {

    // see https://github.com/AsteroidOS/asteroid-icons-ion
    const QMap<QString, QString> iconMapping = {
        {"Dekko", "ios-mail"},
        {"TELEports", "ios-paper-plane"},
        {"Cinny", "ios-paper-plane"},
        {"indicator-datetime", "ios-alarm"},
        {"incoming-call", "ios-call"},
    };

    for (auto it = iconMapping.begin(); it != iconMapping.end(); ++it) {
        if (sender == it.key()) {
            return it.value();
        }
    }

    return "ios-mail-open-outline";
}