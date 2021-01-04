#include "deviceinterface.h"

#include "deviceinfoservice.h"
#include "mibandservice.h"
#include "miband2service.h"
#include "alertnotificationservice.h"
#include "hrmservice.h"
#include "bipfirmwareservice.h"
#include "devicefactory.h"
#include "amazfishconfig.h"

#include <QDir>
#include <KDb3/KDbDriverManager>

static const char *SERVICE = SERVICE_NAME;
static const char *PATH = "/application";

DeviceInterface::DeviceInterface()
{
    //Start by registering on DBUS
    registerDBus();

    auto config = AmazfishConfig::instance();

    //Create a device object
    m_device = DeviceFactory::createDevice(config->pairedName());
    if (m_device) {
        connect(m_device, &AbstractDevice::connectionStateChanged, this, &DeviceInterface::onConnectionStateChanged);
        connect(m_device, &AbstractDevice::message, this, &DeviceInterface::message);
        connect(m_device, &AbstractDevice::downloadProgress, this, &DeviceInterface::downloadProgress);
        connect(m_device, &QBLEDevice::operationRunningChanged, this, &DeviceInterface::operationRunningChanged);
        connect(m_device, &AbstractDevice::buttonPressed, this, &DeviceInterface::handleButtonPressed);
        connect(m_device, &AbstractDevice::informationChanged, this, &DeviceInterface::slot_informationChanged);
        connect(m_device, &AbstractDevice::deviceEvent, this, &DeviceInterface::deviceEvent);
    }

    //Create the DBUS HRM Interface
    m_dbusHRM = new DBusHRM(this);

    setupDatabase();

    //Notifications
    connect(&m_notificationMonitor, &watchfish::NotificationMonitor::notification, this, &DeviceInterface::onNotification);

    // Calls
#ifdef MER_EDITION_SAILFISH
    connect(&m_voiceCallController, &watchfish::VoiceCallController::ringingChanged, this, &DeviceInterface::onRingingChanged);
#endif
    //Weather
    connect(&m_cityManager, &CityManager::citiesChanged, this, &DeviceInterface::onCitiesChanged);
    connect(&m_currentWeather, &CurrentWeather::ready, this, &DeviceInterface::onWeatherReady);
    if (m_cityManager.cities().count() > 0) {
        m_currentWeather.setCity(qobject_cast<City*>(m_cityManager.cities()[0]));
    }

    //Refresh timer
    m_refreshTimer = new QTimer();
    connect(m_refreshTimer, &QTimer::timeout, this, &DeviceInterface::onRefreshTimer);
    m_refreshTimer->start(60000);

    //Music
    connect(&m_musicController, &watchfish::MusicController::statusChanged, this, &DeviceInterface::musicChanged);
    connect(&m_musicController, &watchfish::MusicController::titleChanged, this, &DeviceInterface::musicChanged);
    connect(&m_musicController, &watchfish::MusicController::artistChanged, this, &DeviceInterface::musicChanged);
    connect(&m_musicController, &watchfish::MusicController::albumChanged, this, &DeviceInterface::musicChanged);

    //Navigation
    connect(&m_navigationInterface, &NavigationInterface::runningChanged, this, &DeviceInterface::navigationRunningChanged);
    connect(&m_navigationInterface, &NavigationInterface::navigationChanged, this, &DeviceInterface::navigationChanged);

    //Finally, connect to device if it is defined
    QString pairedAddress = config->pairedAddress();
    if (!pairedAddress.isEmpty()) {
        connectToDevice(pairedAddress);
    }
}

DeviceInterface::~DeviceInterface()
{
    disconnect();

}

void DeviceInterface::connectToDevice(const QString &address)
{
    qDebug() << "DeviceInterface::connectToDevice:" << address;

    if (m_device) {
        m_deviceAddress = address;
        m_device->setDevicePath(address);
        m_device->connectToDevice();
    }
    else {
        qDebug() << "DeviceInterface::connectToDevice:device was not valid";
        message(tr("Device is not valid, it may not be supported"));
    }
}

