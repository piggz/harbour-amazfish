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
    auto manifestData = manifestEntry->data();

    // Open manifest file and parse JSON
    QJsonDocument manifestDocument = QJsonDocument::fromJson(manifestData);
    QJsonObject manifestObject = manifestDocument.object();
    auto resourceList = manifestObject["resources"].toArray();

    // Get total size to be uploaded
    auto totalSize = getTotalSize(resourceList, archiveRoot);

    // List directory from the watch
    auto fileList = getRemoteFileList(service);


    // Upload the resources
    int progressSize = 0;
    for(const QJsonValue& resource : resourceList)
    {
        auto sourceFile = resource["filename"].toString();
        auto destinationFile = resource["path"].toString();

        if(fileExists(fileList, destinationFile))
        {
            eraseRemoteFile(service, destinationFile);
        }

        auto* resourceEntry = dynamic_cast<const KZipFileEntry*>(archiveRoot->entry(sourceFile));
        auto resourceData = resourceEntry->data();

        auto writeFileFuture = service->writeFileStart(destinationFile.toStdString(), resourceData.size(), 0);
        writeFileFuture.wait();

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
            writeFileDataFuture.get();

            written += toWrite;
            progressSize += toWrite;
            service->downloadProgress((100.0f / totalSize) * progressSize);
        }
    }
}

std::vector<AdafruitBleFsOperation::File> BleFsWorker::getRemoteFileList(AdafruitBleFsOperation* service)
{
    auto listDirectoryFuture = service->listDirectory();
    listDirectoryFuture.wait();
    return listDirectoryFuture.get();

}

void BleFsWorker::eraseRemoteFile(AdafruitBleFsOperation* service, QString file)
{
    auto eraseFileFuture = service->eraseFile(file.toStdString());
    eraseFileFuture.wait();
}
