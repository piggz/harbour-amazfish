#include "zepposfiletransferservice.h"
#include "qbleservice.h"
#include "huami/zeppos/filetransfer/zepposfiletransferv3.h"

ZeppOsFileTransferService::ZeppOsFileTransferService(ZeppOSDevice *device) : AbstractZeppOsService(device, false)
{
    m_endpoint = 0x000d;
}

void ZeppOsFileTransferService::handlePayload(const QByteArray &payload)
{
    qDebug() << Q_FUNC_INFO << payload;
    if (m_impl) {
        m_impl->handlePayload(payload);
        return;
    }

    if (payload[0] != ZeppOsFileTransferImpl::CMD_CAPABILITIES_RESPONSE) {
        qDebug() << "Got file transfer command, but impl is not initialized";
        return;
    }
    int version = payload[1] & 0xff;
    if (version == 1 || version == 2) {
        qDebug() << "Unsupported file transfer service version: " << version;
        return;
    } else if (version == 3) {
        m_impl = new ZeppOsFileTransferV3(this, m_device);
    } else {
        qDebug() << "Unsupported file transfer service version: " << version;
        return;
    }

    m_impl->handlePayload(payload);
}

QString ZeppOsFileTransferService::name() const
{
    return "FileTransfer";
}

void ZeppOsFileTransferService::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    if (!m_impl) {
        qDebug() << "Service not initialized, ignoring characteristic change for " << characteristic;
        return;
    }

    m_impl->characteristicChanged(characteristic, value);
}

void ZeppOsFileTransferService::sendFile(const QString &url, const QString &filename, const QByteArray &bytes, bool compress, Callback *callback)
{
    qDebug() << Q_FUNC_INFO;

    if (!m_impl) {
        qDebug() << "Service not initialized, refusing to send " << url;
        callback->fileUploadFinish(false);
        return;
    }

    m_impl->uploadFile(url, filename, bytes, compress, callback);
}

void ZeppOsFileTransferService::initialize()
{
    qDebug() << Q_FUNC_INFO;

    QByteArray data;
    data += UCHARVAL_TO_BYTEARRAY(ZeppOsFileTransferImpl::CMD_CAPABILITIES_REQUEST);
    write(data);
}