QString DeviceInterface::pair(const QString &name, const QString &address)
{
    qDebug() << "DeviceInterface::pair:" << name << address;
    m_deviceAddress = address;

    if (m_device) {
        delete m_device;
    }
    m_device = DeviceFactory::createDevice(name);

    if (m_device) {
        m_device->setDevicePath(address);
        connect(m_device, &AbstractDevice::connectionStateChanged, this, &DeviceInterface::onConnectionStateChanged);
        connect(m_device, &AbstractDevice::message, this, &DeviceInterface::message);
        connect(m_device, &AbstractDevice::downloadProgress, this, &DeviceInterface::downloadProgress);
        connect(m_device, &QBLEDevice::operationRunningChanged, this, &DeviceInterface::operationRunningChanged);
        connect(m_device, &AbstractDevice::buttonPressed, this, &DeviceInterface::buttonPressed);
        connect(m_device, &AbstractDevice::informationChanged, this, &DeviceInterface::slot_informationChanged);
        connect(m_device, &AbstractDevice::deviceEvent, this, &DeviceInterface::deviceEvent);
        return m_device->pair();
    }
    
    qDebug() << "DeviceInterface::pair:device not created";

    return QString("no device found");
}

void DeviceInterface::disconnect()
{
    qDebug() << "DeviceInterface::disconnect";
    if (m_device) {
        m_device->disconnectFromDevice();
    }
}

QString DeviceInterface::connectionState() const
{
    if (!m_device) {
        return QString();
    }
    return m_device->connectionState();
}

MiBandService *DeviceInterface::miBandService() const
{
    return qobject_cast<MiBandService*>(m_device->service(MiBandService::UUID_SERVICE_MIBAND));
}

HRMService *DeviceInterface::hrmService() const
{
    return qobject_cast<HRMService*>(m_device->service(HRMService::UUID_SERVICE_HRM));
}

void DeviceInterface::onNotification(watchfish::Notification *notification)
{
    if (m_device && m_device->connectionState() == "authenticated" && m_device->supportsFeature(AbstractDevice::FEATURE_ALERT)){
        qDebug() << "Sending alert to device";
        m_device->sendAlert(notification->appName(), notification->summary(), notification->body());
    } else {
        qDebug() << "no notification service, buffering notification";

        m_notificationBuffer.enqueue(notification);
        if (m_notificationBuffer.count() > 10) {
            m_notificationBuffer.dequeue();
        }
    }
}

void DeviceInterface::onRingingChanged()
{
#ifdef MER_EDITION_SAILFISH
    qDebug() << Q_FUNC_INFO << m_voiceCallController.ringing();

    if (!m_device) {
        return;
    }

    if (m_voiceCallController.ringing()) {
        m_device->incomingCall(m_voiceCallController.findPersonByNumber(m_voiceCallController.callerId()));
    } else {
        if (m_device->service("00001802-0000-1000-8000-00805f9b34fb")){
            m_device->service("00001802-0000-1000-8000-00805f9b34fb")->writeValue("00002a06-0000-1000-8000-00805f9b34fb", QByteArray(1, 0x00)); //TODO properly abstract immediate notification service
        }
    }
#endif
}

