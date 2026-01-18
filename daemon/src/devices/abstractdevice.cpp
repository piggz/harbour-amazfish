#include "abstractdevice.h"
#include "abstractoperationservice.h"
#include "hrmservice.h"

#include <QString>
    
AbstractDevice::AbstractDevice(const QString &pairedName, QObject *parent) : QBLEDevice(parent)
{
    qDebug() << Q_FUNC_INFO;

    setConnectionState("disconnected");
    m_pairedName = pairedName;
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(60000);
    connect(m_reconnectTimer, &QTimer::timeout, this, &AbstractDevice::reconnectionTimer);
    //connect(this, &QBLEDevice::pairFinished, this, &AbstractDevice::devicePairFinished);
    connect(this, &QBLEDevice::error, this, &AbstractDevice::deviceError);
}

void AbstractDevice::pair()
{
    qDebug() << Q_FUNC_INFO;

    m_needsAuth = true;
    m_pairing = true;
    m_autoreconnect = true;
    //disconnectFromDevice();
    setConnectionState("pairing");

    QBLEDevice::pair();
}


void AbstractDevice::connectToDevice()
{
    qDebug() << Q_FUNC_INFO;

    m_pairing = false;
    m_autoreconnect = true;
    QBLEDevice::disconnectFromDevice();
    setConnectionState("connecting");
    QBLEDevice::connectToDevice();
    m_reconnectTimer->start(); //Start timer to attempt to reconnect every 60 seconds
}

void AbstractDevice::disconnectFromDevice()
{
    qDebug() << Q_FUNC_INFO;

    m_autoreconnect = false;
    setConnectionState("disconnected");

    QBLEDevice::disconnectFromDevice();
}

void AbstractDevice::reconnectionTimer()
{
    //qDebug() << Q_FUNC_INFO;

    if ((!deviceProperty("Connected").toBool() && m_autoreconnect) || connectionState() == "authfailed") {
        qDebug() << Q_FUNC_INFO << "Lost connection";
        QBLEDevice::disconnectFromDevice();
        QBLEDevice::connectToDevice();
    }
}

void AbstractDevice::devicePairFinished(const QString &status)
{
    qDebug() << Q_FUNC_INFO;
    if (status == "paired") {
        setConnectionState("paired");
    }
}

void AbstractDevice::setConnectionState(const QString &state)
{
    qDebug() << Q_FUNC_INFO << "Connection state:" << state;
    if (state != m_connectionState) {
        m_connectionState = state;
        emit connectionStateChanged();
    }
}

void AbstractDevice::deviceError(const QString &msg)
{
    qDebug() << Q_FUNC_INFO << msg;
    Q_EMIT message(msg);
    setConnectionState("disconnected");
}

QString AbstractDevice::connectionState() const
{
    return m_connectionState;
}

bool AbstractDevice::supportsFeature(Amazfish::Feature f) const
{
    return (supportedFeatures() & f);
}

bool AbstractDevice::supportsDataType(Amazfish::DataType t) const
{
    return (supportedDataTypes() & t);
}

void AbstractDevice::setDatabase(KDbConnection *conn)
{
    m_conn = conn;
}

KDbConnection *AbstractDevice::database()
{
    return m_conn;
}

QString AbstractDevice::deviceName() const
{
    return m_pairedName;
}

void AbstractDevice::prepareFirmwareDownload(const AbstractFirmwareInfo *info)
{
   Q_UNUSED(info);
}

void AbstractDevice::startDownload()
{
}

void AbstractDevice::downloadSportsData()
{   
}

void AbstractDevice::downloadActivityData()
{
}

void AbstractDevice::fetchLogs()
{
}

void AbstractDevice::sendWeather(CurrentWeather *weather)
{
    Q_UNUSED(weather);
}

void AbstractDevice::refreshInformation()
{
    
}

QString AbstractDevice::information(Amazfish::Info i) const
{
    Q_UNUSED(i);
    return QString();
}

void AbstractDevice::applyDeviceSetting(Amazfish::Settings s)
{
    Q_UNUSED(s);
    emit message(tr("Device does not support settings"));
}

void AbstractDevice::rebootWatch()
{

}

//!
//! \brief AbstractDevice::sendEventReminder
//! \param id Optional identifer if required by the device
//! \param dt Date/time of event
//! \param event Event text
//!
void AbstractDevice::sendEventReminder(int id, const QDateTime &dt, const QString &event)
{
    Q_UNUSED(id);
    Q_UNUSED(dt);
    Q_UNUSED(event);
}

void AbstractDevice::enableFeature(Amazfish::Feature feature)
{
    Q_UNUSED(feature);
}

void AbstractDevice::setMusicStatus(bool playing, const QString &title, const QString &artist, const QString &album, int duration, int position)
{
    Q_UNUSED(playing);
    Q_UNUSED(title);
    Q_UNUSED(artist);
    Q_UNUSED(album);
}

void AbstractDevice::abortOperations()
{

}

void AbstractDevice::navigationRunning(bool running)
{
    Q_UNUSED(running);
}

void AbstractDevice::navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress)
{
    Q_UNUSED(flag)
    Q_UNUSED(narrative)
}

void AbstractDevice::requestScreenshot()
{
    qDebug() << Q_FUNC_INFO;
}

QStringList AbstractDevice::supportedDisplayItems() const
{
    return QStringList();
}

void AbstractDevice::immediateAlert(int level) {
    qDebug() << Q_FUNC_INFO << level;
}

void AbstractDevice::requestManualHeartrate() const
{
    HRMService *hrm = qobject_cast<HRMService*>(service(HRMService::UUID_SERVICE_HRM));
    if (hrm) {
        hrm->enableManualHRMeasurement(true);
    }
}

AbstractActivitySummaryParser *AbstractDevice::activitySummaryParser() const
{
    return nullptr;
}

AbstractActivityDetailParser *AbstractDevice::activityDetailParser() const
{
    return nullptr;
}

bool AbstractDevice::operationRunning()
{
    bool running = false;
    foreach(QBLEService* service, services()) {
        AbstractOperationService *a = qobject_cast<AbstractOperationService*>(service);
        if (a) {
            running |= a->operationRunning();
        }
    }
    return running;
}
