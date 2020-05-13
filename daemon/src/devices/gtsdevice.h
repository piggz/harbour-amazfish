#ifndef GTSDEVICE_H
#define GTSDEVICE_H

#include <QObject>
#include "bipdevice.h"

class GtsDevice : public BipDevice
{
public:
    GtsDevice(const QString &pairedName, QObject *parent = 0);

    virtual QString deviceType() override;
    virtual void sendAlert(const QString &sender, const QString &subject, const QString &message) override;
    virtual void sendEventReminder(int id, const QDateTime &dt, const QString &event) override;
    virtual int supportedFeatures() override;
    void prepareFirmwareDownload(const AbstractFirmwareInfo *info) override;

    virtual AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;

    virtual void sendWeather(CurrentWeather *weather) override;
    
private:
    void initialise();
    void parseServices();

    virtual void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);

private:
    QString pairedName;
};

#endif // GTSDEVICE_H
