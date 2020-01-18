#ifndef BIPFIRMWARESERVICE_H
#define BIPFIRMWARESERVICE_H

#include "qble/qbleservice.h"
#include "settingsmanager.h"
#include "updatefirmwareoperation.h"

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
    static const char *UUID_SERVICE_FIRMWARE;
    static const char *UUID_CHARACTERISTIC_FIRMWARE;
    static const char *UUID_CHARACTERISTIC_FIRMWARE_DATA;


    static const char RESPONSE = 0x10;
    static const char SUCCESS = 0x01;
    static const char FAIL = 0x04;

    static const char COMMAND_FIRMWARE_INIT = 0x01; // to UUID_CHARACTERISTIC_FIRMWARE, followed by fw file size in bytes
    static const char COMMAND_FIRMWARE_START_DATA = 0x03; // to UUID_CHARACTERISTIC_FIRMWARE
    static const char COMMAND_FIRMWARE_UPDATE_SYNC = 0x00; // to UUID_CHARACTERISTIC_FIRMWARE
    static const char COMMAND_FIRMWARE_CHECKSUM = 0x04; // to UUID_CHARACTERISTIC_FIRMWARE
    static const char COMMAND_FIRMWARE_REBOOT = 0x05; // to UUID_CHARACTERISTIC_FIRMWARE


    void prepareFirmwareDownload(const AbstractFirmwareInfo *info);
    void startDownload();
    Q_SIGNAL void downloadProgress(int percent);

    Q_INVOKABLE virtual bool operationRunning() override;
    void abortOperations();

private:
    Q_SLOT void characteristicChanged(const QString &characteristic, const QByteArray &value);

    SettingsManager m_settings;
    int m_operationRunning = 0;

    UpdateFirmwareOperation *m_updateFirmware = nullptr;
};

#endif
