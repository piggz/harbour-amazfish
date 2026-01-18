
#include "zepposfiletransferimpl.h"
#include "typeconversion.h"
#include "huami/zepposdevice.h"
#include "bipfirmwareservice.h"

auto static readUntilNull(int &start, const QByteArray b) {
    QByteArray out;
    int len = 0;
    while ((start + len < b.length()) && b[start + len] != 0) {
        out += b[start + len];
        len++;
    };
    start += len;
    return out;
};

void ZeppOsFileTransferImpl::uploadFile(const QString &url, const QString &filename, const QByteArray &bytes, bool compress, ZeppOsFileTransferService::Callback *callback)
{
    qDebug() << Q_FUNC_INFO << "Sending " << bytes.length() << " bytes to " << filename << " in " << url;

    Request *request = new Request(
                url,
                filename,
                bytes.length(),
                compress && mCompressedChunkSize > 0 ? Amazfish::compressData(bytes) : bytes,
                compress && mCompressedChunkSize > 0,
                Amazfish::calculateCRC32(bytes),
                compress && mCompressedChunkSize > 0 ? mCompressedChunkSize : mChunkSize,
                callback
                );

    uploadFile(request);
}

ZeppOsFileTransferImpl::ZeppOsFileTransferImpl(ZeppOsFileTransferService *fileTransferService, ZeppOSDevice *device) : m_fileTransferService(fileTransferService), m_device(device)
{
    qDebug() << Q_FUNC_INFO;
}

void ZeppOsFileTransferImpl::handlePayload(const QByteArray &payload)
{
    qDebug() << Q_FUNC_INFO;
    switch (payload[0]) {
    case CMD_CAPABILITIES_RESPONSE:
        handleCapabilitiesResponse(payload);
        return;
    case CMD_TRANSFER_REQUEST:
        handleFileTransferRequest(payload);
        return;
    default:
        qDebug() << "Unexpected file transfer payload byte {}" << payload[0];
    }
}

void ZeppOsFileTransferImpl::handleCapabilitiesResponse(const QByteArray &payload)
{
    qDebug() << Q_FUNC_INFO << payload.toHex();

    supportedServices.clear();

    int i = 0;

    mVersion = payload[++i] & 0xff;
    i += 1;
    mChunkSize = TypeConversion::toInt16( payload[i], payload[i+1]) & 0xffff;
    i += 2;

    qDebug() << "Version:" << mVersion << " Chunk Size:" << mChunkSize;


    if (mVersion >= 3) {
        mCompressedChunkSize = TypeConversion::toInt32( payload[i], payload[i+1], payload[i+2], payload[i+3]);
        i += 4;
        int numServices = TypeConversion::toInt16( payload[i], payload[1+1]);
        i += 2;

        for (int j = 0; j < numServices; j++) {
            // gtr 4:    terminal agps notification jsapp sticky_notification nfc sport httpproxy
            // active 2: terminal agps notification jsapp sticky_notification nfc sport httpproxy readiness voicememo
            m_supportedServices << readUntilNull(i, payload);
            i++;
        }

        qDebug() << m_supportedServices;

        // TODO: 3 unknown bytes for v3

        BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(m_device->service(BipFirmwareService::UUID_SERVICE_FIRMWARE));
        if (fw) {
            fw->enableNotification(BipFirmwareService::UUID_CHARACTERISTIC_ZEPP_OS_FILE_TRANSFER_V3_SEND);
            fw->enableNotification(BipFirmwareService::UUID_CHARACTERISTIC_ZEPP_OS_FILE_TRANSFER_V3_RECEIVE);
        }
    }

    //TODO mSupport.evaluateGBDeviceEvent(new GBDeviceEventUpdatePreferences(PREF_SUPPORTED_SERVICES, new HashSet<>(supportedServices)));

}

