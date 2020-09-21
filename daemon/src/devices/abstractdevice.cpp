#include "abstractdevice.h"

#include <QString>
    
AbstractDevice::AbstractDevice(const QString &pairedName, QObject *parent) : QBLEDevice(parent)
{
    setConnectionState("disconnected");
    m_pairedName = pairedName;
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(60000);
    connect(m_reconnectTimer, &QTimer::timeout, this, &AbstractDevice::reconnectionTimer);
}

QString AbstractDevice::pair()
{
    qDebug() << "AbstractDevice::pair";

    m_needsAuth = true;
    m_pairing = true;
    m_autoreconnect = true;
    //disconnectFromDevice();
    setConnectionState("pairing");

    QBLEDevice::pair();
    QBLEDevice::connectToDevice();
    return "pairing";
}

void AbstractDevice::pairAsync()
{
    qDebug() << "AbstractDevice::pairAsync";

    m_needsAuth = true;
    m_pairing = true;
    m_autoreconnect = true;
    //disconnectFromDevice();
    setConnectionState("pairing");

    QBLEDevice::pairAsync();
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
    qDebug() << "AbstractDevice::reconnectionTimer";
    if ((!deviceProperty("Connected").toBool() && m_autoreconnect) || connectionState() == "authfailed") {
        qDebug() << "Lost connection";
        QBLEDevice::disconnectFromDevice();
        QBLEDevice::connectToDevice();
    }
}

void AbstractDevice::setConnectionState(const QString &state)
{
    if (state != m_connectionState) {
        m_connectionState = state;
        emit connectionStateChanged();
    }
}

QString AbstractDevice::connectionState() const
{
    return m_connectionState;
}

bool AbstractDevice::supportsFeature(AbstractDevice::Feature f)
{
    return (supportedFeatures() & f) == f;
}

QString AbstractDevice::deviceName()
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

void AbstractDevice::sendWeather(CurrentWeather *weather)
{
    Q_UNUSED(weather);
}

void AbstractDevice::refreshInformation()
{
    
}

QString AbstractDevice::information(Info i)
{
    Q_UNUSED(i);
    return QString();
}

void AbstractDevice::applyDeviceSetting(Settings s)
{
    Q_UNUSED(s);
    emit message(tr("Device doen not support settings"));
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

void AbstractDevice::setMusicStatus(bool playing, const QString &title, const QString &artist, const QString &album)
{
    Q_UNUSED(playing);
    Q_UNUSED(title);
    Q_UNUSED(artist);
    Q_UNUSED(album);
}

void AbstractDevice::abortOperations()
{

}
