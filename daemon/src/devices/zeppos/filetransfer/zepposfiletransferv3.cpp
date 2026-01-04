#include "zepposfiletransferv3.h"
#include "qbleservice.h"
#include "typeconversion.h"
#include <qdatetime.h>
#include "mibandservice.h"
#include "zepposdevice.h"

ZeppOsFileTransferV3::ZeppOsFileTransferV3(ZeppOsFileTransferService *fileTransferService, ZeppOSDevice *device) : ZeppOsFileTransferImpl(fileTransferService, device)
{

}

void ZeppOsFileTransferV3::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << characteristic << value.toHex();

    if (characteristic == BipFirmwareService::UUID_CHARACTERISTIC_ZEPP_OS_FILE_TRANSFER_V3_RECEIVE) {
        handleFileReceiveData(value);
    } else if (characteristic == BipFirmwareService::UUID_CHARACTERISTIC_ZEPP_OS_FILE_TRANSFER_V3_SEND) {
        if (value[0] != CMD_DATA_V3_ACK) {
            qDebug() << "Got non-ack on file send characteristic";
            return;
        }

        uint8_t status = value[1];
        int chunkIndex = value[2] & 0xff;
        uint8_t unk1 = value[3]; // 1/2?

        qDebug()<< "Band acknowledged file transfer data: chunk:" << chunkIndex;

        if (!m_currentSendRequest) {
            qDebug() << "Got ack for file send, but we are not uploading";
            return;
        }

        if (status != 0) {
            qDebug() << "Unexpected status from band, aborting session {}" << m_currentSendSession;
            onUploadFinish(false);
            return;
        }

        if (m_currentSendRequest->index() - 1 != chunkIndex) {
            qDebug() << "Got ack for unexpected chunk index " << chunkIndex << " expected " << m_currentSendRequest->index() - 1;
            onUploadFinish(false);
            return;
        }

        sendNextQueuedData();
    } else {
        qDebug() << "Unknown characteristic changed: " << characteristic;
    }

}

void ZeppOsFileTransferV3::handlePayload(const QByteArray &payload)
{
    qDebug() << Q_FUNC_INFO;

    uint8_t session = payload[1];
    uint8_t status = payload[2];
    int idx = 3;
    int existingProgress = TypeConversion::toUint32(payload, idx);

    switch (payload[0]) {
    case CMD_TRANSFER_RESPONSE:
        qDebug() << "Band acknowledged file transfer request";
        if (!m_currentSendRequest) {
            qDebug() << "No ongoing send request found";
            return;
        }
        if (status != 0) {
            qDebug() << "Unexpected status from band for session, aborting:" << session;
            onUploadFinish(false);
            resetSend();
            return;
        }
        if (m_currentSendSession != session) {
            qDebug() << "Unexpected send session from band " << session << " expected " << m_currentSendSession << " aborting";
            onUploadFinish(false);
            resetSend();
            return;
        }
        if (existingProgress != 0) {
            qDebug() << "Updating existing progress for session " << session << " to " << existingProgress;
            m_currentSendRequest->setProgress(existingProgress);
        }
        sendNextQueuedData();
        return;
    default:
        ZeppOsFileTransferImpl::handlePayload(payload);
    }
}

void ZeppOsFileTransferV3::uploadFile(Request *request)
{
    qDebug() << Q_FUNC_INFO;

    if (m_currentSendRequest) {
        if (QDateTime::currentMSecsSinceEpoch() - m_lastSendActivityMillis < TRANSFER_TIMEOUT_THRESHOLD_MILLIS) {
            qDebug() << "Already uploading {}" << m_currentSendRequest->filename();
            request->callback()->fileUploadFinish(false);
            return;
        }

        qDebug() << "Timing out existing upload request for " << m_currentSendRequest->filename();
        m_currentSendRequest->callback()->fileUploadFinish(false);
        resetSend();
    }

    uint8_t session = nextSession++;

    int payloadSize = 2 +
            request->url().toLocal8Bit().length() + 1 +
            request->filename().toLocal8Bit().length() + 1 +
            4 + 4 + 2;
    if (request->compressed()) {
        payloadSize += 4;
    }

    QByteArray buf;

    buf += CMD_TRANSFER_REQUEST;
    buf += session;
    buf += request->url().toLocal8Bit();
    buf += uint8_t(0x00);
    buf += request->filename().toLocal8Bit();
    buf += uint8_t(0x00);
    buf += TypeConversion::fromInt32(request->rawLength());
    buf += TypeConversion::fromInt32(request->crc32());
    buf += UCHARVAL_TO_BYTEARRAY(request->compressed() ? 0x01 : 0x00);

    if (request->compressed()) {
        buf += TypeConversion::fromInt32(mCompressedChunkSize);
    }
    buf += UCHARVAL_TO_BYTEARRAY(0x00);

    m_fileTransferService->write(buf);

    m_currentSendSession = session;
    m_currentSendRequest = request;
    m_lastSendActivityMillis = QDateTime::currentMSecsSinceEpoch();
}

