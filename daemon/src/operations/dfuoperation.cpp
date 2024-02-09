#include "dfuoperation.h"
#include "dfuservice.h"
#include "typeconversion.h"
#include <QApplication>
#include <KArchive>
#include <kzip.h>
#include <KCompressionDevice>

namespace {
QString getFirmwareFileName(QByteArray& manifestData) {
    QJsonDocument manifestDocument = QJsonDocument::fromJson(manifestData);

    if(manifestDocument.isNull() || !manifestDocument.isObject()) return {};
    QJsonObject manifestJson = manifestDocument.object();

    if(manifestJson["manifest"].isUndefined() || !manifestJson["manifest"].isObject())  return {};
    QJsonObject manifestObject = manifestJson["manifest"].toObject();

    if(manifestObject["application"].isUndefined() || !manifestObject["application"].isObject()) return {};
    QJsonObject applicationObject = manifestObject["application"].toObject();

    if(applicationObject["bin_file"].isUndefined() || !applicationObject["bin_file"].isString()) return {};

    QString binFile = applicationObject["bin_file"].toString();
    return binFile;
}

uint16_t getFirmwareCrc(QByteArray& manifestData, bool& valid) {
    valid = false;
    QJsonDocument manifestDocument = QJsonDocument::fromJson(manifestData);

    if(manifestDocument.isNull() || !manifestDocument.isObject()) return 0;
    QJsonObject manifestJson = manifestDocument.object();

    if(manifestJson["manifest"].isUndefined() || !manifestJson["manifest"].isObject())  return 0;
    auto manifestObject = manifestJson["manifest"].toObject();

    if(manifestObject["application"].isUndefined() || !manifestObject["application"].isObject()) return 0;
    auto applicationObject = manifestObject["application"].toObject();

    if(applicationObject["init_packet_data"].isUndefined() || !applicationObject["init_packet_data"].isObject()) return 0;
    auto initPacketDataObject = applicationObject["init_packet_data"].toObject();


    if(applicationObject["firmware_crc16"].isUndefined()) return 0;

    uint16_t crc = initPacketDataObject["firmware_crc16"].toInt(0);
    valid = true;
    return crc;
}
}

DfuOperation::DfuOperation(const AbstractFirmwareInfo *info, QBLEService *service) : AbstractOperation(service), m_info(info)
{

}

DfuOperation::~DfuOperation()
{
    if (m_workerThread.isRunning()) {
        m_workerThread.requestInterruption();
        qDebug() << Q_FUNC_INFO << "Waiting for thread to finish";
        m_workerThread.quit();
        m_workerThread.wait();
        qDebug() << Q_FUNC_INFO << "Done";
    }
}

bool DfuOperation::probeArchive()
{
    QByteArray zippedFwBytes = m_info->bytes();
    QDataStream in(&zippedFwBytes, QIODevice::ReadOnly);
    KCompressionDevice dev(in.device(), false, KCompressionDevice::CompressionType::None);
    KZip zip(&dev);

    qDebug() << Q_FUNC_INFO;

    if(!zip.open(QIODevice::ReadOnly))
    {
        qDebug() << "Cannot open the firmware archive";
        return false;
    }

    const auto* root = zip.directory();
    const auto* manifestEntry = dynamic_cast<const KZipFileEntry*>(root->entry("manifest.json"));
    if(manifestEntry == nullptr)
    {
        qDebug() << "Invalid firmware archive, cannot find manifest.json";
    }

    auto manifestData = manifestEntry->data();
    QString firmwareFileName = getFirmwareFileName(manifestData);
    if(firmwareFileName.isEmpty())
    {
        qDebug() << "Invalid firmware archive, cannot read manifest.json";
        return false;
    }

    const auto* firmwareEntry = dynamic_cast<const KZipFileEntry*>(root->entry(firmwareFileName));
    if(firmwareEntry == nullptr)
    {
        qDebug() << "Invalid firmware archive, cannot open firmware file";
        return false;
    }
    m_uncompressedFwBytes = firmwareEntry->data();

    bool isCrcValid = false;
    m_crc16 = getFirmwareCrc(manifestData, isCrcValid);
    if(!isCrcValid)
    {
        qDebug() << "Invalid firmware archive, cannot read CRC";
        return false;
    }
    qDebug() << "Firmware file name : " << firmwareFileName << " (CRC : " << m_crc16 <<")";

    return true;
}

void DfuOperation::start()
{
    qDebug() << Q_FUNC_INFO;
    bool probeOk = probeArchive();

    if (m_info->type() == AbstractFirmwareInfo::Firmware && probeOk && m_uncompressedFwBytes.size() > 0) {
        DfuService *serv = dynamic_cast<DfuService*>(m_service);

        m_service->enableNotification(serv->UUID_CHARACTERISTIC_DFU_CONTROL);

        //Send Start command to control point
        m_service->writeValue(DfuService::UUID_CHARACTERISTIC_DFU_CONTROL, UCHAR_TO_BYTEARRAY(DfuService::COMMAND_STARTDFU) + QByteArray(1, 0x04));

        //Send byte count to packet point <Length of SoftDevice><Length of bootloader><Length of application> each uint32
        QByteArray lengths;
        lengths += TypeConversion::fromInt32(0);
        lengths += TypeConversion::fromInt32(0);
        lengths += TypeConversion::fromInt32(m_uncompressedFwBytes.length());
        m_service->writeValue(DfuService::UUID_CHARACTERISTIC_DFU_PACKET, lengths);

        //Send packet request command to control point
        m_service->writeValue(DfuService::UUID_CHARACTERISTIC_DFU_CONTROL, UCHAR_TO_BYTEARRAY(DfuService::COMMAND_PACKET_RECEIPT_NOTIFICATION_REQUEST) + QByteArray(1, m_notificationPackets));

    } else {
        emit transferError("File does not seem to be supported");
    }
}

