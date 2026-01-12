#include "huamifetcher.h"

#include "activityfetchoperation.h"
#include "huamidevice.h"

HuamiFetcher::HuamiFetcher(HuamiDevice *device) : m_device(device)
{

}

void HuamiFetcher::startFetchData(Amazfish::DataTypes type)
{
    qDebug() << Q_FUNC_INFO << type;

    if (type & Amazfish::DataType::TYPE_ACTIVITY) {
        MiBandService *mi = qobject_cast<MiBandService*>(m_device->service(MiBandService::UUID_SERVICE_MIBAND));
        if (mi){
            m_currentOperation = new ActivityFetchOperation(this, m_device->database(), m_device->activitySampleSize(), true);
            m_currentOperation->start(mi);
            setBusy(true);
        }
    }
}

void HuamiFetcher::fetchControl(const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO;
    if (m_currentOperation) {
        if (m_currentOperation->handleMetaData(value)) {
            delete m_currentOperation;
            m_currentOperation = nullptr;
            setBusy(false);
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
