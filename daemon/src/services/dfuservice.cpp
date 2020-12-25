#include "dfuservice.h"
#include "dfuoperation.h"

const char* DfuService::UUID_SERVICE_DFU = "00001530-1212-efde-1523-785feabcd123";
const char* DfuService::UUID_CHARACTERISTIC_DFU_CONTROL = "00001531-1212-efde-1523-785feabcd123";
const char* DfuService::UUID_CHARACTERISTIC_DFU_PACKET = "00001532-1212-efde-1523-785feabcd123";
const char* DfuService::UUID_CHARACTERISTIC_DFU_REVISION = "00001534-1212-efde-1523-785feabcd123";

constexpr uint8_t DfuService::COMMAND_STARTDFU;
constexpr uint8_t DfuService::COMMAND_RESPONSE;
constexpr uint8_t DfuService::COMMAND_INITDFUPARAMETERS;
constexpr uint8_t DfuService::COMMAND_VALIDATE_FIRMWARE;
constexpr uint8_t DfuService::COMMAND_RECEIVE_FIRMWARE_IMAGE;
constexpr uint8_t DfuService::COMMAND_ACTIVATE_IMAGE_AND_RESET;
constexpr uint8_t DfuService::COMMAND_PACKET_RECEIPT_NOTIFICATION;
constexpr uint8_t DfuService::COMMAND_PACKET_RECEIPT_NOTIFICATION_REQUEST;

DfuService::DfuService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_DFU, path, parent)
{
    qDebug() << Q_FUNC_INFO;

    connect(this, &QBLEService::characteristicChanged, this, &DfuService::characteristicChanged);
}

void DfuService::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << characteristic << value;

    if (characteristic == UUID_CHARACTERISTIC_DFU_CONTROL) {
        qDebug() << "...got metadata";
        if (m_operationRunning == 1 && m_updateFirmware) {
            if (m_updateFirmware->handleMetaData(value)) {
                delete m_updateFirmware;
                m_updateFirmware = nullptr;
                m_operationRunning = 0;
            }
        }
    }
}

void DfuService::prepareFirmwareDownload(const AbstractFirmwareInfo *info, DfuOperation* operation)
{
    if (!m_updateFirmware) {
        m_updateFirmware = operation;
    } else {
        if (m_operationRunning == 1) {
            emit message(tr("An operation is currently running, please try later"));
        } else {
            delete m_updateFirmware;
            m_updateFirmware = new DfuOperation(info, this);
        }
    }
}

void DfuService::startDownload()
{
    qDebug() << Q_FUNC_INFO;
    if (m_updateFirmware && m_operationRunning == 0) {
        m_operationRunning = 1;
        m_updateFirmware->start();
    } else {
        emit message(tr("No file selected"));
    }
}

bool DfuService::operationRunning()
{
    qDebug() << "is firmware operation running:" << m_operationRunning;
    return m_operationRunning > 0;
}

void DfuService::abortOperations()
{
    if (m_updateFirmware) {
        delete m_updateFirmware;
        m_updateFirmware = nullptr;
    }
    m_operationRunning = 0;
    emit operationRunningChanged();
}
