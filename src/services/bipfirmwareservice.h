#ifndef BIPFIRMWARESERVICE_H
#define BIPFIRMWARESERVICE_H

#include "qble/qbleservice.h"
#include "settingsmanager.h"

/*
{00001530-0000-3512-2118-0009af100700} Firmware Service
--00001531-0000-3512-2118-0009af100700 //Firmware
--00001532-0000-3512-2118-0009af100700 //Firmware data
*/
class BipFirmwareService : public QBLEService
{
    Q_OBJECT

public:
    BipFirmwareService(const QString &path, QObject *parent);
    static const char *UUID_SERVICE_BIP_FIRMWARE;
    static const char *UUID_CHARACTERISTIC_FIRMWARE;
    static const char *UUID_CHARACTERISTIC_FIRMWARE_DATA;

private:
    Q_SLOT void characteristicChanged(const QString &characteristic, const QByteArray &value);

    SettingsManager m_settings;
};

#endif
