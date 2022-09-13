#ifndef ADAFRUITBLEFSOPERATION_H
#define ADAFRUITBLEFSOPERATION_H

#include "abstractoperation.h"
#include "abstractfirmwareinfo.h"

class AdafruitBleFsService;
class BleFsWorker;

class AdafruitBleFsOperation : public QObject, public AbstractOperation
{
    Q_OBJECT
public:
    AdafruitBleFsOperation(QBLEService *service, const AbstractFirmwareInfo *info);
    ~AdafruitBleFsOperation();
    bool handleMetaData(const QByteArray &meta) override;
    void handleData(const QByteArray &data) override;
    void start() override;

    struct File {        
        std::string name;
        uint64_t timestamp;
        bool isDirectory;
    };

    std::future<std::vector<File>> listDirectory(const std::string& path);
    std::future<void> eraseFile(const std::string& file);
    std::future<size_t> writeFileStart(const std::string& filePath, size_t fileSize, size_t offset);
    std::future<size_t> writeFileData(const std::vector<uint8_t> data, size_t offset);

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

    void handleListDirectory(const QByteArray &data);
    void handleDeleteFile(const QByteArray &data);
    void handleWriteFile(const QByteArray &data);

    const AbstractFirmwareInfo *info;

    std::vector<File> listDirectoryEntries;

    std::promise<std::vector<File>> listDirectoryPromise;
    std::promise<void> deleteFilePromise;
    std::promise<size_t> writeFilePromise;

    QThread m_workerThread;
    BleFsWorker* m_worker = nullptr;

    Q_SIGNAL void startUpdateFiles(AdafruitBleFsOperation* service, const int mtu);
};

#endif // ADAFRUITBLEFSOPERATION_H
