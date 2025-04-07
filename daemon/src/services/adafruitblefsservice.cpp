#include "adafruitblefsservice.h"

const char* AdafruitBleFsService::UUID_SERVICE_FS = "0000febb-0000-1000-8000-00805f9b34fb";
const char* AdafruitBleFsService::UUID_CHARACTERISTIC_FS_VERSION = "adaf0100-4669-6c65-5472-616e73666572";
const char* AdafruitBleFsService::UUID_CHARACTERISTIC_FS_TRANSFER = "adaf0200-4669-6c65-5472-616e73666572";

AdafruitBleFsService::AdafruitBleFsService(const QString &path, QObject *parent, size_t mtu) : AbstractOperationService(UUID_SERVICE_FS, path, parent), m_mtu(mtu)
{
    qDebug() << Q_FUNC_INFO;

    connect(this, &QBLEService::characteristicChanged, this, &AdafruitBleFsService::characteristicChanged);
    enableNotification(UUID_CHARACTERISTIC_FS_TRANSFER);
}

size_t AdafruitBleFsService::mtu()
{
    return m_mtu;
}

void AdafruitBleFsService::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << characteristic << value;

    AbstractOperationService::notifyOperation(characteristic, value);
}
