#include "asteroidosdevice.h"

#include "batteryservice.h"
#include "asteroidtimeservice.h"
#include "asteroidweatherservice.h"
#include "asteroidnotificationservice.h"
#include "asteroidmediaservice.h"
#include "asteroidscreenshotservice.h"

#include <QtXml/QtXml>


AsteroidOSDevice::AsteroidOSDevice(const QString &pairedName, QObject *parent) : AbstractDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO << pairedName;
    connect(this, &QBLEDevice::propertiesChanged, this, &AsteroidOSDevice::onPropertiesChanged, Qt::UniqueConnection);
}

int AsteroidOSDevice::supportedFeatures() const
{
    return FEATURE_WEATHER | FEATURE_WEATHER | FEATURE_ALERT | FEATURE_SCREENSHOT;
//FEATURE_HRM  | FEATURE_STEPS;
}

QString AsteroidOSDevice::deviceType() const
{
    return "asteroidos";
}

AbstractFirmwareInfo *AsteroidOSDevice::firmwareInfo(const QByteArray &bytes)
{
    qDebug() << Q_FUNC_INFO;
    return nullptr;
}

void AsteroidOSDevice::sendAlert(const QString &sender, const QString &subject, const QString &message)
{
    qDebug() << Q_FUNC_INFO << sender << subject << message;

    AsteroidNotificationService *notification = qobject_cast<AsteroidNotificationService*>(service(AsteroidNotificationService::UUID_SERVICE_NOTIFICATION));
    if (notification) {
        notification->sendAlert(sender, subject, message);
    }

}

void AsteroidOSDevice::incomingCall(const QString &caller)
{
    qDebug() << Q_FUNC_INFO << caller;

    AsteroidNotificationService *notification = qobject_cast<AsteroidNotificationService*>(service(AsteroidNotificationService::UUID_SERVICE_NOTIFICATION));
    if (notification) {
        notification->incomingCall(caller);
    }

}


void AsteroidOSDevice::pair()
{
    qDebug() << Q_FUNC_INFO;

    m_needsAuth = true;
    m_pairing = true;
    m_autoreconnect = true;
    //disconnectFromDevice();
    setConnectionState("pairing");
    emit connectionStateChanged();

    QBLEDevice::connectToDevice();
}


/*
//Firmware handling
AbstractFirmwareInfo * AsteroidOSDevice::firmwareInfo(const QByteArray &bytes) = 0; //Caller owns the pointer and should delete it
void AsteroidOSDevice::prepareFirmwareDownload(const AbstractFirmwareInfo* info);
void AsteroidOSDevice::startDownload();

void AsteroidOSDevice::downloadSportsData();
void AsteroidOSDevice::refreshInformation();
QString AsteroidOSDevice::information(Info i) const;
void AsteroidOSDevice::applyDeviceSetting(Settings s);
void AsteroidOSDevice::rebootWatch();
void AsteroidOSDevice::sendEventReminder(int id, const QDateTime &dt, const QString &event);
void AsteroidOSDevice::enableFeature(AbstractDevice::Feature feature);
void AsteroidOSDevice::setMusicStatus(bool playing, const QString &title, const QString &artist, const QString &album, int duration = 0, int position = 0);
void AsteroidOSDevice::navigationRunning(bool running);
void AsteroidOSDevice::navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress);
QStringList AsteroidOSDevice::supportedDisplayItems() const;
*/


void AsteroidOSDevice::onPropertiesChanged(QString interface, QVariantMap map, QStringList list)
{
    qDebug() << Q_FUNC_INFO << interface << map << list;

    if (interface == "org.bluez.Device1") {
        m_reconnectTimer->start();
        if (deviceProperty("ServicesResolved").toBool() ) {
            initialise();
        }
        if (map.contains("Connected")) {
            bool value = map["Connected"].toBool();

            if (!value) {
                setConnectionState("disconnected");
            } else {
                setConnectionState("connected");
            }
        }
    }

}


