#include "bipfirmwareservice.h"

const char* BipFirmwareService::UUID_SERVICE_FIRMWARE = "00001530-0000-3512-2118-0009af100700";
const char* BipFirmwareService::UUID_CHARACTERISTIC_FIRMWARE = "00001531-0000-3512-2118-0009af100700";
const char* BipFirmwareService::UUID_CHARACTERISTIC_FIRMWARE_DATA = "00001532-0000-3512-2118-0009af100700";

BipFirmwareService::BipFirmwareService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_FIRMWARE, path, parent)
{
    qDebug() << Q_FUNC_INFO;

    connect(this, &QBLEService::characteristicChanged, this, &BipFirmwareService::characteristicChanged);
}


void BipFirmwareService::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << "Changed:" << characteristic << value;

    if (characteristic == UUID_CHARACTERISTIC_FIRMWARE) {
        qDebug() << "...got metadata";
        if (m_operationRunning == 1 && m_updateFirmware) {
            if (m_updateFirmware->handleMetaData(value)) {
                delete m_updateFirmware;
                m_updateFirmware = nullptr;
                m_operationRunning = 0;
                emit operationRunningChanged();
            }
        }
    }
}

void BipFirmwareService::prepareFirmwareDownload(const AbstractFirmwareInfo *info, UpdateFirmwareOperation* operation)
{
    if (!m_updateFirmware) {
        m_updateFirmware = operation;
    } else {
        if (m_operationRunning == 1) {
            emit message(tr("An operation is currently running, please try later"));
        } else {
            delete m_updateFirmware;
            m_updateFirmware = new UpdateFirmwareOperation(info, this);
        }
    }
}

void BipFirmwareService::startDownload()
{
    if (m_updateFirmware && m_operationRunning == 0) {
        m_operationRunning = 1;
        emit operationRunningChanged();
        emit message(tr("Sending %1...").arg(m_updateFirmware->version()));
        m_updateFirmware->start();
    } else {
        emit message(tr("No file selected"));
    }
}

bool BipFirmwareService::operationRunning()
{
    if (m_operationRunning > 0)
        qDebug() << Q_FUNC_INFO << "Firmware operation running:" << m_operationRunning;
    return m_operationRunning > 0;
}

void BipFirmwareService::abortOperations()
{
    if (m_updateFirmware) {
        delete m_updateFirmware;
        m_updateFirmware = nullptr;
    }
    m_operationRunning = 0;
    emit operationRunningChanged();
}
