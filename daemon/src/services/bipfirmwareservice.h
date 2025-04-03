#ifndef BIPFIRMWARESERVICE_H
#define BIPFIRMWARESERVICE_H

#include "abstractoperationservice.h"
#include "updatefirmwareoperation.h"

/*
{00001530-0000-3512-2118-0009af100700} Firmware Service
--00001531-0000-3512-2118-0009af100700 //Firmware
--00001532-0000-3512-2118-0009af100700 //Firmware data
*/
class BipFirmwareService : public AbstractOperationService
{
    Q_OBJECT

public:
    BipFirmwareService(const QString &path, QObject *parent);
    static const char *UUID_SERVICE_FIRMWARE;
    static const char *UUID_CHARACTERISTIC_FIRMWARE;
    static const char *UUID_CHARACTERISTIC_FIRMWARE_DATA;

    static constexpr uint8_t RESPONSE = 0x10;
    static constexpr uint8_t SUCCESS = 0x01;
    static constexpr uint8_t FAIL = 0x04;

    static constexpr uint8_t COMMAND_FIRMWARE_INIT = 0x01; // to UUID_CHARACTERISTIC_FIRMWARE, followed by fw file size in bytes
    static constexpr uint8_t COMMAND_FIRMWARE_START_DATA = 0x03; // to UUID_CHARACTERISTIC_FIRMWARE
    static constexpr uint8_t COMMAND_FIRMWARE_UPDATE_SYNC = 0x00; // to UUID_CHARACTERISTIC_FIRMWARE
    static constexpr uint8_t COMMAND_FIRMWARE_CHECKSUM = 0x04; // to UUID_CHARACTERISTIC_FIRMWARE
    static constexpr uint8_t COMMAND_FIRMWARE_REBOOT = 0x05; // to UUID_CHARACTERISTIC_FIRMWARE


    void prepareFirmwareDownload(const AbstractFirmwareInfo *info, UpdateFirmwareOperation* operation);
    void startDownload();
    Q_SIGNAL void downloadProgress(int percent);

    Q_INVOKABLE virtual bool operationRunning() override;
    void abortOperations();

private:
    Q_SLOT void characteristicChanged(const QString &characteristic, const QByteArray &value);

    int m_operationRunning = 0;

    UpdateFirmwareOperation *m_updateFirmware = nullptr;
};

#endif
