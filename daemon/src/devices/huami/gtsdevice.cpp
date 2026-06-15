#include "gtsdevice.h"
#include "gtsfirmwareinfo.h"
#include <QDateTime>
#include "typeconversion.h"
#include "huami/updatefirmwareoperationnew.h"
#include "huami/huamiupdatefirmwareoperation2020.h"
#include "mibandservice.h"
#include "miband2service.h"
#include "deviceinfoservice.h"
#include "bipfirmwareservice.h"
#include "hrmservice.h"

GtsDevice::GtsDevice(const QString &pairedName, QObject *parent) : HuamiDevice(pairedName, parent)
{
    qDebug() << "Creating GTS Device";
}

QString GtsDevice::deviceType() const
{
    return "amazfitgts";
}

Amazfish::Features GtsDevice::supportedFeatures() const
{
    return Amazfish::Feature::FEATURE_HRM |
            Amazfish::Feature::FEATURE_WEATHER |
            Amazfish::Feature::FEATURE_ACTIVITY |
            Amazfish::Feature::FEATURE_STEPS |
            Amazfish::Feature::FEATURE_ALARMS |
            Amazfish::Feature::FEATURE_ALERT |
            Amazfish::Feature::FEATURE_EVENT_REMINDER |
            Amazfish::Feature::FEATURE_MUSIC_CONTROL |
            Amazfish::Feature::FEATURE_BUTTON_ACTION |
            Amazfish::Feature::FEATURE_FILE_INSTALL;
}

Amazfish::DataTypes GtsDevice::supportedDataTypes() const
{
    return Amazfish::DataType::TYPE_ACTIVITY | Amazfish::DataType::TYPE_GPS_TRACK | Amazfish::DataType::TYPE_HEART_RATE;
}

void GtsDevice::serviceEvent(uint8_t event)
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
    case MiBandService::EVENT_FIND_PHONE:
        emit deviceEvent(AbstractDevice::EVENT_FIND_PHONE);
        break;
    case MiBandService::EVENT_CANCEL_FIND_PHONE:
        emit deviceEvent(AbstractDevice::EVENT_CANCEL_FIND_PHONE);
        break;
    default:
        break;
    }
}

void GtsDevice::initialise()
{
    qDebug() << Q_FUNC_INFO;
    setConnectionState("connected");
    parseServices();

    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        connect(mi, &MiBandService::message, this, &HuamiDevice::message, Qt::UniqueConnection);
        connect(mi, &MiBandService::buttonPressed, this, &GtsDevice::handleButtonPressed, Qt::UniqueConnection);
        connect(mi, &MiBandService::informationChanged, this, &HuamiDevice::informationChanged, Qt::UniqueConnection);
        connect(mi, &MiBandService::serviceEvent, this, &GtsDevice::serviceEvent, Qt::UniqueConnection);
        connect(mi, &QBLEService::characteristicChanged, this, &GtsDevice::characteristicChanged, Qt::UniqueConnection);
    }

    MiBand2Service *mi2 = qobject_cast<MiBand2Service*>(service(MiBand2Service::UUID_SERVICE_MIBAND2));
    if (mi2) {
        qDebug() << "Got MiBand2 service";
        connect(mi2, &MiBand2Service::authenticated, this, &HuamiDevice::authenticated, Qt::UniqueConnection);
        //connect(mi2, &AbstractOperationService::operationRunningChanged, this, &AbstractDevice::operationRunningChanged, Qt::UniqueConnection);

        mi2->enableNotification(MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH);
        mi2->initialise(false);
    }

    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(BipFirmwareService::UUID_SERVICE_FIRMWARE));
    if (fw) {
        connect(fw, &BipFirmwareService::message, this, &HuamiDevice::message, Qt::UniqueConnection);
        connect(fw, &AbstractOperationService::operationRunningChanged, this, &AbstractDevice::operationRunningChanged, Qt::UniqueConnection);
    }

    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (info) {
        connect(info, &DeviceInfoService::informationChanged, this, &HuamiDevice::informationChanged, Qt::UniqueConnection);
    }

    HRMService *hrm = qobject_cast<HRMService*>(service(HRMService::UUID_SERVICE_HRM));
    if (hrm) {
        connect(hrm, &HRMService::informationChanged, this, &HuamiDevice::informationChanged, Qt::UniqueConnection);
    }

    QString revision = softwareRevision();
    if (revision > "0.0.9.0") {
        qDebug() << Q_FUNC_INFO << "GTS with new FW";
        m_ActivitySampleSize = 8;
    }
}


AbstractFirmwareInfo *GtsDevice::firmwareInfo(const QByteArray &bytes, const QString &path)
{
    return new GtsFirmwareInfo(bytes);
}

