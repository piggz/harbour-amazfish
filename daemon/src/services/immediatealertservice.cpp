#include "immediatealertservice.h"

const char* ImmediateAlertService::UUID_SERVICE_IMMEDIATE_ALERT = "00001802-0000-1000-8000-00805f9b34fb";
const char* ImmediateAlertService::UUID_CHARACTERISTIC_IMMEDIATE_ALERT_LEVEL = "00002a06-0000-1000-8000-00805f9b34fb";

ImmediateAlertService::ImmediateAlertService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_IMMEDIATE_ALERT, path, parent)
{
    qDebug() << Q_FUNC_INFO;
}

QString ImmediateAlertService::levelToString(const ImmediateAlertService::Levels level)
{
    switch (level) {
    case Levels::NoAlert:
        return QStringLiteral("NoAlert");
    case Levels::MildAlert:
        return QStringLiteral("MildAlert");
    case Levels::HighAlert:
        return QStringLiteral("HighAlert");
    default:
        return QStringLiteral("Unknown");
    }
}

ImmediateAlertService::Levels ImmediateAlertService::alertLevel()
{
    qDebug() << Q_FUNC_INFO << levelToString(m_alertLevel);
    return m_alertLevel;
}

void ImmediateAlertService::sendAlert(ImmediateAlertService::Levels level)
{
    qDebug() << Q_FUNC_INFO << levelToString(level);

    QByteArray send = QByteArray(1, (char)(level));
    writeValue(UUID_CHARACTERISTIC_IMMEDIATE_ALERT_LEVEL, send);
}
