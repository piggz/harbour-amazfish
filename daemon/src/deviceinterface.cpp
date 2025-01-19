#include "deviceinterface.h"

#include "deviceinfoservice.h"
#include "alertnotificationservice.h"
#include "hrmservice.h"
#include "bipfirmwareservice.h"
#include "devicefactory.h"
#include "amazfishconfig.h"
#include "transliterator.h"

#include <QDir>
#include <QFile>
#include <QProcess>

#include <KDb3/KDbDriverManager>
#include <KDb3/KDbTransactionGuard>

#if defined(MER_EDITION_SAILFISH) || defined(UUITK_EDITION)
#include <pulse/simple.h>
#include <pulse/error.h>
#endif

static const char *SERVICE = SERVICE_NAME_AMAZFISH;
static const char *PATH = "/application";

DeviceInterface::DeviceInterface()
{
    //Start by registering on DBUS
    registerDBus();

    auto config = AmazfishConfig::instance();

    //Create a device object
    m_device = DeviceFactory::createDevice(config->pairedName());
    if (m_device) {
        connect(m_device, &AbstractDevice::connectionStateChanged, this, &DeviceInterface::onConnectionStateChanged, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::message, this, &DeviceInterface::message, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::downloadProgress, this, &DeviceInterface::downloadProgress, Qt::UniqueConnection);
        connect(m_device, &QBLEDevice::operationRunningChanged, this, &DeviceInterface::operationRunningChanged, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::buttonPressed, this, &DeviceInterface::handleButtonPressed, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::informationChanged, this, &DeviceInterface::slot_informationChanged, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::deviceEvent, this, &DeviceInterface::deviceEvent, Qt::UniqueConnection);
    }

    //Create the DBUS HRM Interface
    m_dbusHRM = new DBusHRM(this);

    setupDatabase();

    //Notifications
    connect(&m_notificationMonitor, &watchfish::NotificationMonitor::notification, this, &DeviceInterface::onNotification);

    // Calls
#if defined(MER_EDITION_SAILFISH) || defined(UUITK_EDITION)
    connect(&m_voiceCallController, &watchfish::VoiceCallController::ringingChanged, this, &DeviceInterface::onRingingChanged);
#endif
    //Weather
    connect(&m_cityManager, &CityManager::citiesChanged, this, &DeviceInterface::onCitiesChanged);
    connect(&m_currentWeather, &CurrentWeather::ready, this, &DeviceInterface::onWeatherReady);
    if (m_cityManager.cities().count() > 0) {
        m_currentWeather.setCity(qobject_cast<City*>(m_cityManager.cities().at(0)));
    }

    //Refresh timer
    m_refreshTimer = new QTimer();
    connect(m_refreshTimer, &QTimer::timeout, this, &DeviceInterface::onRefreshTimer);
    m_refreshTimer->start(60000);
    m_lastWeatherSync = m_lastCalendarSync = m_lastActivitySync = QDateTime::currentDateTime();

    //Find device playback timer
    m_findDeviceTimer = new QTimer();
    connect(m_findDeviceTimer, &QTimer::timeout, this, &DeviceInterface::findDevice);

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
    qDebug() << Q_FUNC_INFO << ": address:" << address;


    if (m_device) {
        m_deviceAddress = address;
        m_device->setDevicePath(address);
        m_device->connectToDevice();
    }
    else {
        qDebug() << Q_FUNC_INFO << ": device was not valid";
        message(tr("Device is not valid, it may not be supported"));
    }
}

QString DeviceInterface::pair(const QString &name, const QString &address)
{
    qDebug() << Q_FUNC_INFO << name << address;

    m_deviceAddress = address;

    if (m_device) {
        delete m_device;
    }
    m_device = DeviceFactory::createDevice(name);

    if (m_device) {
        m_device->setDevicePath(address);
        connect(m_device, &AbstractDevice::connectionStateChanged, this, &DeviceInterface::onConnectionStateChanged, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::message, this, &DeviceInterface::message, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::downloadProgress, this, &DeviceInterface::downloadProgress, Qt::UniqueConnection);
        connect(m_device, &QBLEDevice::operationRunningChanged, this, &DeviceInterface::operationRunningChanged, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::buttonPressed, this, &DeviceInterface::handleButtonPressed, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::informationChanged, this, &DeviceInterface::slot_informationChanged, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::deviceEvent, this, &DeviceInterface::deviceEvent, Qt::UniqueConnection);
        m_device->pair();
        return "pairing";
    }
    
    qDebug() << Q_FUNC_INFO << ": device not created";

    return QString("no device found");
}