void ZeppOsFileTransferV3::handleFileDownloadRequest(uint8_t session, Request *request)
{
    qDebug() << Q_FUNC_INFO << session;

    if (!m_currentReceiveRequest) {
        if (QDateTime::currentMSecsSinceEpoch() - m_lastReceiveActivityMillis < TRANSFER_TIMEOUT_THRESHOLD_MILLIS) {
            qDebug() << "Already downloading " <<  m_currentReceiveRequest->filename();
            // TODO how to send nack?
            return;
        }

        qDebug() << "Timing out existing download request for " << m_currentReceiveRequest->filename();
        m_currentReceiveRequest->callback()->fileUploadFinish(false);

        resetReceive();
    }

    QByteArray buf;
    buf += UCHARVAL_TO_BYTEARRAY(CMD_TRANSFER_RESPONSE);
    buf += session;
    buf += UCHARVAL_TO_BYTEARRAY(0x00);
    buf += TypeConversion::fromInt32(0);
    buf += UCHARVAL_TO_BYTEARRAY(0x00);

    m_fileTransferService->write(buf);

    m_currentReceiveChunkIsLast = false;
    m_currentReceiveChunkSize = -1;
    m_receivePacketBuffer.clear();
    m_currentReceiveSession = session;
    m_currentReceiveRequest = request;
    m_lastReceiveActivityMillis = QDateTime::currentMSecsSinceEpoch();
}

void ZeppOsFileTransferV3::writeChunk(Request *request)
{
    qDebug() << Q_FUNC_INFO << request->chunkSize() << request->progress();

    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(m_device->service(BipFirmwareService::UUID_SERVICE_FIRMWARE));
    if (!fw) {
        return;
    }

    QByteArray chunk = request->bytes().mid(request->progress(), request->chunkSize());

    uint8_t flags = 0;
    if (request->progress() == 0) {
        flags |= FLAG_FIRST_CHUNK;
    }
    if (request->progress() + request->chunkSize() >= request->size()) {
        flags |= FLAG_LAST_CHUNK;
    }

    int partSize = 127; // TODO mtu hardcoded

    QByteArray buf;
    buf += UCHARVAL_TO_BYTEARRAY(CMD_DATA_V3_SEND);
    buf += UCHARVAL_TO_BYTEARRAY(flags);
    buf += UCHARVAL_TO_BYTEARRAY(request->index());
    buf += TypeConversion::fromInt16(chunk.length());
    buf += chunk;

    for (int i = 0; i < buf.length(); i += partSize) {
        QByteArray part = buf.mid(i, partSize);
        fw->writeValue(BipFirmwareService::UUID_CHARACTERISTIC_ZEPP_OS_FILE_TRANSFER_V3_SEND, part);
    }

    request->setProgress(request->progress() + chunk.length());
    request->setIndex(request->index() + 1);
    request->callback()->fileUploadProgress(request->progress() / (request->size() / 100.0));
}