void DeviceInterface::createTables()
{
    m_conn->setAutoCommit(false);

    KDbTransaction t = m_conn->beginTransaction();
    if (m_conn->result().isError()) {
        qDebug() << m_conn->result();
        return;
    }

    //now: lets create tables:
    KDbField *f;

    if (!m_conn->containsTable("mi_band_activity")) {
        KDbTableSchema *t_activity = new KDbTableSchema("mi_band_activity");
        t_activity->setCaption("Activity data samples");
        t_activity->addField(f = new KDbField("id", KDbField::Integer, KDbField::PrimaryKey | KDbField::AutoInc, KDbField::Unsigned));
        f->setCaption("ID");
        t_activity->addField(f = new KDbField("timestamp", KDbField::Integer, nullptr, KDbField::Unsigned));
        f->setCaption("Timestamp");
        t_activity->addField(f = new KDbField("timestamp_dt", KDbField::DateTime));
        f->setCaption("Timestamp in Date/Time format");
        t_activity->addField(f = new KDbField("device_id", KDbField::Integer, nullptr, KDbField::Unsigned));
        f->setCaption("Device ID");
        t_activity->addField(f = new KDbField("user_id", KDbField::Integer, nullptr, KDbField::Unsigned));
        f->setCaption("User ID");
        t_activity->addField(f = new KDbField("raw_intensity", KDbField::Integer, nullptr, KDbField::Unsigned));
        f->setCaption("Raw Intensity");
        t_activity->addField(f = new KDbField("steps", KDbField::Integer, nullptr, KDbField::Unsigned));
        f->setCaption("Step Count");
        t_activity->addField(f = new KDbField("raw_kind", KDbField::Integer, nullptr, KDbField::Unsigned));
        f->setCaption("Activity Kind");
        t_activity->addField(f = new KDbField("heartrate", KDbField::Integer, nullptr, KDbField::Unsigned));
        f->setCaption("Heartrate");

        if (!m_conn->createTable(t_activity)) {
            qDebug() << m_conn->result();
            return;
        }
        qDebug() << "-- mi_band_activity created --";
        qDebug() << *t_activity;
    }


    if (!m_conn->containsTable("sports_data")) {
        KDbTableSchema *t_summary = new KDbTableSchema("sports_data");
        t_summary->setCaption("Sports Data");
        t_summary->addField(f = new KDbField("id", KDbField::Integer, KDbField::PrimaryKey | KDbField::AutoInc, KDbField::Unsigned));
        f->setCaption("ID");

        t_summary->addField(f = new KDbField("name", KDbField::Text));
        f->setCaption("Name");

        t_summary->addField(f = new KDbField("version", KDbField::Integer, nullptr, KDbField::Unsigned));
        f->setCaption("Version");

        t_summary->addField(f = new KDbField("start_timestamp", KDbField::Integer, nullptr, KDbField::Unsigned));
        f->setCaption("Start Timestamp");
        t_summary->addField(f = new KDbField("start_timestamp_dt", KDbField::DateTime));
        f->setCaption("Start Timestamp in Date/Time format");

        t_summary->addField(f = new KDbField("end_timestamp", KDbField::Integer, nullptr, KDbField::Unsigned));
        f->setCaption("End Timestamp");
        t_summary->addField(f = new KDbField("end_timestamp_dt", KDbField::DateTime));
        f->setCaption("End Timestamp in Date/Time format");

        t_summary->addField(f = new KDbField("kind", KDbField::Integer, KDbField::NoConstraints, KDbField::Unsigned));
        f->setCaption("Activity Kind");

        t_summary->addField(f = new KDbField("base_longitude", KDbField::Double));
        f->setCaption("Base Longitude");
        t_summary->addField(f = new KDbField("base_latitude", KDbField::Double));
        f->setCaption("Base Latitude");
        t_summary->addField(f = new KDbField("base_altitude", KDbField::Double));
        f->setCaption("Base Altitude");

        t_summary->addField(f = new KDbField("device_id", KDbField::Integer, KDbField::NoConstraints, KDbField::Unsigned));
        f->setCaption("Device ID");
        t_summary->addField(f = new KDbField("user_id", KDbField::Integer, KDbField::NoConstraints, KDbField::Unsigned));
        f->setCaption("User ID");

        t_summary->addField(f = new KDbField("gpx", KDbField::LongText));
        f->setCaption("GPX Data");

        if (!m_conn->createTable(t_summary)) {
            qDebug() << m_conn->result();
            return;
        }
        qDebug() << "-- sports_data created --";
        qDebug() << *t_summary;
    }

    if (!m_conn->containsTable("sports_meta")) {
        KDbTableSchema *t_meta = new KDbTableSchema("sports_meta");
        t_meta->setCaption("Sports Meta-Data");
        t_meta->addField(f = new KDbField("id", KDbField::Integer, KDbField::PrimaryKey | KDbField::AutoInc, KDbField::Unsigned));
        f->setCaption("ID");

        t_meta->addField(f = new KDbField("sport_id", KDbField::Integer, KDbField::NoConstraints, KDbField::Unsigned));
        f->setCaption("Sport ID");

        t_meta->addField(f = new KDbField("key", KDbField::Text));
        f->setCaption("Key");

        t_meta->addField(f = new KDbField("value", KDbField::Text));
        f->setCaption("Value");

        t_meta->addField(f = new KDbField("unit", KDbField::Text));
        f->setCaption("Unit");

        if (!m_conn->createTable(t_meta)) {
            qDebug() << m_conn->result();
            return;
        }
        qDebug() << "-- sports_meta created --";
        qDebug() << *t_meta;
    }

    if (!m_conn->commitTransaction(t)) {
        qDebug() << m_conn->result();
        return;
    }

    qDebug() << m_conn->tableNames();
}