void AsteroidOSDevice::parseServices()
{
    qDebug() << Q_FUNC_INFO;

    QDBusInterface adapterIntro("org.bluez", devicePath(), "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus(), 0);
    QDBusReply<QString> xml = adapterIntro.call("Introspect");

    qDebug() << "Resolved services...";

    qDebug().noquote() << xml.value();

    QDomDocument doc;
    doc.setContent(xml.value());

    QDomNodeList nodes = doc.elementsByTagName("node");

    qDebug() << nodes.count() << "nodes";

    for (int x = 0; x < nodes.count(); x++)
    {
        QDomElement node = nodes.at(x).toElement();
        QString nodeName = node.attribute("name");

        if (nodeName.startsWith("service")) {
            QString path = devicePath() + "/" + nodeName;

            QDBusInterface devInterface("org.bluez", path, "org.bluez.GattService1", QDBusConnection::systemBus(), 0);
            QString uuid = devInterface.property("UUID").toString();

            qDebug() << "Creating service for: " << uuid;

            if (uuid == BatteryService::UUID_SERVICE_BATTERY && !service(BatteryService::UUID_SERVICE_BATTERY)) {
                addService(BatteryService::UUID_SERVICE_BATTERY, new BatteryService(path, this));
            } else if (uuid == AsteroidTimeService::UUID_SERVICE_ASTEROID_TIME && !service(AsteroidTimeService::UUID_SERVICE_ASTEROID_TIME)) {
                addService(AsteroidTimeService::UUID_SERVICE_ASTEROID_TIME, new AsteroidTimeService(path, this));
            } else if (uuid == AsteroidWeatherService::UUID_SERVICE_WEATHER && !service(AsteroidWeatherService::UUID_SERVICE_WEATHER)) {
                addService(AsteroidWeatherService::UUID_SERVICE_WEATHER, new AsteroidWeatherService(path, this));
            } else if (uuid == AsteroidNotificationService::UUID_SERVICE_NOTIFICATION && !service(AsteroidNotificationService::UUID_SERVICE_NOTIFICATION)) {
                addService(AsteroidNotificationService::UUID_SERVICE_NOTIFICATION, new AsteroidNotificationService(path, this));
            } else if (uuid == AsteroidMediaService::UUID_SERVICE_MEDIA  && !service(AsteroidMediaService::UUID_SERVICE_MEDIA  )) {
                addService(AsteroidMediaService::UUID_SERVICE_MEDIA  , new AsteroidMediaService(path, this));
            } else if (uuid == AsteroidScreenshotService::UUID_SERVICE_SCREENSHOT  && !service(AsteroidScreenshotService::UUID_SERVICE_SCREENSHOT  )) {
                addService(AsteroidScreenshotService::UUID_SERVICE_SCREENSHOT, new AsteroidScreenshotService(path, this));
            } else if ( !service(uuid)) {
                addService(uuid, new QBLEService(uuid, path, this));
            }
        }
    }
    setConnectionState("authenticated");
}

void AsteroidOSDevice::initialise()
{
    qDebug() << Q_FUNC_INFO;
    setConnectionState("connected");
    parseServices();

    AsteroidNotificationService *notification = qobject_cast<AsteroidNotificationService*>(service(AsteroidNotificationService::UUID_SERVICE_NOTIFICATION));
    if (notification) {
//        notification->enableNotification(AsteroidNotificationService::UUID_CHARACTERISTIC_ALERT_NOTIFICATION_EVENT);
//        connect(alert, &AlertNotificationService::serviceEvent, this, &PinetimeJFDevice::serviceEvent, Qt::UniqueConnection);
    }


    BatteryService *battery = qobject_cast<BatteryService*>(service(BatteryService::UUID_SERVICE_BATTERY));
    if (battery) {
        connect(battery, &BatteryService::informationChanged, this, &AsteroidOSDevice::informationChanged, Qt::UniqueConnection);
    }


    AsteroidTimeService *ats = qobject_cast<AsteroidTimeService*>(service(AsteroidTimeService::UUID_SERVICE_ASTEROID_TIME));
    if (ats) {
        ats->setCurrentTime();
    }


    AsteroidMediaService *ms = qobject_cast<AsteroidMediaService*>(service(AsteroidMediaService::UUID_SERVICE_MEDIA));
    if (ms) {
        ms->enableNotification(AsteroidMediaService::UUID_CHARACTERISTIC_MEDIA_COMMAND);
        connect(ms, &AsteroidMediaService::serviceEvent, this, &AsteroidOSDevice::serviceEvent, Qt::UniqueConnection);
    }

    AsteroidScreenshotService *screenshot = qobject_cast<AsteroidScreenshotService*>(service(AsteroidScreenshotService::UUID_SERVICE_SCREENSHOT));
    if (screenshot) {
        connect(screenshot, &AsteroidScreenshotService::screenshotReceived, this, &AsteroidOSDevice::screenshotReceived, Qt::UniqueConnection);
    }

}


void AsteroidOSDevice::authenticated(bool ready)
{
    qDebug() << Q_FUNC_INFO << ready;

    if (ready) {
        setConnectionState("authenticated");
    } else {
        setConnectionState("authfailed");
    }
}

void AsteroidOSDevice::refreshInformation()
{

    BatteryService *bat = qobject_cast<BatteryService*>(service(BatteryService::UUID_SERVICE_BATTERY));
    if (bat) {
        bat->refreshInformation();
    }

}

void AsteroidOSDevice::sendWeather(CurrentWeather *weather)
{
    AsteroidWeatherService *w = qobject_cast<AsteroidWeatherService*>(service(AsteroidWeatherService::UUID_SERVICE_WEATHER));
    if (w){
        w->sendWeather(weather);
    }
}

void AsteroidOSDevice::setMusicStatus(bool playing, const QString &title, const QString &artist, const QString &album, int duration, int position)
{
    Q_UNUSED(duration)
    Q_UNUSED(position)

    AsteroidMediaService *media = qobject_cast<AsteroidMediaService*>(service(AsteroidMediaService::UUID_SERVICE_MEDIA));
    if (media) {
        media->setStatus(playing);
        media->setAlbum(album);
        media->setTrack(title);
        media->setArtist(artist);
    }
}

void AsteroidOSDevice::serviceEvent(const QString &characteristic, uint8_t event, uint8_t data)
{
    if (characteristic == AsteroidMediaService::UUID_CHARACTERISTIC_MEDIA_COMMAND) {
        switch(event) {
            case AsteroidMediaService::EVENT_MEDIA_PREV:
                emit deviceEvent(AbstractDevice::EVENT_MUSIC_PREV);
            break;
            case AsteroidMediaService::EVENT_MEDIA_NEXT:
                emit deviceEvent(AbstractDevice::EVENT_MUSIC_NEXT);
            break;
            case AsteroidMediaService::EVENT_MEDIA_PLAY:
                emit deviceEvent(AbstractDevice::EVENT_MUSIC_PLAY);
            break;
            case AsteroidMediaService::EVENT_MEDIA_PAUSE:
                emit deviceEvent(AbstractDevice::EVENT_MUSIC_PAUSE);
            break;
            case AsteroidMediaService::EVENT_MEDIA_VOL:
        qDebug() << Q_FUNC_INFO << "Command volume" << characteristic << event << data;
                if (data < m_prevVolume) {
                    emit deviceEvent(AbstractDevice::EVENT_MUSIC_VOLDOWN);
                } else {
                    emit deviceEvent(AbstractDevice::EVENT_MUSIC_VOLUP);
                }
                m_prevVolume = data;
                AsteroidMediaService *media = qobject_cast<AsteroidMediaService*>(service(AsteroidMediaService::UUID_SERVICE_MEDIA));
                if (media) {
                    media->setVolume(data);
                }

            break;


        }
    } else {
        qDebug() << Q_FUNC_INFO << characteristic << event << data;
    }

}

void AsteroidOSDevice::screenshotReceived(QByteArray data) {

    QString filename = "screenshot_" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss") + ".jpg";
    QDir picturelocation = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (!picturelocation.exists()) {
        qDebug() << "Creating picture amazfish folder";
        if (!picturelocation.mkpath("")) {
            qDebug() << "Error creating amazfish picture folder!";
            return;
        }
    }

    QString fullpath = picturelocation.absolutePath() + "/" + filename;
    QFile screenshotFile(fullpath);

    qDebug() << fullpath << data.size();

    if (!screenshotFile.open(QIODevice::WriteOnly)) {
        qWarning() << "cannot open " << fullpath;
    }

    screenshotFile.write(data);
    screenshotFile.close();

    emit message(tr("Stored %1...").arg(filename));

}

void AsteroidOSDevice::requestScreenshot() {
    qDebug() << Q_FUNC_INFO;

    AsteroidScreenshotService *screenshot = qobject_cast<AsteroidScreenshotService*>(service(AsteroidScreenshotService::UUID_SERVICE_SCREENSHOT));
    if (screenshot) {
        screenshot->requestScreenshot();
    }

}