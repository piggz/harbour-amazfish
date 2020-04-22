#include "updatefirmwareoperationnew.h"
#include"typeconversion.h"
#include "bipfirmwareservice.h"

UpdateFirmwareOperationNew::UpdateFirmwareOperationNew(const AbstractFirmwareInfo *info, QBLEService *service) : UpdateFirmwareOperation(info, service)
{
    m_startWithFWInfo = false;
}

bool UpdateFirmwareOperationNew::sendFwInfo()
{
    int fwSize = m_fwBytes.size();
    QByteArray sizeBytes = TypeConversion::fromInt24(fwSize);
    int arraySize = 10;

    QByteArray bytes(arraySize, char(0x00));
    int i = 0;
    bytes[i++] = BipFirmwareService::COMMAND_FIRMWARE_INIT;
    bytes[i++] = m_info->type();
    bytes[i++] = sizeBytes[0];
    bytes[i++] = sizeBytes[1];
    bytes[i++] = sizeBytes[2];
    bytes[i++] = 0; // TODO: what is that?
    int crc32 = m_info->getCrc32();
    QByteArray crcBytes = TypeConversion::fromInt32(crc32);
    qDebug() << "Got" << crcBytes;
    bytes[i++] = crcBytes[0];
    bytes[i++] = crcBytes[1];
    bytes[i++] = crcBytes[2];
    bytes[i] = crcBytes[3];

    qDebug() << "Sending FW info" <<bytes;

    m_service->writeValue(BipFirmwareService::UUID_CHARACTERISTIC_FIRMWARE, bytes);
    return true;
}

void UpdateFirmwareOperationNew::sendChecksum()
{
    m_service->writeValue(BipFirmwareService::UUID_CHARACTERISTIC_FIRMWARE, QByteArray(1, BipFirmwareService::COMMAND_FIRMWARE_CHECKSUM));
}

QByteArray UpdateFirmwareOperationNew::getFirmwareStartCommand()
{
    return QByteArray(1, BipFirmwareService::COMMAND_FIRMWARE_START_DATA) + QByteArray(1, 0x01);
}
