#include "bipfirmwareservice.h"

const char* BipFirmwareService::UUID_SERVICE_FIRMWARE = "00001530-0000-3512-2118-0009af100700";
const char* BipFirmwareService::UUID_CHARACTERISTIC_FIRMWARE = "00001531-0000-3512-2118-0009af100700";
const char* BipFirmwareService::UUID_CHARACTERISTIC_FIRMWARE_DATA = "00001532-0000-3512-2118-0009af100700";
const char* BipFirmwareService::UUID_CHARACTERISTIC_ZEPP_OS_FILE_TRANSFER_V3_SEND = "00000023-0000-3512-2118-0009af100700";
const char* BipFirmwareService::UUID_CHARACTERISTIC_ZEPP_OS_FILE_TRANSFER_V3_RECEIVE = "00000024-0000-3512-2118-0009af100700";

BipFirmwareService::BipFirmwareService(const QString &path, QObject *parent) : AbstractOperationService(UUID_SERVICE_FIRMWARE, path, parent)
{
    qDebug() << Q_FUNC_INFO;

    connect(this, &QBLEService::characteristicChanged, this, &BipFirmwareService::characteristicChanged);
}


void BipFirmwareService::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << "Changed:" << characteristic << value;

    AbstractOperationService::notifyOperation(characteristic, value);
}
