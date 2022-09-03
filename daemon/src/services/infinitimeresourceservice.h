#ifndef INFINITIMERESOURCESERVICE_H
#define INFINITIMERESOURCESERVICE_H

#include "adafruitblefsservice.h"
#include "qble/qbleservice.h"

class InfiniTimeResourceService :  public QBLEService {
public:
    InfiniTimeResourceService(AdafruitBleFsService* fsService);

    void listDirectory();

private:
    AdafruitBleFsService* fsService = nullptr;
};

#endif // INFINITIMERESOURCESERVICE_H
