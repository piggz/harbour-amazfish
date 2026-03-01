#include "zepposfiletransferv2.h"
#include "qbleservice.h"
#include "typeconversion.h"
#include <qdatetime.h>
#include "mibandservice.h"
#include "huami/zepposdevice.h"
#include "bipfirmwareservice.h"

ZeppOsFileTransferV2::ZeppOsFileTransferV2(ZeppOsFileTransferService *fileTransferService, ZeppOSDevice *device) : ZeppOsFileTransferImpl(fileTransferService, device)
{

}

void ZeppOsFileTransferV2::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qWarning() << Q_FUNC_INFO << "Unknown characteristic changed: " << characteristic << " value:" << value.toHex();
}

void ZeppOsFileTransferV2::handlePayload(const QByteArray &payload)
{
    qDebug() << Q_FUNC_INFO;

    uint8_t session;
    uint8_t status;

    switch (payload[0]) {
    case CMD_TRANSFER_RESPONSE: {
        session = payload[1];
        status = payload[2];
        int idx = 3;
        const int existingProgress = TypeConversion::toUint32(payload, idx);
        qInfo() << "Band acknowledged file transfer request: session " << session << ", status " << status << ", existingProgress " << existingProgress;
        if (status != 0) {
            qWarning() << "Unexpected status from band for session " << session << ", aborting";
            onUploadFinish(session, false);
            return;
        }
        if (existingProgress != 0) {
            qInfo() << "Updating existing progress for session " << session << " " << existingProgress;
            auto request = m_sessionRequests.value(session);
            if (request == nullptr) {
                qWarning() << "No request found for session " << session;
            }
            request->setProgress(existingProgress);
        }
        sendNextChunk(session);
        return;
    }
    case CMD_DATA_SEND:
        handleFileTransferData(payload);
        return;
    case CMD_DATA_ACK:
        session = payload[1];
        status = payload[2];
        qInfo() << "Band acknowledged file transfer data: session " << session << " status " << status;
        if (status != 0) {
            qWarning() << "Unexpected status from band, aborting session " << session;
            onUploadFinish(session, false);
            return;
        }
        sendNextChunk(session);
        return;
    default:
        ZeppOsFileTransferImpl::handlePayload(payload);
    }
}

void ZeppOsFileTransferV2::uploadFile(Request *request)
{
    qDebug() << Q_FUNC_INFO;

    if (request->compressed()) {
        qCritical("V1/V2 does not support compressed transfers");
    }

    uint8_t session = (uint8_t)m_sessionRequests.size();
    while (m_sessionRequests.count(session) > 0) {
        session++;
    }

    int payloadSize = 2 +
            request->url().toLocal8Bit().length() + 1 +
            request->filename().toLocal8Bit().length() + 1 +
            4 + 4;
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

    m_fileTransferService->write(buf);

    m_sessionRequests[session] = request;
}

void ZeppOsFileTransferV2::handleFileDownloadRequest(uint8_t session, Request *request)
{
    qDebug() << Q_FUNC_INFO << session;

    QByteArray buf;
    buf += UCHARVAL_TO_BYTEARRAY(CMD_TRANSFER_RESPONSE);
    buf += session;
    buf += UCHARVAL_TO_BYTEARRAY(0x00);
    buf += TypeConversion::fromInt32(0);

    m_fileTransferService->write(buf);

    m_sessionRequests[session] = request;
}

