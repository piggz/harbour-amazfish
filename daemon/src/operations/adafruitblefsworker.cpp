#include "adafruitblefsworker.h"
#include "adafruitblefsservice.h"
#include "adafruitblefsoperation.h"
#include <KArchive>
#include <kzip.h>
#include <KCompressionDevice>

namespace
{
    KZip openArchive(QByteArray& bytes)
    {
        QDataStream in(&bytes, QIODevice::ReadOnly);
        KCompressionDevice dev(in.device(), false, KCompressionDevice::CompressionType::None);
        KZip zip(&dev);
        return std::move(zip);
    }

    const KArchiveDirectory* getArchiveRootDirectory(KZip& zip)
    {
        if(!zip.open(QIODevice::ReadOnly))
        {
            return nullptr;
        }

        auto* root = zip.directory();
        auto list = root->entries();
        return root;
    }

    size_t getTotalSize(const QJsonArray& resourceList, const KArchiveDirectory* root)
    {
        size_t totalSize = 0;
        for(const QJsonValue& resource : resourceList) {
            auto sourceFile = resource["filename"].toString();
            auto* resourceEntry = dynamic_cast<const KZipFileEntry*>(root->entry(sourceFile));
            totalSize += resourceEntry->size();
        }
        return totalSize;
    }

    bool fileExists(std::vector<AdafruitBleFsOperation::File> list, QString name)
    {
        auto it = std::find_if(list.begin(), list.end(), [name](const AdafruitBleFsOperation::File& f)
        {
            return f.name == name.toStdString();
        });

        return it != list.end();
    }
}

BleFsWorker::BleFsWorker(const AbstractFirmwareInfo *info, QObject *parent) : QObject(parent), info{info}
{

}


BleFsWorker::~BleFsWorker()
{

}



void BleFsWorker::updateFiles(AdafruitBleFsOperation* service, int transferMtu)
{
    // Open ZIP file
    auto archiveBytes = info->bytes();

    QDataStream in(&archiveBytes, QIODevice::ReadOnly);
    KCompressionDevice dev(in.device(), false, KCompressionDevice::CompressionType::None);
    KZip zip(&dev);

    auto* archiveRoot = getArchiveRootDirectory(zip);
    if(archiveRoot == nullptr)
    {
        return;
    }

    auto archiveEntries = archiveRoot->entries();
    if(!archiveEntries.contains("resources.json"))
        return;

    // Get manifest file data from the archive
    auto* manifestEntry = dynamic_cast<const KZipFileEntry*>(archiveRoot->entry("resources.json"));
    if(manifestEntry == nullptr)
        return;

    auto manifestData = manifestEntry->data();

    // Open manifest file and parse JSON
    QJsonDocument manifestDocument = QJsonDocument::fromJson(manifestData);
    if(manifestDocument.isNull() || !manifestDocument.isObject())
        return;

    QJsonObject manifestObject = manifestDocument.object();
    if(manifestObject["resources"].isUndefined() || !manifestObject["resources"].isArray())
        return;

    auto resourceList = manifestObject["resources"].toArray();

    if(manifestObject["obsolete_files"].isUndefined() || !manifestObject["obsolete_files"].isArray())
        return;

    auto obsoleteFileList = manifestObject["obsolete_files"].toArray();

    // Remove obsolete files
    for(const QJsonValue& file : obsoleteFileList)
    {
        if(file["path"].isUndefined() || !file["path"].isString())
            continue;

        auto path = file["path"];
        eraseRemoteFile(service, path.toString());
    }

    // Get total size to be uploaded
    auto totalSize = getTotalSize(resourceList, archiveRoot);

    // Upload the resources
    int progressSize = 0;
    for(const QJsonValue& resource : resourceList)
    {
        if(resource["filename"].isUndefined() || !resource["filename"].isString())
            continue;

        if(resource["path"].isUndefined() || !resource["path"].isString())
            continue;

        auto sourceFile = resource["filename"].toString();
        auto destinationFile = resource["path"].toString();

        // Remove the destination file if it already exists on the target
        QStringList folderList = destinationFile.split('/');
        QString file = folderList.last();
        folderList.removeLast();
        QString folder = folderList.join('/');
        auto fileList = getRemoteFileList(service, folder);

        if(fileExists(fileList, file))
        {
            eraseRemoteFile(service, destinationFile);
        }

        if(!createParentFolder(service, folderList))
            continue;

        auto* resourceEntry = dynamic_cast<const KZipFileEntry*>(archiveRoot->entry(sourceFile));
        if(resourceEntry == nullptr)
            continue;
        auto resourceData = resourceEntry->data();

        auto writeFileFuture = service->writeFileStart(destinationFile.toStdString(), resourceData.size(), 0);
        writeFileFuture.wait();
        bool success = writeFileFuture.get();
        if(!success) continue;

        const size_t chunkSize = transferMtu-20;
        size_t written = 0;
        while(written < resourceData.size())
        {
            std::vector<uint8_t> chunk;
            size_t remaining = resourceData.size() - written;
            size_t toWrite = std::min(remaining, chunkSize);
            for(int i = 0; i < toWrite; i++)
            {
                chunk.push_back(resourceData.at(written + i));
            }

            auto writeFileDataFuture = service->writeFileData(chunk, written);
            writeFileDataFuture.wait();
            success = writeFileDataFuture.get();
            if(!success) break;

            written += toWrite;
            progressSize += toWrite;
            service->downloadProgress((100.0f / totalSize) * progressSize);
        }
    }
}

std::vector<AdafruitBleFsOperation::File> BleFsWorker::getRemoteFileList(AdafruitBleFsOperation* service, QString path)
{
    auto listDirectoryFuture = service->listDirectory(path.toStdString());
    listDirectoryFuture.wait();
    return listDirectoryFuture.get();

}

void BleFsWorker::eraseRemoteFile(AdafruitBleFsOperation* service, QString file)
{
    auto eraseFileFuture = service->eraseFile(file.toStdString());
    eraseFileFuture.wait();
}

bool BleFsWorker::createParentFolder(AdafruitBleFsOperation* service, QStringList folderList)
{
    if(folderList.size() <= 1)
        return true;

    auto folderName = folderList.join('/');

    folderList.removeLast();
    bool success = createParentFolder(service, folderList);
    if(!success)
        return false;

    auto createDirectoryFuture = service->createDirectory(folderName.toStdString());
    createDirectoryFuture.wait();
    return createDirectoryFuture.get();
}
