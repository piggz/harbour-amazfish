#include "abstractdevice.h"

#include <QString>
    
QString AbstractDevice::prepareFirmwareDownload(const QString &path)
{
   Q_UNUSED(path)   
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