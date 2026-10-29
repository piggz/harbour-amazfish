#include "bipsdevice.h"
#include "typeconversion.h"

BipSDevice::BipSDevice(const QString &pairedName, QObject *parent) : BipDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO;
}

QString BipSDevice::deviceType() const
{
    return "amazfitbips";
}

Amazfish::Features BipSDevice::supportedFeatures() const
{
    return Amazfish::Feature::FEATURE_HRM |
            Amazfish::Feature::FEATURE_WEATHER |
            Amazfish::Feature::FEATURE_ACTIVITY |
            Amazfish::Feature::FEATURE_STEPS |
            Amazfish::Feature::FEATURE_ALARMS |
            Amazfish::Feature::FEATURE_ALERT |
            Amazfish::Feature::FEATURE_EVENT_REMINDER |
            Amazfish::Feature::FEATURE_MUSIC_CONTROL |
            Amazfish::Feature::FEATURE_FILE_INSTALL;
}

void BipSDevice::applyDeviceSetting(Amazfish::Settings s)
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (!mi) {
        return;
    }

    if (s == Amazfish::Settings::SETTING_DEVICE_DISPLAY_ITEMS) {
        mi->setDisplayItemsNew();
    } else {
        HuamiDevice::applyDeviceSetting(s);
    }
}

AbstractFirmwareInfo *BipSDevice::firmwareInfo(const QByteArray &bytes, const QString &path)
{
    return nullptr;
}

void BipSDevice::setMusicStatus(bool playing, const QString &artist, const QString &album, const QString &track, int duration, int position)
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi){
        mi->setMusicStatus(playing, artist, album, track, duration, position);
    }
}

void BipSDevice::serviceEvent(uint8_t event)
{
    switch(event) {
    case MiBandService::EVENT_MUSIC_PLAY:
        emit deviceEvent(AbstractDevice::EVENT_MUSIC_PLAY);
        break;
    case MiBandService::EVENT_MUSIC_PAUSE:
        emit deviceEvent(AbstractDevice::EVENT_MUSIC_PAUSE);
        break;
    case MiBandService::EVENT_MUSIC_NEXT:
        emit deviceEvent(AbstractDevice::EVENT_MUSIC_NEXT);
        break;
    case MiBandService::EVENT_MUSIC_PREV:
        emit deviceEvent(AbstractDevice::EVENT_MUSIC_PREV);
        break;
    case MiBandService::EVENT_MUSIC_VOLUP:
        emit deviceEvent(AbstractDevice::EVENT_MUSIC_VOLUP);
        break;
    case MiBandService::EVENT_MUSIC_VOLDOWN:
        emit deviceEvent(AbstractDevice::EVENT_MUSIC_VOLDOWN);
        break;
    case MiBandService::EVENT_MUSIC_OPEN:
        emit deviceEvent(AbstractDevice::EVENT_APP_MUSIC);
        break;
    case MiBandService::EVENT_DECLINE_CALL:
        emit deviceEvent(AbstractDevice::EVENT_DECLINE_CALL);
        break;
    case MiBandService::EVENT_IGNORE_CALL:
        emit deviceEvent(AbstractDevice::EVENT_IGNORE_CALL);
        break;
    default:
        break;
    }
}

QStringList BipSDevice::supportedDisplayItems() const
{
    QStringList items;

    items << "status";
    items << "pai";
    items << "hr";
    items << "workout";
    items << "alipay";
    items << "nfc";
    items << "activity";
    items << "weather";
    items << "music";
    items << "compass";
    items << "alarm";
    items << "worldclock";
    items << "timer_stopwatch";
    items << "settings";
    items << "compass";

    return items;
}

QBLEService *BipSDevice::drv_createService(const QString &uuid, const QString &path)
{
    qDebug() << Q_FUNC_INFO << uuid;

    return HuamiDevice::drv_createService(uuid, path);
}