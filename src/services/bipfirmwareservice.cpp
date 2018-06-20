#include "bipfirmwareservice.h"

const char* BipFirmwareService::UUID_SERVICE_FIRMWARE = "00001530-0000-3512-2118-0009af100700";
const char* BipFirmwareService::UUID_CHARACTERISTIC_FIRMWARE = "00001531-0000-3512-2118-0009af100700";
const char* BipFirmwareService::UUID_CHARACTERISTIC_FIRMWARE_DATA = "00001532-0000-3512-2118-0009af100700";


BipFirmwareService::BipFirmwareService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_FIRMWARE, path, parent)
{
    qDebug() << "BipFirmwareService::BipFirmwareService";

    connect(this, &QBLEService::characteristicChanged, this, &BipFirmwareService::characteristicChanged);
}


void BipFirmwareService::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << "FW Changed:" << characteristic << value;
}

void BipFirmwareService::downloadFile(const QString &path)
{
    qDebug() << "Sending file " << path;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;
    m_fwBytes = file.readAll();

    m_info = new HuamiFirmwareInfo(m_fwBytes);
}
