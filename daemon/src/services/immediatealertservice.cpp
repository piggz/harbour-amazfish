#include "immediatealertservice.h"

const char* ImmediateAlertService::UUID_SERVICE_IMMEDIATE_ALERT = "00001802-0000-1000-8000-00805f9b34fb";
const char* ImmediateAlertService::UUID_CHARACTERISTIC_IMMEDIATE_ALERT_LEVEL = "00002a06-0000-1000-8000-00805f9b34fb";


ImmediateAlertService::ImmediateAlertService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_IMMEDIATE_ALERT, path, parent)
{
    qDebug() << Q_FUNC_INFO;

    connect(this, &QBLEService::characteristicRead, this, &ImmediateAlertService::characteristicRead);
}

void ImmediateAlertService::refreshInformation()
{
    qDebug() << Q_FUNC_INFO;

    readAsync(UUID_CHARACTERISTIC_IMMEDIATE_ALERT_LEVEL);
}

void ImmediateAlertService::characteristicRead(const QString &characteristic, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << "Read:" << characteristic << value;
    if (characteristic == UUID_CHARACTERISTIC_IMMEDIATE_ALERT_LEVEL) {
        m_alertLevel = value[0];
        emit informationChanged(AbstractDevice::INFO_IMMEDIATE_ALERT, QString::number(m_alertLevel));
    } else {
        qWarning() << Q_FUNC_INFO << "Unknown value";
    }
}

int ImmediateAlertService::alertLevel() const
{
    qDebug() << Q_FUNC_INFO << m_alertLevel;
    return m_alertLevel;
}
