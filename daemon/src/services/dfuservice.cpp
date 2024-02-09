#include "dfuservice.h"
#include "dfuoperation.h"

const char* DfuService::UUID_SERVICE_DFU = "00001530-1212-efde-1523-785feabcd123";
const char* DfuService::UUID_CHARACTERISTIC_DFU_CONTROL = "00001531-1212-efde-1523-785feabcd123";
const char* DfuService::UUID_CHARACTERISTIC_DFU_PACKET = "00001532-1212-efde-1523-785feabcd123";
const char* DfuService::UUID_CHARACTERISTIC_DFU_REVISION = "00001534-1212-efde-1523-785feabcd123";

DfuService::DfuService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_DFU, path, parent)
{
    qDebug() << Q_FUNC_INFO;
    m_waitForWatch.store(false);

    connect(this, &QBLEService::characteristicChanged, this, &DfuService::characteristicChanged);
}

void DfuService::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << characteristic << value;

    if (characteristic == UUID_CHARACTERISTIC_DFU_CONTROL) {
        qDebug() << "...got metadata";
        if (m_operationRunning == 1 && m_updateFirmware) {
            if (m_updateFirmware->handleMetaData(value)) {
                m_updateFirmware.release();
                m_operationRunning = 0;
            }
        }
    }
}

void DfuService::prepareFirmwareDownload(const AbstractFirmwareInfo *info)
{
    if (m_operationRunning == 1) {
        emit message(tr("An operation is currently running, please try later"));
    } else {
        m_updateFirmware.reset(new DfuOperation(info, this));
    }
}

void DfuService::startDownload()
{
    qDebug() << Q_FUNC_INFO;
    if (m_updateFirmware && m_operationRunning == 0) {
        m_operationRunning = 1;
        connect(m_updateFirmware.get(), &DfuOperation::transferError, this, &DfuService::onTransferError);
        m_updateFirmware->start();
    } else {
        emit message(tr("No file selected"));
    }
}

bool DfuService::operationRunning()
{
    if (m_operationRunning > 0)
        qDebug() << Q_FUNC_INFO << "Firmware operation running:" << m_operationRunning;

    return m_operationRunning > 0;
}

void DfuService::abortOperations()
{
    if (m_updateFirmware) {
        m_updateFirmware.release();
    }
    m_operationRunning = 0;
    emit operationRunningChanged();
}

void DfuService::setWaitForWatch(bool wait)
{
    m_waitForWatch.store(wait);
}

bool DfuService::waitForWatch()
{
    return m_waitForWatch.load();
}

void DfuService::onTransferError(const QString error) {
    m_operationRunning = 0;
    qDebug() << Q_FUNC_INFO << "Transfer error:" << error;
    emit message(error);
}
