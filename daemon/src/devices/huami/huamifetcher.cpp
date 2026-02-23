#include "huamifetcher.h"

#include "huami/activityfetchoperation.h"
#include "huami/fetchhrvoperation.h"
#include "huami/fetchpaioperation.h"
#include "huami/fetchspo2normaloperation.h"
#include "huami/fetchspo2sleepoperation.h"
#include "huamidevice.h"
#include "huami/logfetchoperation.h"
#include "huami/sportssummaryoperation.h"

HuamiFetcher::HuamiFetcher(HuamiDevice *device) : m_device(device)
{
    m_operationTimeout = new QTimer();
    connect(m_operationTimeout, &QTimer::timeout, this, &HuamiFetcher::operationTimeout);
}

HuamiFetcher::~HuamiFetcher()
{
    delete m_operationTimeout;
}

void HuamiFetcher::startFetchData(Amazfish::DataTypes type)
{
    qDebug() << Q_FUNC_INFO << type;

    if (type & Amazfish::DataType::TYPE_ACTIVITY && m_device->supportsDataType(Amazfish::DataType::TYPE_ACTIVITY)) {
        m_operations.append(new ActivityFetchOperation(this, m_device->database(), m_device->activitySampleSize(), m_device->isZeppOs()));
    } else if (type & Amazfish::DataType::TYPE_DEBUGLOG && m_device->supportsDataType(Amazfish::DataType::TYPE_DEBUGLOG)) {
        m_operations.append(new LogFetchOperation(this, m_device->isZeppOs()));
    } else if (type & Amazfish::DataType::TYPE_GPS_TRACK && m_device->supportsDataType(Amazfish::DataType::TYPE_GPS_TRACK)) {
        m_operations.append(new SportsSummaryOperation(this, m_device->activitySummaryParser(), m_device->isZeppOs()));
    } else if (type & Amazfish::DataType::TYPE_PAI && m_device->supportsDataType(Amazfish::DataType::TYPE_PAI)) {
        m_operations.append(new FetchPaiOperation(this, m_device->database(), m_device->isZeppOs()));
    } else if (type & Amazfish::DataType::TYPE_HRV && m_device->supportsDataType(Amazfish::DataType::TYPE_HRV)) {
        m_operations.append(new FetchHrvOperation(this, m_device->database(), m_device->isZeppOs()));
    } else if (type & Amazfish::DataType::TYPE_SPO2 && m_device->supportsDataType(Amazfish::DataType::TYPE_SPO2)) {
        m_operations.append(new FetchSpo2NormalOperation(this, m_device->database(), m_device->isZeppOs()));
        m_operations.append(new FetchSpo2SleepOperation(this, m_device->database(), m_device->isZeppOs()));
    }

    if (!m_currentOperation) {
        triggerNextOperation();
    }
}

void HuamiFetcher::fetchControl(const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO;
    if (m_currentOperation) {
        m_operationTimeout->start(10000);
        if (m_currentOperation->handleMetaData(value)) {
            emit fetchOperationComplete(m_currentOperation);
            delete m_currentOperation;
            triggerNextOperation();
        }
    }
}

void HuamiFetcher::fetchData(const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO;
    if (m_currentOperation) {
        m_operationTimeout->start(10000);
        m_currentOperation->handleData(value);
    }
}

void HuamiFetcher::setNotifications(bool control, bool data)
{
    qDebug() << Q_FUNC_INFO;
    m_device->setActivityNotifications(control, data);
}

void HuamiFetcher::writeControl(const QByteArray value)
{
    qDebug() << Q_FUNC_INFO;
    m_device->writeActivityControl(value);
}

void HuamiFetcher::message(const QString &string)
{
    m_device->message(string);
}

void HuamiFetcher::jumpQueue(AbstractFetchOperation *operation)
{
    m_operations.push_front(operation);
}

void HuamiFetcher::reset()
{
    qDebug() << Q_FUNC_INFO;

    if (!m_operations.isEmpty() || m_currentOperation) {
        m_device->message(tr("All operations cancelled"));
    }
    qDeleteAll(m_operations);
    delete m_currentOperation;
    m_currentOperation = nullptr;
    m_operationTimeout->stop();

    setBusy(false);
}

bool HuamiFetcher::busy() const
{
    return m_busy;
}

void HuamiFetcher::setBusy(bool b)
{
    if (b != m_busy) {
        m_busy = b;
        emit busyChanged();
    }
}

void HuamiFetcher::triggerNextOperation()
{
    qDebug() << Q_FUNC_INFO;
    bool wasFetching = m_currentOperation != nullptr;
    if (m_operations.isEmpty()) {
        m_currentOperation = nullptr;
        m_operationTimeout->stop();
    } else {
        m_currentOperation = m_operations.takeFirst();
    }

    if (m_currentOperation) {
        m_currentOperation->start(0);
        setBusy(true);

        return;
    }

    if (wasFetching) {
        m_device->message(tr("All operations complete"));
        setBusy(false);
    }
}

void HuamiFetcher::operationTimeout()
{
    qDebug() << Q_FUNC_INFO;
    reset();
}
