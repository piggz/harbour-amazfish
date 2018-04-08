#ifndef BIPINFOSERVICE_H
#define BIPINFOSERVICE_H

#include "bipservice.h"

class BipInfoService : public BipService
{
    Q_OBJECT
public:
    BipInfoService(QObject *parent);

    const QString UUID_CHARACTERISTIC_INFO_SERIAL_NO = "{00002a25-0000-1000-8000-00805f9b34fb}";
    const QString UUID_CHARACTERISTIC_INFO_HARDWARE_REV = "{00002a27-0000-1000-8000-00805f9b34fb}";
    const QString UUID_CHARACTERISTIC_INFO_SOFTWARE_REV = "{00002a28-0000-1000-8000-00805f9b34fb}";
    const QString UUID_CHARACTERISTIC_INFO_SYSTEM_ID = "{00002a23-0000-1000-8000-00805f9b34fb}";
    const QString UUID_CHARACTERISTIC_INFO_PNP_ID = "{00002a50-0000-1000-8000-00805f9b34fb}";

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

    Q_SLOT void characteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value);
};

#endif // BIPINFOSERVICE_H
