
#include "zeppos/zepposfiletransferservice.h"
#include <QStringList>
#include <optional>

class ZeppOsFileTransferImpl {
public:

    class Request {
    public:
        Request(QString url,
                   const QString &filename,
                   int rawLength,
                   const QByteArray &bytes,
                   bool compressed,
                   int crc32,
                   int chunkSize,
                   ZeppOsFileTransferService::Callback *callback);

        QString url() const;
        void setUrl(const QString &newUrl);

        ZeppOsFileTransferService::Callback *callback() const;
        void setCallback(ZeppOsFileTransferService::Callback *newCallback);

        int progress() const;
        void setProgress(int newProgress);

        int size() const;

        QString filename() const;
        void setFilename(const QString &newFilename);

        int index() const;
        void setIndex(int newIndex);

        bool compressed() const;
        void setCompressed(bool newCompressed);

        int rawLength() const;
        void setRawLength(int newRawLength);

        int crc32() const;
        void setCrc32(int newCrc32);

        QByteArray &bytes();

        int chunkSize() const;
        void setChunkSize(int newChunkSize);

    private:
        QString m_url;
        QString m_filename;
        int m_rawLength = 0;
        QByteArray m_bytes;
        bool m_compressed = false;
        int m_crc32 = 0;
        int m_chunkSize = 0;
        ZeppOsFileTransferService::Callback *m_callback = nullptr;

        int m_progress = 0;
        int m_index = 0;

    };

    static constexpr uint8_t CMD_CAPABILITIES_REQUEST = 0x01;
    static constexpr uint8_t CMD_CAPABILITIES_RESPONSE = 0x02;
    static constexpr uint8_t CMD_TRANSFER_REQUEST = 0x03;
    static constexpr uint8_t CMD_TRANSFER_RESPONSE = 0x04;
    static constexpr uint8_t CMD_DATA_SEND = 0x10;
    static constexpr uint8_t CMD_DATA_ACK = 0x11;
    static constexpr uint8_t CMD_DATA_V3_SEND = 0x12;
    static constexpr uint8_t CMD_DATA_V3_ACK = 0x13;
    static constexpr uint8_t FLAG_FIRST_CHUNK = 0x01;
    static constexpr uint8_t FLAG_LAST_CHUNK = 0x02;
    static constexpr uint8_t FLAG_CRC = 0x04;
    const QString PREF_SUPPORTED_SERVICES = "zepp_os_file_transfer_supported_service";

    ZeppOsFileTransferService *m_fileTransferService = nullptr;
    ZeppOSDevice *m_device = nullptr;

    int mVersion = -1;
    int mChunkSize = -1;
    int mCompressedChunkSize = -1;
    QStringList supportedServices;

    ZeppOsFileTransferImpl(ZeppOsFileTransferService *fileTransferService, ZeppOSDevice *device);

    void uploadFile(const QString &url,
                    const QString &ilename,
                    const QByteArray &bytes,
                    bool compress,
                    ZeppOsFileTransferService::Callback *callback);

    virtual void handlePayload(const QByteArray &payload);

    virtual void uploadFile(Request *request) = 0;
    virtual void handleFileDownloadRequest(uint8_t session, Request *request) = 0;
    virtual void characteristicChanged(const QString &characteristic, const QByteArray &value) = 0;

private:
    void handleCapabilitiesResponse(const QByteArray &payload);
    void handleFileTransferRequest(const QByteArray &payload);

    static std::optional<bool> boolFromByte(uint8_t byte);

    QStringList m_supportedServices;

};
