#include "zepposagpsupdateoperation.h"
#include "zepposdevice.h"

ZeppOsAgpsUpdateOperation::ZeppOsAgpsUpdateOperation(ZeppOSDevice *device, const QByteArray &data, ZeppOsAgpsService *agpsService, ZeppOsFileTransferService *fileTransferService) :
    m_device(device), m_agpsService(agpsService), m_fileTransferService(fileTransferService)
{
    m_fileBytes = data;
}

void ZeppOsAgpsUpdateOperation::doPerform()
{
    m_agpsService->setCallback(this);
    m_agpsService->startUpload(m_fileBytes.length());
}

void ZeppOsAgpsUpdateOperation::finished(bool success)
{
    m_agpsService->setCallback(nullptr);
    operationFinished();
}

void ZeppOsAgpsUpdateOperation::agpsUploadStartResponse(bool success)
{
    qDebug() << Q_FUNC_INFO;
    if (!success) {
        finished(false);
        return;
    }

    m_fileTransferService->sendFile(AGPS_UPDATE_URL, AGPS_UPDATE_FILE, m_fileBytes, false, this);
}

void ZeppOsAgpsUpdateOperation::agpsProgressResponse(int size, int progress)
{
    qDebug() << Q_FUNC_INFO << size << progress;
    m_device->downloadProgress(progress / ( size / 100.0));
}

void ZeppOsAgpsUpdateOperation::agpsUpdateFinishResponse(int status)
{
    qDebug() << Q_FUNC_INFO << status;

    if (status == 0x01) { //success
        //TODO nned to trigger config service update
        m_device->message(QIODevice::tr("AGPS update Ok"));
    } else if (status == 0x10) { //corrupt file
        m_device->message(QIODevice::tr("AGPS File is corrupt"));
    } else if (status  == 0x11) {
        m_device->message(QIODevice::tr("AGPS File is not the correct format"));
    } else {
        m_device->message(QIODevice::tr("Unknown error sending AGPS file"));
    }

}

void ZeppOsAgpsUpdateOperation::fileUploadFinish(bool success)
{
    qDebug() << Q_FUNC_INFO << success;
    if (success) {
        m_device->message(QIODevice::tr("File upload Ok.  Performing on-device update"));
        m_agpsService->startUpdate();
    } else {
        m_device->message(QIODevice::tr("File upload failed"));
    }
}

void ZeppOsAgpsUpdateOperation::fileUploadProgress(int progress)
{
    qDebug() << Q_FUNC_INFO << progress;
    m_device->downloadProgress(progress);
}

void ZeppOsAgpsUpdateOperation::fileDownloadFinish(const QString &url, const QString &filename, const QByteArray &data)
{
    qDebug() << Q_FUNC_INFO;
}
