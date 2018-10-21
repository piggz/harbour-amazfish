#include "abstractdevice.h"

#include <QString>
    
AbstractDevice::AbstractDevice(QObject *parent) : QBLEDevice(parent)
{
    
}

QString AbstractDevice::prepareFirmwareDownload(const QString &path)
{
   Q_UNUSED(path)
    return "";
}

void AbstractDevice::startDownload()
{
    
}

void AbstractDevice::downloadSportsData()
{
    
}

void AbstractDevice::sendWeather(CurrentWeather *weather)
{
    Q_UNUSED(weather)
}

void AbstractDevice::refreshInformation()
{
    
}

QString AbstractDevice::information(Info i)
{
    Q_UNUSED(i)
}

void AbstractDevice::applyDeviceSettings(Settings s)
{
    Q_UNUSED(s)
    emit message(tr("Device doen not support settings"));
}