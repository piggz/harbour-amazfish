#ifndef ZEPPOSFILETRANSFERSERVICE_H
#define ZEPPOSFILETRANSFERSERVICE_H

#include "huami/zeppos/abstractzepposservice.h"

class ZeppOsDevice;
class ZeppOsFileTransferImpl;

class ZeppOsFileTransferService : public AbstractZeppOsService
{
public:
    class Callback {
    public:
        virtual void fileUploadFinish(bool success) = 0;
        virtual void fileUploadProgress(int progress) = 0;
        virtual void fileDownloadFinish(const QString &url, const QString &filename, const QByteArray &data) = 0;
    };

    class DownlodCallback : public Callback {
    public:
        void fileUploadFinish(bool success) override;
        void fileUploadProgress(int progress) override;
        void fileDownloadFinish(const QString &url, const QString &filename, const QByteArray &data) override;
    };

    class UploadCallback : public Callback {
    public:
        void fileUploadFinish(bool success) override;
        void fileUploadProgress(int progress) override;
        void fileDownloadFinish(const QString &url, const QString &filename, const QByteArray &data) override;
    };

    ZeppOsFileTransferService(ZeppOSDevice *device);

    void handlePayload(const QByteArray &payload) override;
    QString name() const override;

    void characteristicChanged(const QString &characteristic, const QByteArray &value);
    void sendFile(const QString &url, const QString &filename, const  QByteArray &bytes, bool compress, Callback *callback);

    void initialize() override;
private:
    ZeppOsFileTransferImpl *m_impl = nullptr;

    friend class ZeppOsFileTransferV3;
};

#endif // ZEPPOSFILETRANSFERSERVICE_H
