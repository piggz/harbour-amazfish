#include "abstractdevice.h"

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
    qDebug() << "AbstractDevice::connectToDevice";

    m_pairing = false;
    m_autoreconnect = true;
    QBLEDevice::disconnectFromDevice();
    setConnectionState("connecting");
    QBLEDevice::connectToDevice();
    m_reconnectTimer->start(); //Start timer to attempt to reconnect every 60 seconds
}

void AbstractDevice::disconnectFromDevice()
{
    qDebug() << "AbstractDevice::disconnectFromDevice";

    m_autoreconnect = false;
    setConnectionState("disconnected");

    QBLEDevice::disconnectFromDevice();
}

void AbstractDevice::reconnectionTimer()
{
    //qDebug() << Q_FUNC_INFO;

    if ((!deviceProperty("Connected").toBool() && m_autoreconnect) || connectionState() == "authfailed") {
        qDebug() << "Lost connection";
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
    qDebug() << Q_FUNC_INFO << state;
    if (state != m_connectionState) {
        m_connectionState = state;
        emit connectionStateChanged();
    }
}

QString AbstractDevice::connectionState() const
{
    return m_connectionState;
}

bool AbstractDevice::supportsFeature(AbstractDevice::Feature f) const
{
    return (supportedFeatures() & f) == f;
}

void AbstractDevice::setDatabase(KDbConnection *conn)
{
    m_conn = conn;
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

QString AbstractDevice::information(Info i) const
{
    Q_UNUSED(i);
    return QString();
}

void AbstractDevice::applyDeviceSetting(Settings s)
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

void AbstractDevice::enableFeature(AbstractDevice::Feature feature)
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

QStringList AbstractDevice::supportedDisplayItems() const
{
    return QStringList();
}
