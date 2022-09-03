#ifndef ADAFRUITBLEFSSERVICE_H
#define ADAFRUITBLEFSSERVICE_H

#include "qble/qbleservice.h"
#include "adafruitblefsoperation.h"

/*
{0000febb-0000-1000-8000-00805f9b34fb} FS Service
--adaf0100-4669-6c65-5472-616e73666572 // Version
--adaf0200-4669-6c65-5472-616e73666572 // transfer
*/

class AdafruitBleFsService : public QBLEService
{
    Q_OBJECT

public:
    AdafruitBleFsService(const QString &path, QObject *parent, size_t mtu);
    static const char *UUID_SERVICE_FS;
    static const char *UUID_CHARACTERISTIC_FS_VERSION;
    static const char *UUID_CHARACTERISTIC_FS_TRANSFER;

    void prepareDownload(AdafruitBleFsOperation* operation);
    void updateFiles();

    Q_SIGNAL void downloadProgress(int percent);

private:
    Q_SLOT void characteristicChanged(const QString &characteristic, const QByteArray &value);

    AdafruitBleFsOperation* operation = nullptr;
    const size_t mtu;
};

#endif // ADAFRUITBLEFSSERVICE_H
