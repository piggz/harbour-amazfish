#ifndef ZEPPOSFILETRANSFERV3_H
#define ZEPPOSFILETRANSFERV3_H

#include "zepposfiletransferimpl.h"

#include <cstdint>

class ZeppOsFileTransferV3 : public ZeppOsFileTransferImpl
{
public:
    static constexpr uint8_t CMD_DATA_V3_SEND = 0x12;
    static constexpr uint8_t CMD_DATA_V3_ACK = 0x13;
    static constexpr uint8_t CMD_CANCEL_TRANSFER_REQ = 0x20; // 20:01
    static constexpr uint8_t CMD_CANCEL_TRANSFER_ACK = 0x21; // 21:01:00

    static constexpr uint64_t TRANSFER_TIMEOUT_THRESHOLD_MILLIS = 5000;

    ZeppOsFileTransferV3(ZeppOsFileTransferService *fileTransferService, ZeppOSDevice *device);

    void characteristicChanged(const QString &characteristic, const QByteArray &value) override;
    void handlePayload(const QByteArray &payload) override;
    void uploadFile(Request *request) override;
    void handleFileDownloadRequest(uint8_t session, Request *request) override;

private:
    void writeChunk(Request *request);
    void handleFileReceiveData(const QByteArray &payload);
    void sendNextQueuedData();
    void onUploadFinish(bool success);
    void resetSend();
    void resetReceive();

    uint8_t nextSession = 0;

    Request *m_currentSendRequest = nullptr;
    int8_t m_currentSendSession = -1;
    int64_t m_lastSendActivityMillis = -1;

    Request *m_currentReceiveRequest = nullptr;
    int8_t m_currentReceiveSession = -1;
    int64_t m_lastReceiveActivityMillis = -1;

    int32_t m_currentReceiveChunkSize = -1;
    bool m_currentReceiveChunkIsLast = false;
    QByteArray m_receivePacketBuffer;

};

#endif // ZEPPOSFILETRANSFERV3_H
