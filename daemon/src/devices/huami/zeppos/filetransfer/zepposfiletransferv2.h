#ifndef ZEPPOSFILETRANSFERV2_H
#define ZEPPOSFILETRANSFERV2_H

#include "zepposfiletransferimpl.h"

#include <cstdint>

class ZeppOsFileTransferV2 : public ZeppOsFileTransferImpl
{
public:
    ZeppOsFileTransferV2(ZeppOsFileTransferService *fileTransferService, ZeppOSDevice *device);

    void characteristicChanged(const QString &characteristic, const QByteArray &value) override;
    void handlePayload(const QByteArray &payload) override;
    void uploadFile(Request *request) override;
    void handleFileDownloadRequest(uint8_t session, Request *request) override;

private:
    void writeChunk(uint8_t session, Request *request);
    void handleFileTransferData(const QByteArray &payload);
    void sendNextChunk(const uint8_t session);
    void onUploadFinish(uint8_t session, bool success);

    QMap<uint8_t, Request*> m_sessionRequests;

};

#endif // ZEPPOSFILETRANSFERV2_H