void DeviceInterface::disconnect()
{
    qDebug() << Q_FUNC_INFO;
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

HRMService *DeviceInterface::hrmService() const
{
    return qobject_cast<HRMService*>(m_device->service(HRMService::UUID_SERVICE_HRM));
}

void DeviceInterface::onNotification(watchfish::Notification *notification)
{
    if (m_device && m_device->connectionState() == "authenticated" && m_device->supportsFeature(AbstractDevice::FEATURE_ALERT)){
        qDebug() << Q_FUNC_INFO << "Sending alert to device";
        sendAlert(notification->appName(), notification->summary(), notification->body());
    } else {
        qDebug() << Q_FUNC_INFO << "no notification service, buffering notification";

        WatchNotification n;
        n.id = notification->id();
        n.appName = notification->appName();
        n.summary = notification->summary();
        n.body = notification->body();

        m_notificationBuffer.enqueue(n);
        if (m_notificationBuffer.count() > 10) {
            m_notificationBuffer.dequeue();
        }
    }
}

void DeviceInterface::onRingingChanged()
{
#if defined(MER_EDITION_SAILFISH) || defined(UUITK_EDITION)
    qDebug() << Q_FUNC_INFO << m_voiceCallController.ringing();

    if (!m_device) {
        return;
    }

    if (m_voiceCallController.ringing()) {

        QString caller = m_voiceCallController.findPersonByNumber(m_voiceCallController.callerId());

        if (AmazfishConfig::instance()->appTransliterate()) {
            m_device->incomingCall( Transliterator::convert( caller ) );
        } else {
            m_device->incomingCall(caller);
        }

    } else {
        m_device->incomingCallEnded();
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


    if (!m_conn->containsTable("info_log")) {
        KDbTableSchema *t_info = new KDbTableSchema("info_log");
        t_info->setCaption("Info log");
        t_info->addField(f = new KDbField("id", KDbField::Integer, KDbField::PrimaryKey | KDbField::AutoInc, KDbField::Unsigned));
        f->setCaption("ID");
        t_info->addField(f = new KDbField("timestamp", KDbField::Integer, nullptr));
        f->setCaption("Timestamp");
        t_info->addField(f = new KDbField("timestamp_dt", KDbField::DateTime));
        f->setCaption("Timestamp in Date/Time format");
        t_info->addField(f = new KDbField("key", KDbField::Integer, nullptr, KDbField::Unsigned));
        f->setCaption("Key based on AbstractDevice::Info");
        t_info->addField(f = new KDbField("value", KDbField::Integer, nullptr, KDbField::Unsigned));
        f->setCaption("Value");

        if (!m_conn->createTable(t_info)) {
            qDebug() << m_conn->result();
            return;
        }
        qDebug() << "-- info_log created --";
        qDebug() << *t_info;
    }

    int batteryLevel = 0;

    if (m_conn->querySingleNumber(
                KDbEscapedString("SELECT value FROM info_log WHERE key = %1 ORDER BY id DESC").arg(AbstractDevice::INFO_BATTERY), // automatically adds LIMIT 1 into query
                &batteryLevel) == true) { // comparision of tristate type (true, false, canceled)
        m_lastBatteryLevel = batteryLevel;
        qDebug() << Q_FUNC_INFO << "Last Battery Level: " << m_lastBatteryLevel;
    } else {
        qWarning() << "Cannot get battery level";
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
    qDebug() << Q_FUNC_INFO << "DRIVERS: " << driverIds;
    if (driverIds.isEmpty()) {
        qWarning() << Q_FUNC_INFO << ": No drivers found";
        return;
    }
    if (manager.result().isError()) {
        qDebug() << Q_FUNC_INFO << manager.result();
        return;
    }

    //get driver
    m_dbDriver = manager.driver("org.kde.kdb.sqlite");
    if (!m_dbDriver || manager.result().isError()) {
        qDebug() << Q_FUNC_INFO << manager.result();
        return;
    }

    m_connData.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+ "/amazfish.kexi");

    qDebug() << Q_FUNC_INFO << ": Database is: " << m_connData.databaseName();

    m_conn = m_dbDriver->createConnection(m_connData);

    if (!m_conn || m_dbDriver->result().isError()) {
        qDebug() << Q_FUNC_INFO << ": Could not create connection:" << m_dbDriver->result();
        return;
    }
    qDebug() << Q_FUNC_INFO << ": KDbConnection object created.";
    if (!m_conn->connect()) {
        qDebug() << Q_FUNC_INFO << ": Could not connect: " << m_conn->result();
        return;
    }
    qDebug() << Q_FUNC_INFO << ": KDbConnection::connect() OK.";

    if (!m_conn->databaseExists(m_connData.databaseName())) {
        if (!m_conn->createDatabase(m_connData.databaseName())) {
            qDebug() << Q_FUNC_INFO << ": Could not create Database: " << m_conn->result();
            return;
        }
        qDebug() << Q_FUNC_INFO << ": Database" << m_connData.databaseName() << "created";
    }

    if (!m_conn->useDatabase()) {
        qDebug() << Q_FUNC_INFO << ": Could not use Database: " << m_conn->result();
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
    qDebug() << Q_FUNC_INFO << connectionState();

    if (connectionState() == "authenticated") {
        m_device->setDatabase(dbConnection());
        if (m_device) {
            m_dbusHRM->setDevice(m_device);
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

void DeviceInterface::log_battery_level(int level) {

    if (!m_conn || !(m_conn->isDatabaseUsed())) {
        qDebug() << Q_FUNC_INFO  << "Database not connected";
        return;
    }

    QDateTime m_sampleTime = QDateTime::currentDateTime();
    qDebug() << Q_FUNC_INFO << "Start time" << m_sampleTime;

    KDbTransaction transaction = m_conn->beginTransaction();
    KDbTransactionGuard tg(transaction);

    KDbFieldList fields;
    auto s_battery = m_conn->tableSchema("info_log");

    fields.addField(s_battery->field("timestamp"));
    fields.addField(s_battery->field("timestamp_dt"));
    fields.addField(s_battery->field("key"));
    fields.addField(s_battery->field("value"));

    QList<QVariant> values;

    values << m_sampleTime.toMSecsSinceEpoch() / 1000;
    values << m_sampleTime;
    values << AbstractDevice::INFO_BATTERY;
    values << level;

    if (!m_conn->insertRecord(&fields, values)) {
        qDebug() << Q_FUNC_INFO << "error inserting record";
        return;
    }
    tg.commit();

}

void DeviceInterface::slot_informationChanged(AbstractDevice::Info key, const QString &val)
{
    qDebug() << Q_FUNC_INFO << key << val;

    //Handle notification of low battery
    if (key == AbstractDevice::INFO_BATTERY) {
        int battery_level = val.toInt();
        if (battery_level != m_lastBatteryLevel) {

            log_battery_level(battery_level);

            if (battery_level <= 10 && battery_level < m_lastBatteryLevel && AmazfishConfig::instance()->appNotifyLowBattery()) {
                sendAlert("Amazfish", tr("Low Battery"), tr("Battery level now ") + QString::number(battery_level) + "%");
            }
            m_lastBatteryLevel = battery_level;
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

void DeviceInterface::deviceEvent(AbstractDevice::Event event)
{
    qDebug() << Q_FUNC_INFO << event;
    emit deviceEventTriggered(QMetaEnum::fromType<AbstractDevice::Event>().valueToKey(event));
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
#if defined(MER_EDITION_SAILFISH) || defined(UUITK_EDITION)
    case AbstractDevice::EVENT_IGNORE_CALL:
        m_voiceCallController.silence();
        break;
    case AbstractDevice::EVENT_DECLINE_CALL:
        m_voiceCallController.hangup();
        break;
    case AbstractDevice::EVENT_ANSWER_CALL:
        m_voiceCallController.answer();
        break;
#endif
    case AbstractDevice::EVENT_FIND_PHONE:
        m_findDeviceTimer->start(5000);
        break;
    case AbstractDevice::EVENT_CANCEL_FIND_PHONE:
        m_findDeviceTimer->stop();
        break;
    }
}

void DeviceInterface::handleButtonPressed(int presses)
{
    qDebug() << Q_FUNC_INFO << presses;

    if (m_device && m_device->supportsFeature(AbstractDevice::FEATURE_BUTTON_ACTION)) {
        QString action = "action-none";

        if (presses == 2) {
            action = AmazfishConfig::instance()->appButtonDoublePressAction();
        } else if (presses == 3) {
            action = AmazfishConfig::instance()->appButtonTriplePressAction();
        } else if (presses == 4) {
            action = AmazfishConfig::instance()->appButtonQuadPressAction();
        }

        qDebug() << Q_FUNC_INFO << "Action:" << action;

        if (action == "action-music-next"){
            m_musicController.next();
        } else if (action == "action-music-prev"){
            m_musicController.previous();
        } else if (action == "action-vol-up"){
            m_musicController.volumeUp();
        } else if (action == "action-vol-down"){
            m_musicController.volumeDown();
        } else if (action == "action-custom"){
            //Run custom script in ~/harbour-amazfish-scirpt.sh and pass in press count
            QString fileName = QDir::homePath() + "/harbour-amazfish-script.sh";
            if (QFile::exists(fileName)) {
                if (!QProcess::startDetached("/bin/sh", QStringList{fileName,QString::number(presses)})) {
                    qDebug() << "Failed to run" << fileName;
                }
            } else {
                qDebug() << fileName <<  "does not exist";
            }
        }
        emit buttonPressed(presses);
    }
}

void DeviceInterface::onEventTimer()
{
    qDebug() << Q_FUNC_INFO;
    if (m_eventlist.isEmpty())
        return;
    watchfish::CalendarEvent event = m_eventlist.takeFirst();
    sendAlert("calendar", event.title(), event.description().isEmpty()?" ":event.description());
    scheduleNextEvent();
}

void DeviceInterface::backgroundActivityStateChanged()
{
#ifdef MER_EDITION_SAILFISH
    if (m_backgroundActivity->isRunning())
        onEventTimer();
#endif
}

void DeviceInterface::sendBufferedNotifications()
{
    qDebug() << Q_FUNC_INFO;
    while (m_notificationBuffer.count() > 0) {
        WatchNotification n = m_notificationBuffer.dequeue();
        if (m_device->supportsFeature(AbstractDevice::FEATURE_ALERT)){
            qDebug() << "Sending notification";
            sendAlert(n.appName, n.summary, n.body);
        }
    }
}

void DeviceInterface::scheduleNextEvent()
{
    qDebug() << Q_FUNC_INFO;
    if (m_eventlist.isEmpty())
        return;
#ifdef MER_EDITION_SAILFISH
    if (!m_backgroundActivity) {
        m_backgroundActivity = new BackgroundActivity(this);
        connect(m_backgroundActivity, &BackgroundActivity::stateChanged,
                this, &DeviceInterface::backgroundActivityStateChanged);
    }
#endif

    watchfish::CalendarEvent nextEvent = m_eventlist.first();
    if (!nextEvent.alertTime().isValid())
        return;
    int secsToNextEvent = QDateTime::currentDateTime().secsTo(nextEvent.alertTime());
    while (secsToNextEvent < 30) {
        m_eventlist.takeFirst();
        if (m_eventlist.isEmpty())
            return;
        nextEvent = m_eventlist.first();
        if (!nextEvent.alertTime().isValid())
            return;
        secsToNextEvent = QDateTime::currentDateTime().secsTo(nextEvent.alertTime());
    }
    qDebug() << "seconds until next event: " << secsToNextEvent;
    qDebug() << "event title: " << nextEvent.title();
#ifdef MER_EDITION_SAILFISH
    m_backgroundActivity->setWakeupRange(secsToNextEvent-30, secsToNextEvent+30);
    m_backgroundActivity->wait();
#endif
}

void DeviceInterface::findDevice()
{
    qDebug() << Q_FUNC_INFO;

    m_playedSounds++;

#if defined(MER_EDITION_SAILFISH) || defined(UUITK_EDITION)

    /* The Sample format to use */
    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 2
    };

    pa_simple *s = NULL;
    int error;


#ifdef MER_EDITION_SAILFISH
    QFile file("/usr/share/harbour-amazfishd/chirp.raw");
#else // elif defined(UUITK_EDITION)
    QFile file("/opt/click.ubuntu.com/uk.co.piggz.amazfish/current/share/harbour-amazfishd/chirp.raw");
#endif

    if(!file.open(QIODevice::ReadOnly))
    {
        qWarning() << Q_FUNC_INFO << "Unable to open chirp sound";
        return;
    }

    QByteArray bytes = file.readAll();
    file.close();

    if (!(s = pa_simple_new(NULL, "amazfish-daemon", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
        qDebug() << ": pa_simple_new() failed: " << pa_strerror(error);
        return;
    }

    /* ... and play it */
    if (pa_simple_write(s, bytes.data(), (size_t) bytes.size(), &error) < 0) {
        qDebug() << ": pa_simple_write() failed: " << pa_strerror(error);
        return;
    }

    /* Make sure that every single sample was played */
    if (pa_simple_drain(s, &error) < 0) {
        qDebug() << ": pa_simple_drain() failed: " << pa_strerror(error);
    }

#endif

    if (m_playedSounds >= 10) {
        m_findDeviceTimer->stop();
        m_playedSounds = 0;
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
        if (m_firmwareInfo == nullptr) {
            qWarning() << "m_firmwareInfo is NULL";
            return QString();
        }
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

    if (m_firmwareInfo == nullptr) {
        qWarning() << "m_firmwareInfo is NULL";
        return false;
    }

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
    if (m_device) {
        m_device->downloadSportsData();
    }
}

void DeviceInterface::downloadActivityData()
{
    if (m_device) {
        m_device->downloadActivityData();
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
        m_currentWeather.setCity(qobject_cast<City*>(m_cityManager.cities().at(0)));
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
    qDebug() << Q_FUNC_INFO << "Refreshing device information";

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
        qDebug() << "Snding alert to device";

        if (AmazfishConfig::instance()->appTransliterate()) {
            m_device->sendAlert(
                        Transliterator::convert(sender),
                        Transliterator::convert(subject),
                        Transliterator::convert(message)
                        );
        } else {
            m_device->sendAlert(sender, subject, message);
        }
    }
}

void DeviceInterface::incomingCall(const QString &caller)
{
    if (m_device) {
        if (AmazfishConfig::instance()->appTransliterate()) {
            m_device->incomingCall( Transliterator::convert( caller ) );
        } else {
            m_device->incomingCall(caller);
        }
    }
}

void DeviceInterface::incomingCallEnded() {
    if (m_device) {
        m_device->incomingCallEnded();
    }
}

void DeviceInterface::applyDeviceSetting(int s)
{
    qDebug() << Q_FUNC_INFO << "Setting:" << s << (int)s;

    if (m_device) {
        m_device->applyDeviceSetting((AbstractDevice::Settings)s);
    }
}

void DeviceInterface::requestManualHeartrate()
{
    qDebug() << Q_FUNC_INFO;

    if (hrmService()) {
        hrmService()->enableManualHRMeasurement(true);
    }
}

void DeviceInterface::onRefreshTimer()
{
    //qDebug() << Q_FUNC_INFO;

    auto config = AmazfishConfig::instance();

    QDateTime currentDate = QDateTime::currentDateTime();

    if (m_lastWeatherSync.secsTo(currentDate) >= (config->appRefreshWeather() * 60)) {
        qDebug() << Q_FUNC_INFO << "weather interval reached";
        m_lastWeatherSync = currentDate;
        m_currentWeather.refresh();
    }

    if (m_lastCalendarSync.secsTo(currentDate) >= (config->appRefreshCalendar() * 60)) {
        qDebug() << Q_FUNC_INFO << "calendar interval reached";
        m_lastCalendarSync = currentDate;
        updateCalendar();
    }

    if (config->appAutoSyncData()) {
        if (m_lastActivitySync.secsTo(currentDate) >= (60*60)) {
            qDebug() << Q_FUNC_INFO << "Auto syncing activity data";
            m_lastActivitySync = currentDate;
            downloadActivityData();
        }
    }
}

void DeviceInterface::registerDBus()
{
    qDebug() << Q_FUNC_INFO;

    if (!m_dbusRegistered)
    {
        // DBus
        QDBusConnection connection = QDBusConnection::sessionBus();
        qDebug() << "Registering service on dbus" << SERVICE;
        if (!connection.registerService(SERVICE))
        {
            qCritical() << Q_FUNC_INFO << "Unable to register service. Quit.";
            exit(1);
            return;
        }

        if (!connection.registerObject(PATH, this, QDBusConnection::ExportAllInvokables | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties))
        {
            qCritical() << Q_FUNC_INFO << "Unable to register objects. Quit.";
            exit(1);
            return;
        }
        m_dbusRegistered = true;

        qInfo() << "Succesfully registered to dbus sessionBus";
    }
}

void DeviceInterface::triggerSendWeather()
{
    m_currentWeather.refresh();
}

void DeviceInterface::updateCalendar()
{
    qDebug() << Q_FUNC_INFO;
    if (supportsFeature(AbstractDevice::FEATURE_EVENT_REMINDER)) {
        if (m_device) {
            QList<watchfish::CalendarEvent> eventlist = m_calendarSource.fetchEvents(QDate::currentDate(), QDate::currentDate().addDays(14), true);

            int id=0;
            foreach (const watchfish::CalendarEvent &event, eventlist) {
                qDebug() << event.uid() << event.title() << event.start();
                m_device->sendEventReminder(id, event.start(), event.title());
                id++;
            }
        }
    } else if (AmazfishConfig::instance()->appSimulateEventSupport()){
        QList<watchfish::CalendarEvent> eventlist = m_calendarSource.fetchEvents(QDate::currentDate(), QDate::currentDate().addDays(14), true);
        QList<watchfish::CalendarEvent> filteredEventList;
        foreach (const watchfish::CalendarEvent &event, eventlist) {
            if (!event.alertTime().isValid())
                continue;
            if (filteredEventList.isEmpty()) {
                filteredEventList.append(event);
            } else {
                auto i = filteredEventList.begin();
                while (i != filteredEventList.end() && i->alertTime() < event.alertTime())
                    i++;
                filteredEventList.insert(i, event);
            }
        }
        foreach (const watchfish::CalendarEvent &event, filteredEventList) {
            qDebug() << event.title() << event.alertTime();
        }
        if (!filteredEventList.isEmpty()) {
            m_eventlist = filteredEventList;
            scheduleNextEvent();
        }
    }
}

void DeviceInterface::reloadCities()
{
    m_cityManager.loadCities();
    if (m_cityManager.cities().count() > 0) {
        m_currentWeather.setCity(qobject_cast<City*>(m_cityManager.cities().at(0)));
    }
}

void DeviceInterface::enableFeature(int feature)
{
    qDebug() << Q_FUNC_INFO << feature;
    if (m_device) {
        m_device->enableFeature(AbstractDevice::Feature(feature));
    }
}

void DeviceInterface::fetchLogs()
{
    if (m_device) {
        m_device->fetchLogs();
    }
}

void DeviceInterface::requestScreenshot() {
    if (m_device) {
        m_device->requestScreenshot();
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

void DeviceInterface::immediateAlert(int level)
{
    if (m_device) {
        m_device->immediateAlert(level);
    }
}
