#include "zepposagpsupdateoperation.h"


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
}

void ZeppOsAgpsUpdateOperation::agpsUpdateFinishResponse(bool success)
{
    qDebug() << Q_FUNC_INFO << success;
    //TODO nned to trigger config service update
}

void ZeppOsAgpsUpdateOperation::fileUploadFinish(bool success)
{
    qDebug() << Q_FUNC_INFO << success;
    m_agpsService->startUpdate();
}

void ZeppOsAgpsUpdateOperation::fileUploadProgress(int progress)
{
    qDebug() << Q_FUNC_INFO << progress;
}

void ZeppOsAgpsUpdateOperation::fileDownloadFinish(const QString &url, const QString &filename, const QByteArray &data)
{
    qDebug() << Q_FUNC_INFO;
}
