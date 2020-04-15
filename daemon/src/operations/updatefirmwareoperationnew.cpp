#include "updatefirmwareoperationnew.h"

UpdateFirmwareOperationNew::UpdateFirmwareOperationNew()
{

}

bool UpdateFirmwareOperation::sendFwInfo()
{
    int fwSize = m_fwBytes.size();
    QByteArray sizeBytes = TypeConversion::fromInt24(fwSize);
    int arraySize = 10;

    QByteArray bytes(arraySize, 0x00);
    int i = 0;
    bytes[i++] = BipFirmwareService::COMMAND_FIRMWARE_INIT;
    bytes[i++] = m_info->type();
    bytes[i++] = sizeBytes[0];
    bytes[i++] = sizeBytes[1];
    bytes[i++] = sizeBytes[2];
    bytes[i++] = 0; // TODO: what is that?
    int crc32 = firmwareInfo.getCrc32();
    QByteArray crcBytes = BLETypeConversions.fromUint32(crc32);
    bytes[i++] = crcBytes[0];
    bytes[i++] = crcBytes[1];
    bytes[i++] = crcBytes[2];
    bytes[i] = crcBytes[3];

    m_service->writeValue(BipFirmwareService::UUID_CHARACTERISTIC_FIRMWARE, bytes);
    return true;
}
