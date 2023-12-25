#ifndef ASTEROIDNOTIFICATIONSERVICE_H
#define ASTEROIDNOTIFICATIONSERVICE_H

#include "qble/qbleservice.h"

class AsteroidNotificationService : public QBLEService
{
    Q_OBJECT
public:
    AsteroidNotificationService(const QString &path, QObject *parent);

    static const char* UUID_SERVICE_NOTIFICATION;
    static const char* UUID_CHARACTERISTIC_NOTIFICATION_UPDATE;
    static const char* UUID_CHARACTERISTIC_NOTIFICATION_FEEDBACK;

    Q_INVOKABLE void sendAlert(const QString &sender, const QString &subject, const QString &message);
    Q_INVOKABLE void removeNotification(unsigned int id);
    Q_INVOKABLE void incomingCall(const QString &caller);
    Q_INVOKABLE void incomingCallEnded();
    static QString mapSenderToIcon(const QString &sender);

    Q_SIGNAL void serviceEvent(const QString &c, uint8_t event);

private:
    void characteristicChanged(const QString &c, const QByteArray &value);
    unsigned int m_lastNotificationId = 0;
    unsigned int m_lastVoiceCallNotification = 0;
};

#endif // ASTEROIDNOTIFICATIONSERVICE_H
