#ifndef ZEPPOSAGPSSERVICE_H
#define ZEPPOSAGPSSERVICE_H

#include "abstractzepposservice.h"

class ZeppOsAgpsService : public AbstractZeppOsService
{
public:
    static constexpr uint8_t CMD_UPDATE_START_UPLOAD_REQUEST = 0x03;
    static constexpr uint8_t CMD_UPDATE_START_UPLOAD_RESPONSE = 0x04;
    static constexpr uint8_t CMD_UPDATE_START_REQUEST = 0x05;
    static constexpr uint8_t CMD_UPDATE_PROGRESS_RESPONSE = 0x06;
    static constexpr uint8_t CMD_UPDATE_FINISH_RESPONSE = 0x07;

    class Callback {
    public:
        virtual void agpsUploadStartResponse(bool success) = 0;
        virtual void agpsProgressResponse(int size, int progress) = 0;
        virtual void agpsUpdateFinishResponse(bool success) = 0;
    };

    ZeppOsAgpsService(ZeppOSDevice *device);

    void handlePayload(const QByteArray &data) override;
    QString name() const override;

    void startUpdate();
    void startUpload(int size);

    void setCallback(Callback *callback);

private:
    Callback * m_callback = nullptr;
};

#endif // ZEPPOSAGPSSERVICE_H
