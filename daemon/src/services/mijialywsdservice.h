#ifndef MIJIALYWSDSERVICE_H
#define MIJIALYWSDSERVICE_H

#include "qble/qbleservice.h"
#include "devices/abstractdevice.h"

class MijiaLywsdService : public QBLEService
{
    Q_OBJECT
public:
    MijiaLywsdService(const QString &path, QObject *parent);

    static const char* UUID_SERVICE_MIJIA_LYWSD;
    static const char* UUID_CHARACTERISTIC_LIVE_DATA;

    Q_INVOKABLE void refreshInformation();
    Q_INVOKABLE void enableNotifications();

    Q_SIGNAL void informationChanged(Amazfish::Info key, const QString &val);

private:
    Q_SLOT void characteristicRead(const QString &c, const QByteArray &value);
    Q_SLOT void characteristicChanged(const QString &c, const QByteArray &value);
};

#endif // MIJIALYWSDSERVICE_H
