#include "dfuservice.h"
#include "dfuoperation.h"

const char* DfuService::UUID_SERVICE_DFU = "00001530-1212-efde-1523-785feabcd123";
const char* DfuService::UUID_CHARACTERISTIC_DFU_CONTROL = "00001531-1212-efde-1523-785feabcd123";
const char* DfuService::UUID_CHARACTERISTIC_DFU_PACKET = "00001532-1212-efde-1523-785feabcd123";
const char* DfuService::UUID_CHARACTERISTIC_DFU_REVISION = "00001534-1212-efde-1523-785feabcd123";

DfuService::DfuService(const QString &path, QObject *parent) : AbstractOperationService(UUID_SERVICE_DFU, path, parent)
{
    qDebug() << Q_FUNC_INFO;
    m_waitForWatch.store(false);

    connect(this, &QBLEService::characteristicChanged, this, &DfuService::characteristicChanged);
}

void DfuService::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << characteristic << value;

    AbstractOperationService::notifyOperation(characteristic, value);
}

void DfuService::setWaitForWatch(bool wait)
{
    m_waitForWatch.store(wait);
}

bool DfuService::waitForWatch()
{
    return m_waitForWatch.load();
}