bool DfuOperation::handleMetaData(const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << value;

    if (!(value.length() == 3 || value.length() == 5)) {
        qDebug() << Q_FUNC_INFO << "Notifications should be 3 or 5 bytes long.";
        return false;
    }
    bool success = value[2] == DfuService::ERROR_NO_ERROR;

    if (value[0] == DfuService::COMMAND_RESPONSE && success) {
        switch (value[1]) {
        case DfuService::COMMAND_STARTDFU: {
            //Send Init packet to control point
            m_service->writeValue(DfuService::UUID_CHARACTERISTIC_DFU_CONTROL, UCHAR_TO_BYTEARRAY(DfuService::COMMAND_INITDFUPARAMETERS) + QByteArray(1, 0x00));

            //Send safety info to packets point
            sendFwInfo();
            m_service->writeValue(DfuService::UUID_CHARACTERISTIC_DFU_CONTROL, UCHAR_TO_BYTEARRAY(DfuService::COMMAND_INITDFUPARAMETERS) + QByteArray(1, 0x01));

            break;
        }
        case DfuService::COMMAND_INITDFUPARAMETERS: {
            //Send Init packet to control point
            m_service->writeValue(DfuService::UUID_CHARACTERISTIC_DFU_CONTROL, UCHAR_TO_BYTEARRAY(DfuService::COMMAND_RECEIVE_FIRMWARE_IMAGE));

            //Send the firmware
            sendFirmwareData();
            if (m_transferError) {
                m_service->message(QObject::tr("Update operation failed"));
                return true;
            }
            break;
        }
        case DfuService::COMMAND_RECEIVE_FIRMWARE_IMAGE: {
            //Send Validate command
            m_service->writeValue(DfuService::UUID_CHARACTERISTIC_DFU_CONTROL, UCHAR_TO_BYTEARRAY(DfuService::COMMAND_VALIDATE_FIRMWARE));
            break;
        }
        case DfuService::COMMAND_VALIDATE_FIRMWARE: {
            //Send Validate command
            m_service->writeValue(DfuService::UUID_CHARACTERISTIC_DFU_CONTROL, UCHAR_TO_BYTEARRAY(DfuService::COMMAND_ACTIVATE_IMAGE_AND_RESET));
            //Finish operation
            return true;
        }
        default: {
            qDebug() << "Unexpected response during firmware update: ";
            //operationFailed();
            m_service->message(QObject::tr("Update operation failed"));
            return true;
        }
        }
        return false;
    } else  if (value[0] == DfuService::COMMAND_PACKET_RECEIPT_NOTIFICATION) {
        int bytesReceived = TypeConversion::toUint32(value[1], value[2], value[3], value[4]);
        qDebug() << "Bytes received by watch: " << bytesReceived;
        m_outstandingPacketNotifications--;

        //Use the data from the notification to inform the UI on progress
        int progressPercent = (int) ((((float) bytesReceived) / m_uncompressedFwBytes.length()) * 100);
        DfuService *serv = qobject_cast<DfuService*>(m_service);
        serv->downloadProgress(progressPercent);
        serv->setWaitForWatch(false);
        return false;
    }

    qDebug() << "Unexpected notification during firmware update: ";
    m_service->message(QObject::tr("Update operation failed, unexpected metadata"));
    return true;

}

void DfuOperation::handleData(const QByteArray &data)
{
    Q_UNUSED(data);
}

bool DfuOperation::sendFwInfo()
{
    //uint16_t deviceType
    //uint16_t deviceRevision
    //uint32_t applicationVersion
    //uint16_t softdeviceArrayLength
    //uint16_t sd[softdeviceArrayLength];
    //uint16_t expectedCr
    QByteArray info;

    info += TypeConversion::fromInt16(0);
    info += TypeConversion::fromInt16(0);
    info += TypeConversion::fromInt32(0);
    info += TypeConversion::fromInt16(0);
    info += TypeConversion::fromInt16(m_crc16);
    m_service->writeValue(DfuService::UUID_CHARACTERISTIC_DFU_PACKET, info);
    return true;
}

void DfuOperation::sendFirmwareData()
{
    m_worker = new DfuWorker();
    m_worker->moveToThread(&m_workerThread);
    QObject::connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    QObject::connect(this, &DfuOperation::sendFirmware, m_worker, &DfuWorker::sendFirmware);
    //connect(m_worker, &DfuWorker::done, this, &Controller::handleResults);
    m_workerThread.start();
    DfuService *serv = qobject_cast<DfuService*>(m_service);
    emit sendFirmware(serv, m_uncompressedFwBytes, m_notificationPackets);
}

void DfuOperation::packetNotification()
{
    m_outstandingPacketNotifications++;
    if (m_outstandingPacketNotifications > 3) { //Watch hasnt responded
        qDebug() << Q_FUNC_INFO << "Watch has missed 3 response packets, aborting";
        m_workerThread.requestInterruption();
    }
}