void ZeppOsFileTransferV3::handleFileReceiveData(const QByteArray &payload)
{
    qDebug() << Q_FUNC_INFO;

    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(m_device->service(BipFirmwareService::UUID_SERVICE_FIRMWARE));
    if (!fw) {
        return;
    }

    if (!m_currentReceiveRequest) {
        qDebug() << "No receive request found for V3 session";
        return;
    }

    m_lastReceiveActivityMillis = QDateTime::currentMSecsSinceEpoch();

    if (m_currentReceiveChunkSize > 0) {
        // We are currently receiving a chunk
        m_receivePacketBuffer += payload;
    } else {
        // Start of a chunk
        m_receivePacketBuffer.clear();

        int i = 0;
        uint8_t flags = payload[++i];
        bool firstChunk = (flags & FLAG_FIRST_CHUNK) != 0;

        m_currentReceiveChunkIsLast = (flags & FLAG_LAST_CHUNK) != 0;
        uint8_t index = payload[++i];
        m_currentReceiveChunkSize = TypeConversion::toUint16(payload, ++i);

        if (index != m_currentReceiveRequest->index()) {
            qDebug() << "Unexpected V3 index " << index << " expected " << m_currentReceiveRequest->index();
            return;
        }

        if (firstChunk && m_currentReceiveRequest->progress() != 0) {
            qDebug() << "Got first V3 packet, but progress is " << m_currentReceiveRequest->progress();
            return;
        }

        m_receivePacketBuffer += payload.mid(i);
    }

    if (m_receivePacketBuffer.size() >= m_currentReceiveChunkSize) {
        // Finished a chunk

        m_currentReceiveRequest->bytes() += m_receivePacketBuffer;
        /*System.arraycopy(
                    m_receivePacketBuffer,
                    0,
                    m_currentReceiveRequest,
                    m_currentReceiveRequest->progress(),
                    currentReceiveChunkSize
                    );*/

        m_currentReceiveRequest->setIndex(m_currentReceiveRequest->index() + 1);
        m_currentReceiveRequest->setProgress(m_currentReceiveRequest->progress() + m_currentReceiveChunkSize);


        QByteArray buf;
        buf += UCHARVAL_TO_BYTEARRAY(CMD_DATA_V3_ACK);
        buf += UCHARVAL_TO_BYTEARRAY(0x00);
        buf += UCHARVAL_TO_BYTEARRAY(m_currentReceiveRequest->index() - 1);
        buf += UCHARVAL_TO_BYTEARRAY(0x00);
        buf += UCHARVAL_TO_BYTEARRAY(0x00);
        buf += UCHARVAL_TO_BYTEARRAY(0x00);
        buf += UCHARVAL_TO_BYTEARRAY(0x00);

        fw->writeValue(BipFirmwareService::UUID_CHARACTERISTIC_ZEPP_OS_FILE_TRANSFER_V3_RECEIVE, buf);

        if (m_currentReceiveChunkIsLast) {
            QByteArray data;
            if (m_currentReceiveRequest->compressed()) {
                data = Amazfish::unCompressData(m_currentReceiveRequest->bytes());
                if (data.isEmpty()) {
                    qDebug() << "Failed to decompress V3 bytes for " << m_currentReceiveRequest->filename();
                    resetReceive();
                    return;
                }
            } else {
                data = m_currentReceiveRequest->bytes();
            }

            int checksum = Amazfish::calculateCRC32(data);
            if (checksum != m_currentReceiveRequest->crc32()) {
                qDebug() << "V3 Checksum mismatch: expected " << m_currentReceiveRequest->crc32() << " got " << checksum;
                resetReceive();
                return;
            }

            Request requestBackup = *m_currentReceiveRequest;

            resetReceive();

            requestBackup.callback()->fileDownloadFinish(
                        requestBackup.url(),
                        requestBackup.filename(),
                        data
                        );
        }

        m_currentReceiveChunkSize = -1;
        m_receivePacketBuffer.clear();
    }
}

void ZeppOsFileTransferV3::sendNextQueuedData()
{
    if (!m_currentSendRequest) {
        qDebug() << "No ongoing V3 send request found";
        return;
    }

    if (m_currentSendRequest->progress() >= m_currentSendRequest->size()) {
        qDebug() << "Finished sending: " <<  m_currentSendRequest->url();
        onUploadFinish(true);
        return;
    }

    m_lastSendActivityMillis = QDateTime::currentMSecsSinceEpoch();

    writeChunk(m_currentSendRequest);
}

void ZeppOsFileTransferV3::onUploadFinish(bool success)
{
    if (!m_currentSendRequest) {
        qDebug() << "No request found for session to finish upload:" <<  m_currentSendSession;
        return;
    }

    m_currentSendRequest->callback()->fileUploadFinish(success);

    resetSend();
}

void ZeppOsFileTransferV3::resetSend()
{
    m_currentSendSession = -1;
    delete m_currentSendRequest;
    m_currentSendRequest = nullptr;
    m_lastSendActivityMillis = -1;
}

void ZeppOsFileTransferV3::resetReceive()
{
    m_currentReceiveSession = -1;
    delete m_currentSendRequest;
    m_currentReceiveRequest = nullptr;
    m_lastReceiveActivityMillis = -1;
    m_currentReceiveChunkIsLast = false;
    m_currentReceiveChunkSize = -1;
    m_receivePacketBuffer.clear();
}
