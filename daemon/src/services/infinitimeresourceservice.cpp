#include "infinitimeresourceservice.h"

InfiniTimeResourceService::InfiniTimeResourceService(AdafruitBleFsService* fsService) : fsService{fsService}
{

}

void InfiniTimeResourceService::listDirectory() {
    qDebug() << "TEST4";
    auto p = fsService->listDirectory();

    p.wait();
    auto l = p.get();
    qDebug() << "Received " << l.size() << "files";
    for(auto& f : l) {
        qDebug() << "\n\t" << (f.isDirectory ? "Directory" : "File") << " : " << f.name
                 << "\n\t Timestamp : " << f.timestamp;


    }
}