void DeviceInterface::setupDatabase()
{
    KDbDriverManager manager;
    const QStringList driverIds = manager.driverIds();
    qDebug() << "DRIVERS: " << driverIds;
    if (driverIds.isEmpty()) {
        qWarning() << "No drivers found";
        return;
    }
    if (manager.result().isError()) {
        qDebug() << manager.result();
        return;
    }

    //get driver
    m_dbDriver = manager.driver("org.kde.kdb.sqlite");
    if (!m_dbDriver || manager.result().isError()) {
        qDebug() << manager.result();
        return;
    }

    m_connData.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+ "/amazfish.kexi");

    qDebug() << "Database is: " << m_connData.databaseName();

    m_conn = m_dbDriver->createConnection(m_connData);

    if (!m_conn || m_dbDriver->result().isError()) {
        qDebug() << m_dbDriver->result();
        return;
    }
    qDebug() << "KDbConnection object created.";
    if (!m_conn->connect()) {
        qDebug() << m_conn->result();
        return;
    }
    qDebug() << "KDbConnection::connect() OK.";

    if (!m_conn->databaseExists(m_connData.databaseName())) {
        if (!m_conn->createDatabase(m_connData.databaseName())) {
            qDebug() << m_conn->result();
            return;
        }
        qDebug() << "DB" << m_connData.databaseName() << "created";
    }

    if (!m_conn->useDatabase()) {
        qDebug() << m_conn->result();
        return;
    }

    createTables();

}

KDbConnection *DeviceInterface::dbConnection()
{
    if (m_conn && m_conn->isDatabaseUsed()) {
        return m_conn;
    }
    return nullptr;
}

void DeviceInterface::onConnectionStateChanged()
{
    qDebug() << "DeviceInterface::onConnectionStateChanged" << connectionState();

    if (connectionState() == "authenticated") {
        if (miBandService()) {
            miBandService()->setDatabase(dbConnection());
            m_dbusHRM->setMiBandService(miBandService());
        }
        if (hrmService()) {
            m_dbusHRM->setHRMService(hrmService());
        }
        if (AmazfishConfig::instance()->appNotifyConnect() && m_notificationBuffer.isEmpty()) {
            sendAlert(tr("Amazfish"), tr("Connected"), tr("Phone and watch are connected"), true);
        }

        sendBufferedNotifications();
        updateCalendar();
    } else {
        //Terminate running operations
        m_device->abortOperations();
    }
    emit connectionStateChanged();
}

void DeviceInterface::slot_informationChanged(AbstractDevice::Info key, const QString &val)
{
    qDebug() << Q_FUNC_INFO << key << val;

    //Handle notification of low battery
    if (key == AbstractDevice::INFO_BATTERY) {
        if (val.toInt() != m_lastBatteryLevel) {
            if (val.toInt() <= 10 && val.toInt() < m_lastBatteryLevel && AmazfishConfig::instance()->appNotifyLowBattery()) {
                sendAlert("Amazfish", tr("Low Battery"), tr("Battery level now ") + QString::number(m_lastBatteryLevel) + "%");
            }
            m_lastBatteryLevel = val.toInt();
        }
    }

    emit informationChanged(key, val);
}

void DeviceInterface::musicChanged()
{
    qDebug() << Q_FUNC_INFO;
    if (m_device) {
        m_device->setMusicStatus(m_musicController.status() == watchfish::MusicController::StatusPlaying, m_musicController.title(), m_musicController.artist(), m_musicController.album());
    }
}

