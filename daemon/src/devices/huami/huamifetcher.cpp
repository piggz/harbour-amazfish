#include "huamifetcher.h"

#include "activityfetchoperation.h"
#include "huamidevice.h"
#include "logfetchoperation.h"
#include "sportssummaryoperation.h"

HuamiFetcher::HuamiFetcher(HuamiDevice *device) : m_device(device)
{

}

void HuamiFetcher::startFetchData(Amazfish::DataTypes type)
{
    qDebug() << Q_FUNC_INFO << type;

    if (type & Amazfish::DataType::TYPE_ACTIVITY) {
        m_operations.append(new ActivityFetchOperation(this, m_device->database(), m_device->activitySampleSize(), true));
    } else if (type & Amazfish::DataType::TYPE_DEBUGLOG) {
        m_operations.append(new LogFetchOperation(this));
    } else if (type & Amazfish::DataType::TYPE_GPS_TRACK) {
        m_operations.append(new SportsSummaryOperation(this, m_device->database(), m_device->activitySummaryParser(), true));
    }

    if (!m_currentOperation) {
        triggerNextOperation();
    }
}

void HuamiFetcher::fetchControl(const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO;
    if (m_currentOperation) {
        if (m_currentOperation->handleMetaData(value)) {
            emit fetchOperationComplete(m_currentOperation);
            delete m_currentOperation;
            m_currentOperation = nullptr;
            triggerNextOperation();
        }
    }
}

void HuamiFetcher::fetchData(const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO;
    if (m_currentOperation) {
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
    bool wasFetching = m_currentOperation != nullptr;
    if (m_operations.isEmpty()) {
        m_currentOperation = nullptr;
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
