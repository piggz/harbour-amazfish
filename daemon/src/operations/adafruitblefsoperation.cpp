#include "adafruitblefsoperation.h"
#include "adafruitblefsservice.h"
#include "adafruitblefsworker.h"
#include <QMetaType>

AdafruitBleFsOperation::AdafruitBleFsOperation(QBLEService *service, const AbstractFirmwareInfo *info, AbstractDevice *device) : info{info}, m_device(device)
{

}

AdafruitBleFsOperation::~AdafruitBleFsOperation()
{

}

void AdafruitBleFsOperation::start(QBLEService *service)
{
    m_service = service;

    AdafruitBleFsService *s = dynamic_cast<AdafruitBleFsService*>(service);
    if (!s) {
        return;
    }

    if(m_worker == nullptr)
    {
        m_worker = new BleFsWorker(info);
        m_worker->moveToThread(&m_workerThread);
        QObject::connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
        QObject::connect(this, &AdafruitBleFsOperation::startUpdateFiles, m_worker, &BleFsWorker::updateFiles);
        QObject::connect(m_worker, &QObject::destroyed, this, &AdafruitBleFsOperation::workerDestroyed);

        m_workerThread.start();
    }

    emit startUpdateFiles(this, s->mtu());
}

bool AdafruitBleFsOperation::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    if (!m_worker) {
        return true;
    }

    if (characteristic == AdafruitBleFsService::UUID_CHARACTERISTIC_FS_VERSION) {
        qDebug() << "Version : " << (value[0] + (value[1] << 8));
    } else {
        handleData(value);
    }
    return false;
}

bool AdafruitBleFsOperation::handleMetaData(const QByteArray &value)
{
    return true;
}

void AdafruitBleFsOperation::handleData(const QByteArray &data)
{
    const uint8_t cmdId = data[0];

    switch(cmdId) {
    case RESPONSE_LIST_DIR:
        handleListDirectory(data);
        break;
    case RESPONSE_DELETE_FILE:
        handleDeleteFile(data);
        break;
    case RESPONSE_WRITE_FILE:
        handleWriteFile(data);
        break;
    case RESPONSE_CREATE_DIR:
        handleCreateDirectory(data);
        break;
    default:
        break;
    }
}

void AdafruitBleFsOperation::handleListDirectory(const QByteArray &value)
{
    uint8_t status = value[1];
    uint16_t pathSize = value[2] + (value[3] << 8);
    uint32_t entryNr = value[4] + (value[5] << 8) + (value[6] << 16) + (value[7] << 24);
    uint32_t entryTotal = value[8] + (value[9] << 8) + (value[10] << 16) + (value[11] << 24);
    uint32_t flags = value[12] + (value[13] << 8) + (value[14] << 16) + (value[15] << 24);
    bool isDirectory = (flags & 0x01) == 1;
    uint64_t timestamp = value[16] + (value[17] << 8) + (value[18] << 16) + (value[19] << 24)
            + + ((uint64_t)value[20] << 32) + + ((uint64_t)value[21] << 40) + + ((uint64_t)value[22] << 48) + + ((uint64_t)value[23] << 56);
    uint32_t fileSize = value[24] + (value[25] << 8) + (value[26] << 16) + (value[27] << 24);
    std::string filename;
    for(int i = 0; i < pathSize; i++) {
        filename += (char)value[28+i];
    }
    filename[pathSize] = 0;

    if(entryNr == 0)
    {
        listDirectoryEntries.clear();
    }

    if(status == static_cast<uint8_t>(Status::Success))
    {
        File f {filename, timestamp, isDirectory};
        listDirectoryEntries.push_back(f);
    }

    if(entryNr == entryTotal)
    {
        listDirectoryPromise.set_value(listDirectoryEntries);
    }
}

void AdafruitBleFsOperation::handleDeleteFile(const QByteArray &value)
{
    deleteFilePromise.set_value();
}

void AdafruitBleFsOperation::handleWriteFile(const QByteArray &value)
{
    uint8_t status = value[1];
    uint32_t offset = (uint32_t)value[4] + ((uint32_t)value[5] << 8) + ((uint32_t)value[6] << 16) + ((uint32_t)value[7] << 24);
    uint64_t timestamp = value[8] + (value[9] << 8) + (value[10] << 16) + (value[11] << 24) + ((uint64_t)value[12] << 32) + ((uint64_t)value[13] << 40) + ((uint64_t)value[14] << 48) + ((uint64_t)value[15] << 56);
    uint32_t freeSpace = value[16] + (value[17] << 8) + (value[18] << 16) + (value[19] << 24);

    writeFilePromise.set_value(status == static_cast<uint8_t>(Status::Success));
}

std::future<std::vector<AdafruitBleFsOperation::File>> AdafruitBleFsOperation::listDirectory(const std::string& path)
{
    this->listDirectoryPromise = std::promise<std::vector<File>>();

    QByteArray cmd;
    cmd += REQUEST_LIST_DIR;
    cmd += PADDING_BYTE;
    cmd += (uint8_t)(path.size() & 0xff);
    cmd += (uint8_t)((path.size() >> 8) & 0xff);
    for(int i = 0; i < path.size(); i++) {
        cmd += path[i];
    }
    m_service->writeValue(AdafruitBleFsService::UUID_CHARACTERISTIC_FS_TRANSFER, cmd);

    return listDirectoryPromise.get_future();
}