void DeviceInterface::deviceEvent(AbstractDevice::Events event)
{
    qDebug() << Q_FUNC_INFO << event;
    switch(event) {
    case AbstractDevice::EVENT_MUSIC_STOP:
        m_musicController.pause();
        break;
    case AbstractDevice::EVENT_MUSIC_PLAY:
        m_musicController.play();
        break;
    case AbstractDevice::EVENT_MUSIC_PAUSE:
        m_musicController.pause();
        break;
    case AbstractDevice::EVENT_MUSIC_NEXT:
        m_musicController.next();
        break;
    case AbstractDevice::EVENT_MUSIC_PREV:
        m_musicController.previous();
        break;
    case AbstractDevice::EVENT_MUSIC_VOLUP:
        m_musicController.volumeUp();
        break;
    case AbstractDevice::EVENT_MUSIC_VOLDOWN:
        m_musicController.volumeDown();
        break;
    case AbstractDevice::EVENT_APP_MUSIC:
        musicChanged();
        break;
#ifdef MER_EDITION_SAILFISH
    case AbstractDevice::EVENT_IGNORE_CALL:
        m_voiceCallController.silence();
        break;
    case AbstractDevice::EVENT_DECLINE_CALL:
        m_voiceCallController.hangup();
        break;
#endif
    }
}

void DeviceInterface::handleButtonPressed(int presses)
{
    if (presses == 2) {
        m_musicController.next();
    } else if (presses == 3) {
        m_musicController.previous();
    }
    emit buttonPressed(presses);
}

void DeviceInterface::sendBufferedNotifications()
{
    qDebug() << Q_FUNC_INFO;
    while (m_notificationBuffer.count() > 0) {
        watchfish::Notification *n = m_notificationBuffer.dequeue();
        if (m_device->supportsFeature(AbstractDevice::FEATURE_ALERT)){
            qDebug() << "Sending notification";
            sendAlert(n->appName(), n->summary(), n->body());
        }
    }
}

QString DeviceInterface::prepareFirmwareDownload(const QString &path)
{
    if (!operationRunning()) {
        if (m_firmwareInfo) {
            delete m_firmwareInfo;
        }
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return QString();
        }
        m_firmwareInfo = m_device->firmwareInfo(file.readAll());
        if (m_firmwareInfo->type() != AbstractFirmwareInfo::Invalid) {
            m_device->prepareFirmwareDownload(m_firmwareInfo);
            return m_firmwareInfo->version();
        }
    }
    return QString();
}

bool DeviceInterface::startDownload()
{
    qDebug() << Q_FUNC_INFO;
    auto config = AmazfishConfig::instance();

    if (m_firmwareInfo->supportedOnDevice(m_device->deviceName()) || config->appOverrideFwCheck()) {
        m_device->startDownload();
        return true;
    }
    return false;
}

bool DeviceInterface::operationRunning()
{
    if (m_device) {
        return m_device->operationRunning();
    }
    return false;
}

void DeviceInterface::downloadSportsData()
{
    if (miBandService()) {
        miBandService()->fetchSportsSummaries();
    }
}

void DeviceInterface::downloadActivityData()
{
    if (miBandService()) {
        miBandService()->fetchActivityData();
    }
}

void DeviceInterface::sendWeather(CurrentWeather *weather)
{
    if (m_device) {
        m_device->sendWeather(weather);
    }
}

void DeviceInterface::onCitiesChanged()
{
    if (m_cityManager.cities().count() > 0) {
        m_currentWeather.setCity(qobject_cast<City*>(m_cityManager.cities()[0]));
    }
}

void DeviceInterface::onWeatherReady()
{
    sendWeather(&m_currentWeather);
}

void DeviceInterface::navigationRunningChanged(bool running)
{
    qDebug() << Q_FUNC_INFO << running;
    if (m_device && AmazfishConfig::instance()->appNavigationNotification()) {
        m_device->navigationRunning(running);
    }
}

void DeviceInterface::navigationChanged(const QString &icon, const QString &narrative, const QString &manDist, int progress)
{
    qDebug() << Q_FUNC_INFO << icon << narrative;
    if (m_device && AmazfishConfig::instance()->appNavigationNotification()) {
        m_device->navigationNarrative(icon, narrative, manDist, progress);
    }
}

void DeviceInterface::refreshInformation()
{
    qDebug() << "Refreshing device information";
    if (m_device) {
        return m_device->refreshInformation();
    }
}

QString DeviceInterface::information(int i)
{
    if (m_device) {
        return m_device->information((AbstractDevice::Info)i);
    }
    return QString();
}

