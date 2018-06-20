#include "updatefirmwareoperation.h"
#include "bipfirmwareservice.h"
#include "typeconversion.h"

UpdateFirmwareOperation::UpdateFirmwareOperation(const QString &path, QBLEService *service) : AbstractOperation(service)
{
    qDebug() << "Sending file " << path;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;
    m_fwBytes = file.readAll();

    m_info = new HuamiFirmwareInfo(m_fwBytes);
}

void UpdateFirmwareOperation::start()
{
    if (m_info->type() == HuamiFirmwareInfo::Watchface) {
        BipFirmwareService *serv = dynamic_cast<BipFirmwareService*>(m_service);

        m_service->enableNotification(serv->UUID_CHARACTERISTIC_FIRMWARE);

        if (!sendFwInfo()) {
            m_service->message("Error sending firmware info, aborting.");
            //done();
        }
    }
}

bool UpdateFirmwareOperation::handleMetaData(const QByteArray &value)
{
    BipFirmwareService *serv = dynamic_cast<BipFirmwareService*>(m_service);

    if (value.length() != 3) {
        qDebug() << "Notifications should be 3 bytes long.";
        return true;
    }
    bool success = value[2] == serv->SUCCESS;

    if (value[0] == serv->RESPONSE && success) {
        switch (value[1]) {
        case BipFirmwareService::COMMAND_FIRMWARE_INIT: {
            sendFirmwareData(getFirmwareInfo());
            break;
        }
        case BipFirmwareService::COMMAND_FIRMWARE_START_DATA: {
            sendChecksum(getFirmwareInfo());
            break;
        }
        case BipFirmwareService::COMMAND_FIRMWARE_CHECKSUM: {
            if (getFirmwareInfo().getFirmwareType() == HuamiFirmwareType.FIRMWARE) {
                getSupport().sendReboot(builder);
            } else {
                GB.updateInstallNotification(getContext().getString(R.string.updatefirmwareoperation_update_complete), false, 100, getContext());
                done();
            }
            break;
        }
        case BipFirmwareService::COMMAND_FIRMWARE_REBOOT: {
            qDebug() << "Reboot command successfully sent.";

            done();
            break;
        }
        default: {
            qDebug() << "Unexpected response during firmware update: ";
            operationFailed();
            displayMessage(getContext(), getContext().getString(R.string.updatefirmwareoperation_updateproblem_do_not_reboot), Toast.LENGTH_LONG, GB.ERROR);
            done();
            return;
        }
        }

    } else {
        LOG.error("Unexpected notification during firmware update: ");
        operationFailed();
        getSupport().logMessageContent(value);
        displayMessage(getContext(), getContext().getString(R.string.updatefirmwareoperation_metadata_updateproblem), Toast.LENGTH_LONG, GB.ERROR);
        done();
    }

}

void UpdateFirmwareOperation::handleData(const QByteArray &data)
{
    
}

bool UpdateFirmwareOperation::sendFwInfo() {

    BipFirmwareService *serv = dynamic_cast<BipFirmwareService*>(m_service);

    int fwSize = m_fwBytes.size();
    QByteArray sizeBytes = TypeConversion::fromInt24(fwSize);
    int arraySize = 4;
    bool isFirmwareCode = m_info->type() == HuamiFirmwareInfo::Firmware;
    if (!isFirmwareCode) {
        arraySize++;
    }
    QByteArray bytes(arraySize, 0x00);
    int i = 0;
    bytes[i++] = serv->COMMAND_FIRMWARE_INIT;
    bytes[i++] = sizeBytes[0];
    bytes[i++] = sizeBytes[1];
    bytes[i++] = sizeBytes[2];
    if (!isFirmwareCode) {
        bytes[i++] = m_info->type();
    }

    m_service->writeValue(serv->UUID_CHARACTERISTIC_FIRMWARE, bytes);
    return true;
}
