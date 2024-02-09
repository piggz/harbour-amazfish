#ifndef DEVICEINFOSERVICE_H
#define DEVICEINFOSERVICE_H

#include "qble/qbleservice.h"
#include "devices/abstractdevice.h"

class DeviceInfoService : public QBLEService
{
    Q_OBJECT
public:
    DeviceInfoService(const QString &path, QObject *parent);

    static const char* UUID_SERVICE_DEVICEINFO;
    static const char* UUID_CHARACTERISTIC_INFO_SERIAL_NO;
    static const char* UUID_CHARACTERISTIC_INFO_HARDWARE_REV;
    static const char* UUID_CHARACTERISTIC_INFO_SOFTWARE_REV;
    static const char* UUID_CHARACTERISTIC_INFO_SYSTEM_ID;
    static const char* UUID_CHARACTERISTIC_INFO_PNP_ID;
    static const char* UUID_CHARACTERISTIC_INFO_MODEL_NO;
    static const char* UUID_CHARACTERISTIC_INFO_FW_REVISION;
    static const char* UUID_CHARACTERISTIC_INFO_MANUFACTURER_NAME;

    Q_INVOKABLE void refreshInformation();

    QString serialNumber() const;
    QString hardwareRevision() const;
    QString softwareRevision() const;
    QString systemId() const;
    QString pnpId() const;
    QString readSoftwareRevisionSync();

    //Pinetime
    QString modelNumber() const;
    QString fwRevision() const;
    QString manufacturerName() const;

    Q_SIGNAL void informationChanged(AbstractDevice::Info key, const QString &val);

private:
    QString m_serialNumber;
    QString m_hardwareRevision;
    QString m_softwareRevision;
    QString m_systemId;
    QString m_pnpId;
    QString m_model;
    QString m_fwRevision;
    QString m_manufacturer;

    Q_SLOT void characteristicRead(const QString &c, const QByteArray &value);
};

#endif // DEVICEINFOSERVICE_H
