#ifndef BIPINFOSERVICE_H
#define BIPINFOSERVICE_H

#include "qble/qbleservice.h"

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

    Q_PROPERTY(QString serialNumber READ serialNumber NOTIFY serialNumberChanged)
    Q_PROPERTY(QString hardwareRevision READ hardwareRevision NOTIFY hardwareRevisionChanged)
    Q_PROPERTY(QString softwareRevision READ softwareRevision NOTIFY softwareRevisionChanged)
    Q_PROPERTY(QString systemId READ systemId NOTIFY systemIdChanged)
    Q_PROPERTY(QString pnpId READ pnpId NOTIFY pnpIdChanged)

    Q_INVOKABLE void refreshInformation();

    Q_INVOKABLE QString serialNumber() const;
    Q_INVOKABLE QString hardwareRevision() const;
    Q_INVOKABLE QString softwareRevision() const;
    Q_INVOKABLE QString systemId() const;
    Q_INVOKABLE QString pnpId() const;
    Q_INVOKABLE QString readSoftwareRevisionSync();

    Q_SIGNAL void serialNumberChanged();
    Q_SIGNAL void hardwareRevisionChanged();
    Q_SIGNAL void softwareRevisionChanged();
    Q_SIGNAL void systemIdChanged();
    Q_SIGNAL void pnpIdChanged();

private:
    QString m_serialNumber;
    QString m_hardwareRevision;
    QString m_softwareRevision;
    QString m_systemId;
    QString m_pnpId;

    Q_SLOT void characteristicRead(const QString &c, const QByteArray &value);
};

#endif // BIPINFOSERVICE_H
