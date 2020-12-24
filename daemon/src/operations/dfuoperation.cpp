#include "dfuoperation.h"
#include "dfuservice.h"
#include "typeconversion.h"
#include <QApplication>

DfuOperation::DfuOperation(const AbstractFirmwareInfo *info, QBLEService *service) : AbstractOperation(service), m_info(info)
{
    m_fwBytes = info->bytes();
}

void DfuOperation::start()
{
    qDebug() << Q_FUNC_INFO;
    if (m_info->type() != AbstractFirmwareInfo::Invalid) {
        DfuService *serv = dynamic_cast<DfuService*>(m_service);

        m_service->enableNotification(serv->UUID_CHARACTERISTIC_DFU_CONTROL);

        //Send Start command to control point
        m_service->writeValue(DfuService::UUID_CHARACTERISTIC_DFU_CONTROL, UCHAR_TO_BYTEARRAY(DfuService::COMMAND_STARTDFU) + QByteArray(1, 0x04));

        //Send byte count to packet point <Length of SoftDevice><Length of bootloader><Length of application> each uint32
        QByteArray lengths;
        lengths += TypeConversion::fromInt32(0);
        lengths += TypeConversion::fromInt32(0);
        lengths += TypeConversion::fromInt32(m_fwBytes.length());
        m_service->writeValue(DfuService::UUID_CHARACTERISTIC_DFU_PACKET, lengths);

        //Send packet request command to control point
        m_service->writeValue(DfuService::UUID_CHARACTERISTIC_DFU_CONTROL, UCHAR_TO_BYTEARRAY(DfuService::COMMAND_PACKET_RECEIPT_NOTIFICATION_REQUEST) + QByteArray(1, m_notificationPackets));

    } else {
        m_service->message(QObject::tr("File does not seem to be supported"));
    }
}

bool DfuOperation::handleMetaData(const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << value;

    if (!(value.length() == 3 || value.length() == 5)) {
        qDebug() << "Notifications should be 3 or 5 bytes long.";
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
            break;
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
    info += TypeConversion::fromInt16(m_info->getCrc16());
    m_service->writeValue(DfuService::UUID_CHARACTERISTIC_DFU_PACKET, info);
    return true;
}

void DfuOperation::sendFirmwareData()
{
    int len = m_fwBytes.length();
    int packetLength = 20;
    int packets = len / packetLength;

    // going from 0 to len
    int firmwareProgress = 0;
    int progressPercent = 0;

    DfuService *serv = dynamic_cast<DfuService*>(m_service);

    m_busy = true;
    for (int i = 0; i < packets; i++) {
        QByteArray fwChunk = m_fwBytes.mid(i * packetLength, packetLength);

        if (m_service && !m_transferError) {
            m_service->writeValue(DfuService::UUID_CHARACTERISTIC_DFU_PACKET, fwChunk);
            firmwareProgress += packetLength;
        } else {
            qDebug() << "Service has gone!!";
            m_transferError = true;
            break;
        }
        progressPercent = (int) ((((float) firmwareProgress) / len) * 100);
        if ((i > 0) && (i % m_notificationPackets == 0)) {
            m_outstandingPacketNotifications++;
            serv->downloadProgress(progressPercent);
            QThread::msleep(500);
            QApplication::processEvents();

            if (m_outstandingPacketNotifications > 3) { //Device stopped responding
                m_transferError = true;
                break;
            }
        }
    }
    m_busy = false;

    if (!m_transferError) {
        if (firmwareProgress < len) {
            QByteArray lastChunk = m_fwBytes.mid(packets * packetLength);
            m_service->writeValue(DfuService::UUID_CHARACTERISTIC_DFU_PACKET, lastChunk);
        }

        qDebug() << "Finished sending firmware";
        serv->downloadProgress(100);
    }
}