std::future<void> AdafruitBleFsOperation::eraseFile(const std::string& filename)
{
    deleteFilePromise = std::promise<void>();

    QByteArray cmd;
    cmd += REQUEST_DELETE_FILE;
    cmd += PADDING_BYTE;
    cmd += (uint8_t)(filename.size() & 0xff);
    cmd += (uint8_t)((filename.size() >> 8) & 0xff);
    for(int i = 0; i < filename.size(); i++) {
        cmd += filename[i];
    }
    m_service->writeValue(AdafruitBleFsService::UUID_CHARACTERISTIC_FS_TRANSFER, cmd);

    return deleteFilePromise.get_future();
}


std::future<bool> AdafruitBleFsOperation::writeFileStart(const std::string& filePath, size_t fileSize, size_t offset)
{
    writeFilePromise = std::promise<bool>();

    QByteArray cmd;
    cmd += REQUEST_WRITE_FILE_START;
    cmd += PADDING_BYTE;
    cmd += (uint8_t)(filePath.size() & 0xff);
    cmd += (uint8_t)((filePath.size() >> 8) & 0xff);
    cmd += offset & 0xff;
    cmd += (offset >> 8) & 0xff;
    cmd += (offset >> 16) & 0xff;
    cmd += (offset >> 24) & 0xff;
    cmd += (uint8_t)0; // timestamp
    cmd += (uint8_t)0;
    cmd += (uint8_t)0;
    cmd += (uint8_t)0;
    cmd += (uint8_t)0;
    cmd += (uint8_t)0;
    cmd += (uint8_t)0;
    cmd += (uint8_t)0;
    cmd += fileSize & 0xff;
    cmd += (fileSize >> 8) & 0xff;
    cmd += (fileSize >> 16) & 0xff;
    cmd += (fileSize >> 24) & 0xff;
    for(int i = 0; i < filePath.size(); i++)
    {
        cmd += filePath[i];
    }
    m_service->writeValue(AdafruitBleFsService::UUID_CHARACTERISTIC_FS_TRANSFER, cmd);

    return writeFilePromise.get_future();
}

std::future<bool> AdafruitBleFsOperation::writeFileData(const std::vector<uint8_t> data, size_t offset)
{
    writeFilePromise = std::promise<bool>();

    QByteArray cmd;
    cmd += REQUEST_WRITE_FILE_DATA;
    cmd += (uint8_t)0x01;
    cmd += PADDING_BYTE;
    cmd += PADDING_BYTE;
    cmd += offset & 0xff;
    cmd += (offset >> 8) & 0xff;
    cmd += (offset >> 16) & 0xff;
    cmd += (offset >> 24) & 0xff;
    cmd += data.size() & 0xff;
    cmd += (data.size() >> 8) & 0xff;
    cmd += (data.size() >> 16) & 0xff;
    cmd += (data.size() >> 24) & 0xff;
    for(int i = 0; i < data.size(); i++)
    {
        cmd += data[i];
    }

    m_service->writeValue(AdafruitBleFsService::UUID_CHARACTERISTIC_FS_TRANSFER, cmd);

    return writeFilePromise.get_future();
}

void AdafruitBleFsOperation::downloadProgress(int percent) {
    m_device->downloadProgress(percent);
}


std::future<bool> AdafruitBleFsOperation::createDirectory(const std::string& path) {
    createDirectoryPromise = std::promise<bool>();

    QByteArray cmd;
    cmd += REQUEST_CREATE_DIR;
    cmd += PADDING_BYTE;
    cmd += (path.size()) & 0xff; // Path size
    cmd += (path.size() >> 8) & 0xff;
    cmd += PADDING_BYTE;
    cmd += PADDING_BYTE;
    cmd += PADDING_BYTE;
    cmd += PADDING_BYTE;
    cmd += (uint8_t)0;  // Time
    cmd += (uint8_t)0;
    cmd += (uint8_t)0;
    cmd += (uint8_t)0;
    cmd += (uint8_t)0;
    cmd += (uint8_t)0;
    cmd += (uint8_t)0;
    cmd += (uint8_t)0;
    for(int i = 0; i < path.size(); i++)
    {
        cmd += path[i];
    }

    m_service->writeValue(AdafruitBleFsService::UUID_CHARACTERISTIC_FS_TRANSFER, cmd);

    return createDirectoryPromise.get_future();
}

void AdafruitBleFsOperation::handleCreateDirectory(const QByteArray &value) {
    uint8_t status = value[1];
    createDirectoryPromise.set_value(status != static_cast<uint8_t>(Status::ReadOnly)); // In this case, ERROR means that the directory already existst
}

void AdafruitBleFsOperation::workerDestroyed(QObject *object)
{
    m_worker = nullptr;
}
