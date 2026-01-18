#ifndef ZEPPOSAGPSUPDATEOPERATION_H
#define ZEPPOSAGPSUPDATEOPERATION_H

#include "abstractzepposoperation.h"
#include "huami/zeppos/zepposagpsservice.h"
#include "huami/zeppos/zepposfiletransferservice.h"
#include <QString>

class ZeppOsAgpsUpdateOperation : public AbstractZeppOsOperation, public ZeppOsAgpsService::Callback, public ZeppOsFileTransferService::Callback
{
public:
    ZeppOsAgpsUpdateOperation(ZeppOSDevice *device, const QByteArray &data, ZeppOsAgpsService *agpsService, ZeppOsFileTransferService *fileTransferService);

    //AGPS Callback Interface
    void agpsUploadStartResponse(bool success) override;
    void agpsProgressResponse(int size, int progress) override;
    void agpsUpdateFinishResponse(int status) override;

    //File Transfer Callback Interface
    void fileUploadFinish(bool success) override;
    void fileUploadProgress(int progress) override;
    void fileDownloadFinish(const QString &url, const QString &filename, const QByteArray &data) override;

protected:
    void doPerform() override;

private:
    const QString AGPS_UPDATE_URL = "agps://upgrade";
    const QString AGPS_UPDATE_FILE = "uih.bin";
    QByteArray m_fileBytes;

    ZeppOSDevice *m_device = nullptr;
    ZeppOsAgpsService *m_agpsService = nullptr;
    ZeppOsFileTransferService *m_fileTransferService = nullptr;
    //ZeppOsConfigService *m_configService = nullptr;

    void finished(bool success);
};

#endif // ZEPPOSAGPSUPDATEOPERATION_H
