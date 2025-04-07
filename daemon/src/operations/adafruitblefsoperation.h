#ifndef ADAFRUITBLEFSOPERATION_H
#define ADAFRUITBLEFSOPERATION_H

#include "abstractoperation.h"
#include "abstractdevice.h"
#include "abstractfirmwareinfo.h"
#include <future>

class AdafruitBleFsService;
class BleFsWorker;

class AdafruitBleFsOperation : public QObject, public AbstractOperation
{
    Q_OBJECT
public:
    AdafruitBleFsOperation(QBLEService *service, const AbstractFirmwareInfo *info, AbstractDevice *device);
    ~AdafruitBleFsOperation();
    bool handleMetaData(const QByteArray &meta) override;
    void handleData(const QByteArray &data) override;
    void start(QBLEService *service) override;
    bool characteristicChanged(const QString &characteristic, const QByteArray &value) override;

    struct File {        
        std::string name;
        uint64_t timestamp;
        bool isDirectory;
    };


    std::future<std::vector<File>> listDirectory(const std::string& path);
    std::future<void> eraseFile(const std::string& file);
    std::future<bool> writeFileStart(const std::string& filePath, size_t fileSize, size_t offset);
    std::future<bool> writeFileData(const std::vector<uint8_t> data, size_t offset);
    std::future<bool> createDirectory(const std::string& path);

    void updateFiles(const int mtu);
    void downloadProgress(int percent);

private:
    static const uint8_t PADDING_BYTE = 0x00;
    static const uint8_t REQUEST_WRITE_FILE_START = 0x20;
    static const uint8_t RESPONSE_WRITE_FILE = 0x21;
    static const uint8_t REQUEST_WRITE_FILE_DATA = 0x22;
    static const uint8_t REQUEST_DELETE_FILE = 0x30;
    static const uint8_t RESPONSE_DELETE_FILE = 0x31;
    static const uint8_t REQUEST_LIST_DIR = 0x50;
    static const uint8_t RESPONSE_LIST_DIR = 0x51;
    static const uint8_t REQUEST_CREATE_DIR = 0x40;
    static const uint8_t RESPONSE_CREATE_DIR = 0x41;

    enum class Status : uint8_t {
        Success = 0x01,
        Error = 0x02,
        ReadOnly = 0x05
    };

    void handleListDirectory(const QByteArray &data);
    void handleDeleteFile(const QByteArray &data);
    void handleWriteFile(const QByteArray &data);
    void handleCreateDirectory(const QByteArray &data);

    const AbstractFirmwareInfo *info;

    std::vector<File> listDirectoryEntries;

    std::promise<std::vector<File>> listDirectoryPromise;
    std::promise<void> deleteFilePromise;
    std::promise<bool> writeFilePromise;
    std::promise<bool> createDirectoryPromise;

    QThread m_workerThread;
    BleFsWorker* m_worker = nullptr;

    QBLEService *m_service = nullptr;
    AbstractDevice *m_device = nullptr;

    Q_SIGNAL void startUpdateFiles(AdafruitBleFsOperation* service, const int mtu);
    Q_SLOT void workerDestroyed(QObject *object);
};

#endif // ADAFRUITBLEFSOPERATION_H