void ZeppOsFileTransferV2::writeChunk(uint8_t session, Request *request)
{
    qDebug() << Q_FUNC_INFO << request->chunkSize() << request->progress();
    QByteArray buf;
    buf += UCHARVAL_TO_BYTEARRAY(CMD_DATA_SEND);

    uint8_t flags = 0;
    if (request->progress() == 0) {
        flags |= FLAG_FIRST_CHUNK;
    }
    if (request->progress() + request->chunkSize() >= request->size()) {
        flags |= FLAG_LAST_CHUNK;
    }

    int partSize = 127; // TODO mtu hardcoded

    buf += UCHARVAL_TO_BYTEARRAY(flags);
    buf += UCHARVAL_TO_BYTEARRAY(session);
    buf += UCHARARR_TO_BYTEARRAY(request->index());
    if ((flags & FLAG_FIRST_CHUNK) > 0) {
        buf += UCHARARR_TO_BYTEARRAY(0); // ?
        buf += UCHARARR_TO_BYTEARRAY(0); // ?
        buf += UCHARARR_TO_BYTEARRAY(0); // ?
        buf += UCHARARR_TO_BYTEARRAY(0); // ?
    }


    QByteArray chunk = request->bytes().mid(request->progress(), request->chunkSize());
    buf += TypeConversion::fromInt16(chunk.length());
    buf += chunk;

    request->setProgress(request->progress() + chunk.length());
    request->setIndex(request->index() + 1);
    request->callback()->fileUploadProgress(request->progress() / (request->size() / 100.0));

    m_fileTransferService->write(buf);
}

void ZeppOsFileTransferV2::handleFileTransferData(const QByteArray &payload)
{
    qDebug() << Q_FUNC_INFO;
    {
        int i = 0;
        uint8_t flags = payload[++i];
        bool firstChunk = (flags & FLAG_FIRST_CHUNK) != 0;
        bool lastChunk = (flags & FLAG_LAST_CHUNK) != 0;
        uint8_t session = payload[++i];
        uint8_t index = payload[++i];

        if (firstChunk) {
            i+=4; // ?
        }

        const uint16_t size = TypeConversion::toUint16(payload, ++i);
        i+=2;

        auto request = m_sessionRequests[session];
        if (request == nullptr) {
            qCritical() << "No download request found for V1 session " << session;
            return;
        }

        if (index != request->index()) {
            qCritical() << "Unexpected index " << index << " expected " << request->index();
            return;
        }

        if (firstChunk && request->progress() != 0) {
            qCritical() << "Got first packet, but progress is " << request->progress();
            return;
        }

        request->bytes() += payload.mid(i);
        request->setIndex(index + 1);
        request->setProgress(request->progress() + size);


        QByteArray buf;
        buf += UCHARVAL_TO_BYTEARRAY(CMD_DATA_ACK);
        buf += UCHARVAL_TO_BYTEARRAY(session);
        buf += UCHARVAL_TO_BYTEARRAY(0x00);

        m_fileTransferService->write(buf);

        if (lastChunk) {
            m_sessionRequests.remove(session);

            QByteArray data;
            if (request->compressed()) {
                data = Amazfish::unCompressData(request->bytes());
                if (data.isEmpty()) {
                    qDebug() << "Failed to decompress V123 bytes for " << request->filename();
                    return;
                }
            } else {
                data = request->bytes();
            }

            int checksum = Amazfish::calculateCRC32(data);
            if (checksum != request->crc32()) {
                qDebug() << "Checksum mismatch: expected " << request->crc32() << " got " << checksum;
                return;
            }

            request->callback()->fileDownloadFinish(
                        request->url(),
                        request->filename(),
                        data
                        );
            delete request; // TODO
        }

    }
}

void ZeppOsFileTransferV2::sendNextChunk(const uint8_t session)
{
    Request* request = m_sessionRequests.value(session);
    if (!request) {
        qWarning() << "No request found for session " << session << " to send next chunk";
        return;
    }

    if (request->progress() >= request->size()) {
        qDebug() << "Finished sending: " <<  request->url();
        onUploadFinish(session, true);
        return;
    }

    qDebug() << "Sending file data for session " << session << " progress " << request->progress() << " index " << request->index();

    writeChunk(session, request);
}

void ZeppOsFileTransferV2::onUploadFinish(uint8_t session, bool success)
{
    auto request = m_sessionRequests.value(session);
    if (!request) {
        qDebug() << "No request found for session to finish upload:" <<  request;
        return;
    }

    m_sessionRequests.remove(session);

    request->callback()->fileUploadFinish(success);
}
