#ifndef DFUSERVICE_H
#define DFUSERVICE_H

#include "qble/qbleservice.h"
#include "abstractfirmwareinfo.h"

/*
{00001530-1212-EFDE-1523-785FEABCD123} DFU Service
--00001531-1212-EFDE-1523-785FEABCD123 //Control Point
--00001532-1212-EFDE-1523-785FEABCD123 //Firmware data
--00001534-1212-EFDE-1523-785FEABCD123 //Revision
*/

class DfuOperation;

class DfuService : public QBLEService
{
    Q_OBJECT

public:
    DfuService(const QString &path, QObject *parent);
    static const char *UUID_SERVICE_DFU;
    static const char *UUID_CHARACTERISTIC_DFU_CONTROL;
    static const char *UUID_CHARACTERISTIC_DFU_PACKET;
    static const char *UUID_CHARACTERISTIC_DFU_REVISION;

    static constexpr uint8_t COMMAND_STARTDFU = 0x01;
    static constexpr uint8_t COMMAND_INITDFUPARAMETERS = 0x02;
    static constexpr uint8_t COMMAND_RECEIVE_FIRMWARE_IMAGE = 0x03;
    static constexpr uint8_t COMMAND_VALIDATE_FIRMWARE = 0x04;
    static constexpr uint8_t COMMAND_ACTIVATE_IMAGE_AND_RESET = 0x05;
    static constexpr uint8_t COMMAND_PACKET_RECEIPT_NOTIFICATION_REQUEST = 0x08;
    static constexpr uint8_t COMMAND_RESPONSE = 0x10;
    static constexpr uint8_t COMMAND_PACKET_RECEIPT_NOTIFICATION = 0x11;

    static constexpr uint8_t TYPE_NOIMAGE = 0x00;
    static constexpr uint8_t TYPE_SOFTDEVICE = 0x01;
    static constexpr uint8_t TYPE_BOOLOADER = 0x02;
    static constexpr uint8_t TYPE_SOFTDEVICE_AND_BOOTLOADER = 0x03;
    static constexpr uint8_t TYPE_APPLICATION = 0x04;

    static constexpr uint8_t ERROR_UNKNOWN = 0x00;
    static constexpr uint8_t ERROR_NO_ERROR = 0x01;
    static constexpr uint8_t ERROR_INVALID_STATE = 0x02;
    static constexpr uint8_t ERROR_NOT_SUPPORTED = 0x03;
    static constexpr uint8_t ERROR_DATA_SIZE_EXCEEDS_LIMITS = 0x04;
    static constexpr uint8_t ERROR_CRC_ERROR = 0x05;
    static constexpr uint8_t ERROR_OPERATION_FAILED = 0x06;

    void prepareFirmwareDownload(const AbstractFirmwareInfo *info, DfuOperation* operation);
    void startDownload();
    Q_SIGNAL void downloadProgress(int percent);

    Q_INVOKABLE virtual bool operationRunning() override;
    void abortOperations();

private:
    Q_SLOT void characteristicChanged(const QString &characteristic, const QByteArray &value);

    int m_operationRunning = 0;
    DfuOperation *m_updateFirmware = nullptr;
};

#endif // DFUSERVICE_H