void DeviceInterface::sendAlert(const QString &sender, const QString &subject, const QString &message, bool allowDuplicate)
{
    qDebug() << Q_FUNC_INFO;

    int hash = qHash(sender + subject + message);
    if (hash == m_lastAlertHash && !allowDuplicate) {
        qDebug() << "Discarded duplicate alert";
        return; //Do not send duplicate alerts
    }
    m_lastAlertHash = hash;

    if (m_device && m_device->connectionState() == "authenticated" && m_device->supportsFeature(AbstractDevice::FEATURE_ALERT)){
        qDebug() << "Sending alert to device";
        m_device->sendAlert(sender, subject, message);
    }
}

void DeviceInterface::incomingCall(const QString &caller)
{
    if (m_device) {
        m_device->incomingCall(caller);
    }
}

void DeviceInterface::applyDeviceSetting(int s)
{
    qDebug() << "Apply setting:" << s << (int)s;
    if (m_device) {
        m_device->applyDeviceSetting((AbstractDevice::Settings)s);
    }
}

void DeviceInterface::requestManualHeartrate()
{
    if (hrmService()) {
        hrmService()->enableManualHRMeasurement(true);
    }
}

void DeviceInterface::onRefreshTimer()
{
    qDebug() << "DeviceInterface::onRefreshTimer";
    static int syncActivitiesMinutes = 0;
    static int syncWeatherMinutes = 0;
    static int syncCalendarMinutes = 0;

    auto config = AmazfishConfig::instance();

    syncWeatherMinutes++;
    if (syncWeatherMinutes >= config->appRefreshWeather()) {
        syncWeatherMinutes = 0;
        qDebug() << "weather interval reached";
        m_currentWeather.refresh();
    }
    syncCalendarMinutes++;
    if (syncCalendarMinutes >= config->appRefreshCalendar()) {
        syncCalendarMinutes = 0;
        qDebug() << "calendar interval reached";
        updateCalendar();
    }

    if (config->appAutoSyncData()) {
        syncActivitiesMinutes++;

        if (syncActivitiesMinutes > 60) {
            qDebug() << "Auto syncing activity data";
            syncActivitiesMinutes = 0;
            downloadActivityData();
        }
    }
}

void DeviceInterface::registerDBus()
{
    if (!m_dbusRegistered)
    {
        // DBus
        QDBusConnection connection = QDBusConnection::sessionBus();
        qDebug() << "Registering service on dbus" << SERVICE;
        if (!connection.registerService(SERVICE))
        {
            QCoreApplication::quit();
            return;
        }

        if (!connection.registerObject(PATH, this, QDBusConnection::ExportAllInvokables | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties))
        {
            QCoreApplication::quit();
            return;
        }
        m_dbusRegistered = true;

        qDebug() << "amazfish-daemon: succesfully registered to dbus sessionBus";
    }
}

void DeviceInterface::triggerSendWeather()
{
    m_currentWeather.refresh();
}

void DeviceInterface::updateCalendar()
{
    if (supportsFeature(AbstractDevice::FEATURE_EVENT_REMINDER)) {
        if (m_device) {
            QList<watchfish::CalendarEvent> eventlist = m_calendarSource.fetchEvents(QDate::currentDate(), QDate::currentDate().addDays(14), true);

            int id=0;
            foreach (watchfish::CalendarEvent event, eventlist) {
                qDebug() << event.uid() << event.title() << event.start();
                m_device->sendEventReminder(id, event.start(), event.title());
                id++;
            }
        }
    }
}

void DeviceInterface::reloadCities()
{
    m_cityManager.loadCities();
    if (m_cityManager.cities().count() > 0) {
        m_currentWeather.setCity(qobject_cast<City*>(m_cityManager.cities()[0]));
    }
}

void DeviceInterface::enableFeature(int feature)
{
    qDebug() << Q_FUNC_INFO << feature;
    if (m_device) {
        m_device->enableFeature(AbstractDevice::Feature(feature));
    }
}

QStringList DeviceInterface::supportedDisplayItems()
{
    qDebug() << Q_FUNC_INFO;
    if (m_device) {
        return m_device->supportedDisplayItems();
    }
    return QStringList();
}

bool DeviceInterface::supportsFeature(int f){
    return (supportedFeatures() & f) == f;
}

int DeviceInterface::supportedFeatures()
{
    if (m_device) {
        return m_device->supportedFeatures();
    }
    return 0;
}