void GtsDevice::syncCalendar(QList<watchfish::CalendarEvent> &eventlist)
{
    int id=0;
    foreach (const watchfish::CalendarEvent &event, eventlist) {
        qDebug() << event.uid() << event.title() << event.start();
        sendEventReminder(id, event.start(), event.title());
        id++;
    }
}


void GtsDevice::sendEventReminder(int id, const QDateTime &dt, const QString &event)
{
    //Send event reminder
    //Type: 02
    //00 0b Always 0b
    //01 01 ID
    //02 09 Flags 0x01 = Enable, 0x04 = End Date Preset, 0x08 = Text Present
    //03 00
    //04 00
    //05 00
    //06 Date/Time (6)
    //00
    //MESSAGE
    qDebug() << Q_FUNC_INFO << dt << event;

    QByteArray cmd;
    cmd += (char)0x0b;
    cmd += (char)id;
    cmd += (char)0x09;
    cmd += (char)0x00;
    cmd += (char)0x00;
    cmd += (char)0x00;
    cmd += TypeConversion::dateTimeToBytes(dt, 0).left(6);
    cmd += (char)0x00;
    cmd += event.toLocal8Bit();
    cmd += (char)0x00;

    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        mi->writeChunked(MiBandService::UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 2, cmd);
    }
}

void GtsDevice::prepareFirmwareDownload(const AbstractFirmwareInfo *info)
{
    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(BipFirmwareService::UUID_SERVICE_FIRMWARE));
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));

    if (fw && mi){
        if (fw->currentOperation()) {
            emit message(tr("An operation is currently running, please try later"));
            return;
        }

        QString revision = softwareRevision();
        AbstractOperation *operation;
        if (revision > "0.1.1.16") {
            operation =  new HuamiUpdateFirmwareOperation2020(info, fw, mi, this);
        } else {
            operation =  new UpdateFirmwareOperationNew(info, fw, this);
        }


        if (fw->registerOperation(operation)) {
            emit operationRunningChanged();
        } else {
            delete operation;
        }
    }


}

void GtsDevice::applyDeviceSetting(Amazfish::Settings s)
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

void GtsDevice::sendWeather(CurrentWeather *weather)
{
    sendWeatherHuami(weather, true);
}

void GtsDevice::enableFeature(Amazfish::Feature feature)
{
    qDebug() << Q_FUNC_INFO << feature;
    if (feature == Amazfish::Feature::FEATURE_MUSIC_CONTROL) {
        QByteArray cmd;

        cmd += (char)0x01;
        cmd += (char)0x00;
        cmd += (char)0x01;
        cmd += (char)0x00;
        cmd += (char)0x00;
        cmd += (char)0x00;
        cmd += (char)0x01;
        cmd += (char)0x00;
        MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
        if (mi){
            mi->writeChunked(MiBandService::UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 3, cmd);
        }

        QString track = "Test track";
        QString album = "Awesome Album";
        QString artist = "An Artist";

        char flags = 0x00;
        flags |= 0x01;

        flags |= 0x02;
        flags |= 0x04;
        flags |= 0x08;

        char state = 0x00; //Not playing
        cmd.clear();
        cmd += flags;
        cmd += state;

        //Unknown
        cmd += (char)0x01;
        cmd += (char)0x00;
        cmd += (char)0x00;
        cmd += (char)0x00;

        //Show Track
        cmd += (char)0x01;
        cmd += (char)0x00;

        cmd += track.toLocal8Bit();
        cmd += char(0x00);
        cmd += album.toLocal8Bit();
        cmd += char(0x00);
        cmd += artist.toLocal8Bit();
        cmd += char(0x00);

        if (mi){
            mi->writeChunked(MiBandService::UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 3, cmd);
        }
    }
}

void GtsDevice::setMusicStatus(bool playing, const QString &artist, const QString &album, const QString &track, int duration, int position)
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi){
        mi->setMusicStatus(playing, artist, album, track, duration, position);
    }
}

void GtsDevice::navigationRunning(bool running)
{
    m_navigationRunning = running;
    if (m_navigationRunning) {
        setMusicStatus(m_navigationRunning,"", "", "");
    }
}

void GtsDevice::navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress)
{
    Q_UNUSED(flag)
    setMusicStatus(m_navigationRunning, narrative, "", manDist, 1000, progress * 10);
}

QStringList GtsDevice::supportedDisplayItems() const
{
    QStringList items;

    items << "status";
    items << "pai";
    items << "hr";
    items << "workout";
    items << "activity";
    items << "weather";
    items << "music";
    items << "notifications";
    items << "alarm";
    items << "eventreminder";
    items << "more";
    items << "settings";

    return items;
}

void GtsDevice::sendAlert(const Amazfish::WatchNotification &notification)
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        mi->sendAlert(notification.appName, notification.summary, notification.body);
    }
}
