#include "zepposagpsservice.h"
#include "typeconversion.h"
#include "qbleservice.h"

ZeppOsAgpsService::ZeppOsAgpsService(ZeppOSDevice *device) : AbstractZeppOsService(device, false)
{
    qDebug() << Q_FUNC_INFO;
    m_endpoint = 0x0042;
}

void ZeppOsAgpsService::handlePayload(const QByteArray &payload)
{
    qDebug() << Q_FUNC_INFO;

    uint8_t status = payload[1];

    int idx = 1;
    int size = 0;
    int progress = 0;

    if (payload[0] == CMD_UPDATE_PROGRESS_RESPONSE)
    {
        size = TypeConversion::toUint32(payload, idx);
        progress = TypeConversion::toUint32(payload, idx);
    }

    switch (payload[0]) {
    case CMD_UPDATE_START_UPLOAD_RESPONSE:
        qDebug() << "Got agps start upload status = " << status;
        if (m_callback) {
            m_callback->agpsUploadStartResponse(status == 0x01);
        }
        break;
    case CMD_UPDATE_PROGRESS_RESPONSE:
        qDebug() << "Got agps progress = " << progress << "/" << size;
        if (m_callback) {
            m_callback->agpsProgressResponse(size, progress);
        }
        break;
    case CMD_UPDATE_FINISH_RESPONSE:
        qDebug() << "Got agps update finish status = " << status;
        if (m_callback) {
            m_callback->agpsUpdateFinishResponse(status == 0x01);
        }
        break;
    default:
        qDebug() << "Unexpected agps byte " << payload[0];
    }
}

QString ZeppOsAgpsService::name() const
{
    return "Agps";
}

void ZeppOsAgpsService::startUpdate()
{
    qDebug() << Q_FUNC_INFO;

    write(UCHAR_TO_BYTEARRAY(CMD_UPDATE_START_REQUEST));
}

void ZeppOsAgpsService::startUpload(int size)
{
    qDebug() << Q_FUNC_INFO;

    QByteArray cmd;

    cmd += CMD_UPDATE_START_UPLOAD_REQUEST;
    cmd += TypeConversion::fromInt32(size);

    write(cmd);
}

void ZeppOsAgpsService::setCallback(Callback *callback)
{
    m_callback = callback;
}