void ZeppOsFileTransferImpl::handleFileTransferRequest(const QByteArray &payload)
{
    qDebug() << Q_FUNC_INFO;

    int idx = 0;
    idx++; // discard command byte

    uint8_t session = payload[idx++];
    QByteArray url = readUntilNull(idx, payload);
    idx += (url.length() + 1);

    qDebug() << session << url;

    QByteArray filename = readUntilNull(idx, payload);

    uint32_t length = TypeConversion::toUint32(payload[idx],payload[idx+1],payload[idx+2],payload[idx+3]);
    idx += 4;
    uint32_t crc32 = TypeConversion::toUint32(payload[idx],payload[idx+1],payload[idx+2],payload[idx+3]);
    idx += 4;

    qDebug() << idx << payload.length() << filename;

    bool compressed = false;
    if (idx < payload.length()) {
        uint8_t compressedByte = payload[idx];
        std::optional<bool> compressedBool = boolFromByte(compressedByte);

        if (compressedBool.has_value()) {
            compressed = compressedBool.value();
        }
    }
    QByteArray ba;
    Request *r = new Request(url, filename, length, ba, compressed, crc32, mChunkSize, m_device);

    handleFileDownloadRequest(session, r);
}

std::optional<bool> ZeppOsFileTransferImpl::boolFromByte(uint8_t byte)
{
    switch(byte) {
    case 0x00:
        return false;
    case 0x01:
        return true;
    default:
        return std::nullopt;
    }
}

ZeppOsFileTransferImpl::Request::Request(QString url, const QString &filename, int rawLength, const QByteArray &bytes, bool compressed, int crc32, int chunkSize, ZeppOsFileTransferService::Callback *callback) :
    m_url(url),
    m_filename(filename),
    m_rawLength(rawLength),
    m_compressed(compressed),
    m_crc32(crc32),
    m_chunkSize(chunkSize),
    m_callback(callback)
{
    m_bytes = bytes;
}

QString ZeppOsFileTransferImpl::Request::url() const
{
    return m_url;
}

void ZeppOsFileTransferImpl::Request::setUrl(const QString &newUrl)
{
    m_url = newUrl;
}

ZeppOsFileTransferService::Callback *ZeppOsFileTransferImpl::Request::callback() const
{
    return m_callback;
}

void ZeppOsFileTransferImpl::Request::setCallback(ZeppOsFileTransferService::Callback *newCallback)
{
    m_callback = newCallback;
}

int ZeppOsFileTransferImpl::Request::progress() const
{
    return m_progress;
}

void ZeppOsFileTransferImpl::Request::setProgress(int newProgress)
{
    m_progress = newProgress;
}

int ZeppOsFileTransferImpl::Request::size() const
{
    return m_bytes.length();
}

QString ZeppOsFileTransferImpl::Request::filename() const
{
    return m_filename;
}

void ZeppOsFileTransferImpl::Request::setFilename(const QString &newFilename)
{
    m_filename = newFilename;
}

int ZeppOsFileTransferImpl::Request::index() const
{
    return m_index;
}

void ZeppOsFileTransferImpl::Request::setIndex(int newIndex)
{
    m_index = newIndex;
}

bool ZeppOsFileTransferImpl::Request::compressed() const
{
    return m_compressed;
}

void ZeppOsFileTransferImpl::Request::setCompressed(bool newCompressed)
{
    m_compressed = newCompressed;
}

int ZeppOsFileTransferImpl::Request::rawLength() const
{
    return m_rawLength;
}

void ZeppOsFileTransferImpl::Request::setRawLength(int newRawLength)
{
    m_rawLength = newRawLength;
}

int ZeppOsFileTransferImpl::Request::crc32() const
{
    return m_crc32;
}

void ZeppOsFileTransferImpl::Request::setCrc32(int newCrc32)
{
    m_crc32 = newCrc32;
}

QByteArray& ZeppOsFileTransferImpl::Request::bytes()
{
    return m_bytes;
}

int ZeppOsFileTransferImpl::Request::chunkSize() const
{
    return m_chunkSize;
}

void ZeppOsFileTransferImpl::Request::setChunkSize(int newChunkSize)
{
    m_chunkSize = newChunkSize;
}
