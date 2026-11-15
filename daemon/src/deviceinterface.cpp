#include "deviceinterface.h"
#include "bluezadapter.h"
#include "adaptermodel.h"
#include "qbleagent.h"
#include "deviceinfoservice.h"
#include "alertnotificationservice.h"
#include "hrmservice.h"
#include "devicefactory.h"
#include "amazfishconfig.h"
#include "transliterator.h"

#include <QDir>
#include <QFile>
#include <QProcess>

#include "database.h"

#if defined(MER_EDITION_SAILFISH) || defined(UUITK_EDITION)
#include <pulse/simple.h>
#include <pulse/error.h>
#endif

#ifdef UUITK_EDITION
#include <libusermetricsinput/MetricManager.h>
using namespace UserMetricsInput;
#endif


static const char *SERVICE = SERVICE_NAME_AMAZFISH;
static const char *PATH = "/application";

DeviceInterface::DeviceInterface()
{
    // Create the database before registering on D-Bus: the UI opens the database when the daemon appears.
    setupDatabase();

    //Start by registering on DBUS
    registerDBus();
    initAgent();

    auto config = AmazfishConfig::instance();

    //Create a device object
    m_device = DeviceFactory::createDevice(config->pairedName(), config->pairedType());
    if (m_device) {
        connect(m_device, &AbstractDevice::connectionStateChanged, this, &DeviceInterface::onConnectionStateChanged, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::message, this, &DeviceInterface::message, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::downloadProgress, this, &DeviceInterface::downloadProgress, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::operationRunningChanged, this, &DeviceInterface::operationRunningChanged, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::buttonPressed, this, &DeviceInterface::handleButtonPressed, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::informationChanged, this, &DeviceInterface::slot_informationChanged, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::deviceEvent, this, &DeviceInterface::deviceEvent, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::weatherRequested, this, &DeviceInterface::onWeatherRequested, Qt::UniqueConnection);
    }

    //Create the DBUS HRM Interface
    m_dbusHRM = new DBusHRM(this);

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

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(60000);
    connect(m_reconnectTimer, &QTimer::timeout, this, &DeviceInterface::reconnectionTimer);
    m_reconnectTimer->start(); // Start timer to attempt to reconnect every 60 seconds

    reconnectionTimer();
}

DeviceInterface::~DeviceInterface()
{
    disconnect();
}

void DeviceInterface::initAgent() {

    if (m_agent != nullptr) {
	return;
    }
    m_agent = new QBLEAgent(this);
    connect(m_agent, &QBLEAgent::UIRequestPasskey, this, &DeviceInterface::requestPasskeyFromUI, Qt::UniqueConnection);
    connect(m_agent, &QBLEAgent::UIRequestPinCode, this, &DeviceInterface::requestPinCodeFromUI, Qt::UniqueConnection);
    connect(m_agent, &QBLEAgent::UIRequestConfirmation, this, &DeviceInterface::requestConfirmationFromUI, Qt::UniqueConnection);
    connect(m_agent, &QBLEAgent::UIDisplayPasskey, this, &DeviceInterface::displayPasskeyInUI, Qt::UniqueConnection);

    connect(m_agent, &QBLEAgent::pairingAccepted,
	    this, [this](const QString &devicePath) {
		qDebug() << Q_FUNC_INFO  << "Pairing accepted" << devicePath;
		m_device->trust(true);
	    });

    m_agent->registerAgent();
}

void DeviceInterface::connectToDevice(const QString &address)
{
    qDebug() << Q_FUNC_INFO << ": address:" << address;

    m_deviceAddress = devicePath(address);

    if (m_device && !m_deviceAddress.isEmpty()) {
        m_device->setDevicePath(m_deviceAddress);
        m_device->connectToDevice();
    } else {
        qDebug() << Q_FUNC_INFO << ": device was not valid";
        if (m_allowDeviceNotAvailableMessage) {
            message(tr("Device is not yet available"));
            m_allowDeviceNotAvailableMessage = false;
        }
    }
}

void DeviceInterface::connectToDevice(bool userInitiated)
{
    qDebug() << Q_FUNC_INFO;

    auto config = AmazfishConfig::instance();
    QString pairedAddress = config->pairedAddress();

    m_allowDeviceNotAvailableMessage = userInitiated;

    // Convert old format address to new
    if (pairedAddress.contains("/org/bluez/hci")) {

        QString newAddress = pairedAddress.right(17).replace("_", ":");
        // Migrate data to the new address format
        if (Database::changeDeviceId(pairedAddress, newAddress)) {
            config->setPairedAddress(newAddress);
        }
        //If migration fails, dont save the new address but connect anyway
        pairedAddress = newAddress;
    }

    if (!pairedAddress.isEmpty()) {
        // Connect was called from UI so enable auto reconnect
        m_autoreconnect = true;
        connectToDevice(pairedAddress);
    }
}

void DeviceInterface::reconnectionTimer()
{
    qDebug() << Q_FUNC_INFO;

    const QString state = connectionState();
    if (state == "pairing" || state == "connecting" || state == "connected" || state == "paired") {
        return;
    }

    if ((state != "authenticated" && m_autoreconnect) || state == "authfailed") {
        qDebug() << Q_FUNC_INFO << "Lost connection";
        disconnect();
        connectToDevice(false);
    }
}

QString DeviceInterface::pair(const QString &name, const QString &deviceType, const QString &address)
{
    qDebug() << Q_FUNC_INFO << name << deviceType << address;

    m_deviceAddress = devicePath(address);

    if (m_deviceAddress.isEmpty()) {
        qDebug() << "Device is not available";
        message(tr("Device is not yet available"));
        return QString("device not available");
    }

    if (m_device) {
        delete m_device;
    }
    m_device = DeviceFactory::createDevice(name, deviceType);

    if (m_device) {
        m_device->setDevicePath(m_deviceAddress);
        connect(m_device, &AbstractDevice::connectionStateChanged, this, &DeviceInterface::onConnectionStateChanged, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::message, this, &DeviceInterface::message, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::downloadProgress, this, &DeviceInterface::downloadProgress, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::operationRunningChanged, this, &DeviceInterface::operationRunningChanged, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::buttonPressed, this, &DeviceInterface::handleButtonPressed, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::informationChanged, this, &DeviceInterface::slot_informationChanged, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::deviceEvent, this, &DeviceInterface::deviceEvent, Qt::UniqueConnection);
        connect(m_device, &AbstractDevice::weatherRequested, this, &DeviceInterface::onWeatherRequested, Qt::UniqueConnection);
        m_device->pair();
        m_autoreconnect = true;
        return "pairing";
    }

    qDebug() << Q_FUNC_INFO << ": device not created";

    return QString("no device found");
}

void DeviceInterface::disconnect()
{
    qDebug() << Q_FUNC_INFO;
    m_autoreconnect = false;

    if (m_device) {
        m_device->disconnectFromDevice();
    }
}

void DeviceInterface::unpair()
{
    qDebug() << Q_FUNC_INFO;
    if (m_device && !m_adapterPath.isEmpty()) {
        BluezAdapter adapter;
        adapter.setAdapterPath(m_adapterPath);
        adapter.removeDevice(m_deviceAddress);
        delete m_device;
        m_device = nullptr;

        m_autoreconnect = false;
        m_adapterPath = QString();
    }
}

QString DeviceInterface::connectionState() const
{
    if (!m_device) {
        return QString();
    }
    return m_device->connectionState();
}
int DeviceInterface::connectionStateChangedCount() const
{
    return m_connectionStateChangedCount;
}

HRMService *DeviceInterface::hrmService() const
{
    return qobject_cast<HRMService*>(m_device->service(HRMService::UUID_SERVICE_HRM));
}

void DeviceInterface::onNotification(watchfish::Notification *notification)
{
    Amazfish::WatchNotification n;
    n.id = notification->id();
    n.appId = notification->appId();
    n.appName = notification->appName();
    n.summary = notification->summary();
    n.body = notification->body();

    // Our own popups, like "Downloading activity data", are not useful on the watch.
    if (n.appName == "harbour-amazfish") {
        return;
    }

    QVariantMap notificationSettings = AmazfishConfig::instance()->notificationSettings();

    bool enabled = true;
    QString app_slug = n.appId.isEmpty() ? n.appName : n.appId;
    if (notificationSettings.contains(app_slug)) {
        enabled = notificationSettings.value(app_slug).toBool();
    } else {
        notificationSettings.insert(app_slug, enabled);
    }
    AmazfishConfig::instance()->setNotificationSettings(notificationSettings);

    if (!enabled) {
        qDebug() << Q_FUNC_INFO << "Notification " << app_slug << " disabled in settings";
        return;
    }
    if (m_device && m_device->connectionState() == "authenticated" && m_device->supportsFeature(Amazfish::Feature::FEATURE_ALERT)){
        qDebug() << Q_FUNC_INFO << "Sending alert to device";
        sendAlert(n);
    } else {
        qDebug() << Q_FUNC_INFO << "no notification service, buffering notification";

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

void DeviceInterface::setupDatabase()
{
    QString directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!Database::open(directory + "/amazfish.kexi")) {
        return;
    }

    QVariant batteryLevel = Database::lastInfoValue((int)Amazfish::Info::INFO_BATTERY);
    if (batteryLevel.isValid()) {
        m_lastBatteryLevel = batteryLevel.toInt();
    } else {
        qWarning() << "Cannot get battery level";
    }
}

void DeviceInterface::onConnectionStateChanged()
{
    qDebug() << Q_FUNC_INFO << connectionState();

    if (m_device) {
        if (connectionState() == "authenticated") {
            m_dbusHRM->setDevice(m_device);

            if (hrmService()) {
                m_dbusHRM->setHRMService(hrmService());
            }
            if (AmazfishConfig::instance()->appNotifyConnect() && m_notificationBuffer.isEmpty()) {
                Amazfish::WatchNotification n;
                n.id = 0;
                n.appId = "uk.co.piggz.amazfish";
                n.appName = tr("Amazfish");
                n.summary = tr("Connected");
                n.body = tr("Phone and watch are connected");
                sendAlert(n, true);
            }

            if (m_device->supportsFeature(Amazfish::Feature::FEATURE_ALERT)
                    && AmazfishConfig::instance()->appSilenceConnect()) {
                m_soundProfile.setProfile(watchfish::SoundProfile::Silent);
            }

            sendBufferedNotifications();
            updateCalendar();
            m_connectionStateChangedCount++;
        } else {
            //Terminate running operations
            m_device->abortOperations();

            if (m_device->supportsFeature(Amazfish::Feature::FEATURE_ALERT)
                    && AmazfishConfig::instance()->appSilenceConnect()) {
                m_soundProfile.setProfile(watchfish::SoundProfile::General);
            }

        }
    }
    emit connectionStateChanged();
}

QString DeviceInterface::devicePath(const QString& address)
{
    qDebug() << Q_FUNC_INFO << address;

    QString formattedAddress = address;
    formattedAddress.replace(":", "_");

    if (!determineAdapterPath(formattedAddress)) {
        qDebug() << "No device path found";
        return QString();
    }

    return m_adapterPath + "/dev_" + formattedAddress;
}

bool DeviceInterface::determineAdapterPath(const QString &address)
{
    qDebug() << Q_FUNC_INFO << address;
    AdapterModel adapterModel;

    for (int i = 0; i < adapterModel.rowCount(); ++i) {
        QVariantMap adapter = adapterModel.get(i);

        QString deviceString = adapter["itemText"].toString() + "/dev_" + address;
        qDebug() << adapter << deviceString;

        BluezAdapter bluezAdapter;
        bluezAdapter.setAdapterPath(adapter["itemText"].toString());

        if (bluezAdapter.deviceIsValid(deviceString)) {
            m_adapterPath = adapter["itemText"].toString();
            AmazfishConfig::instance()->setLocalAdapter(m_adapterPath); // Used by PTJF Device for local server
            return true;
        }
    }

    qDebug() << "No device path found";
    return false;
}

void DeviceInterface::slot_informationChanged(Amazfish::Info key, const QString &val)
{
    qDebug() << Q_FUNC_INFO << key << val;

    //Handle notification of low battery
    if (key == Amazfish::Info::INFO_BATTERY) {
        int battery_level = val.toInt();
        if (battery_level != m_lastBatteryLevel) {
            Database::storeInfo((int)Amazfish::Info::INFO_BATTERY, battery_level);
            if (battery_level <= 10 && battery_level < m_lastBatteryLevel && AmazfishConfig::instance()->appNotifyLowBattery()) {
                Amazfish::WatchNotification n;
                n.id = 0;
                n.appId = "uk.co.piggz.amazfish";
                n.appName = tr("Amazfish");
                n.summary = tr("Low Battery");
                n.body = tr("Battery level now ") + QString::number(battery_level) + "%";
                sendAlert(n);
            }
            m_lastBatteryLevel = battery_level;
        }
    }
    if (key == Amazfish::Info::INFO_STEPS) {
        bool conversionOk = false;
        int steps = val.toInt(&conversionOk);
        if (conversionOk) {
            auto goal = AmazfishConfig::instance()->profileFitnessGoal();
            m_achievements.updateStepsStatus(steps, goal);
        }
    }

    emit informationChanged((int)key, val);
}

void DeviceInterface::musicChanged()
{
    qDebug() << Q_FUNC_INFO;
    if (m_device) {
        if (AmazfishConfig::instance()->appTransliterate()) {
                m_device->setMusicStatus(m_musicController.status() == watchfish::MusicController::StatusPlaying,
                                         Transliterator::convert(m_musicController.title()),
                                         Transliterator::convert(m_musicController.artist()),
                                         Transliterator::convert(m_musicController.album()));
        } else {
            m_device->setMusicStatus(m_musicController.status() == watchfish::MusicController::StatusPlaying,
                m_musicController.title(), m_musicController.artist(), m_musicController.album());
        }
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
    case AbstractDevice::EVENT_MUSIC_PLAYPAUSE:
        m_musicController.playPause();
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
        // Chirp at once, then repeat every 5 s. Duplicate requests while the
        // alert runs must not add a chirp or push the next one back.
        if (!m_findDeviceTimer->isActive()) {
            m_findDeviceTimer->start(5000);
            findDevice();
        }
        break;
    case AbstractDevice::EVENT_CANCEL_FIND_PHONE:
        m_findDeviceTimer->stop();
        m_playedSounds = 0;
        break;
    case AbstractDevice::EVENT_WEATHER_REQUEST:
        m_currentWeather.refresh();
        break;
    }
}

void DeviceInterface::handleButtonPressed(int presses)
{
    qDebug() << Q_FUNC_INFO << presses;

    if (m_device && m_device->supportsFeature(Amazfish::Feature::FEATURE_BUTTON_ACTION)) {
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
    Amazfish::WatchNotification n;
    n.id = 0;
    n.appId = "uk.co.piggz.amazfish.calendar";
    n.appName = tr("Calendar");
    n.summary = event.title();
    n.body = event.description().isEmpty()?" ":event.description();

    sendAlert(n);
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
        Amazfish::WatchNotification n = m_notificationBuffer.dequeue();
        if (m_device->supportsFeature(Amazfish::Feature::FEATURE_ALERT)){
            qDebug() << "Sending notification";
            sendAlert(n);
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


#ifdef UUITK_EDITION
    QFile file("/opt/click.ubuntu.com/uk.co.piggz.amazfish/current/share/harbour-amazfishd/chirp.raw");
#else // elif defined(MER_EDITION_SAILFISH)
    QFile file("/usr/share/harbour-amazfishd/chirp.raw");
#endif

    if(!file.open(QIODevice::ReadOnly))
    {
        qWarning() << Q_FUNC_INFO << "Unable to open chirp sound" << file.fileName();
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
        m_firmwareInfo = m_device->firmwareInfo(file.readAll(), path);
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

void DeviceInterface::sendWeather(CurrentWeather *weather)
{
    qDebug() << Q_FUNC_INFO << *weather;
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

void DeviceInterface::onWeatherRequested()
{
    // refresh() keeps the values it already holds, so an asking watch always
    // gets a fresh fetch.
    m_currentWeather.refresh();
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
    qDebug() << Q_FUNC_INFO << "enabled: " << AmazfishConfig::instance()->appNavigationNotification()
             << icon << narrative << manDist << progress << m_device;

    if (m_device && AmazfishConfig::instance()->appNavigationNotification()) {

        if (AmazfishConfig::instance()->appTransliterate()) {
            m_device->navigationNarrative(icon, Transliterator::convert( narrative ), manDist, progress);
        } else {
            m_device->navigationNarrative(icon, narrative, manDist, progress);
        }

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
        return m_device->information((Amazfish::Info)i);
    }
    return QString();
}

void DeviceInterface::sendAlert(const QVariantMap &notification, bool allowDuplicate) {
    sendAlert(Amazfish::WatchNotification::fromQVariantMap(notification), allowDuplicate);
}

void DeviceInterface::sendAlert(const Amazfish::WatchNotification &notification, bool allowDuplicate)
{
    qDebug() << Q_FUNC_INFO;

    if (notification.body.isEmpty() && notification.summary.isEmpty()) { //ignore empty notifications
        return;
    }

    int hash = qHash(notification.appId + notification.appName + notification.summary + notification.body);
    if (hash == m_lastAlertHash && !allowDuplicate) {
        qDebug() << "Discarded duplicate alert";
        return; //Do not send duplicate alerts
    }
    m_lastAlertHash = hash;

    if (m_device && m_device->connectionState() == "authenticated" && m_device->supportsFeature(Amazfish::Feature::FEATURE_ALERT)){
        qDebug() << "Snding alert to device";

        Amazfish::WatchNotification t = notification;

        if (AmazfishConfig::instance()->appTransliterate()) {
            t.appName = Transliterator::convert(notification.appName);
            t.summary = Transliterator::convert(notification.summary);
            t.body = Transliterator::convert(notification.body);
        }
        m_device->sendAlert(t);

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
    qDebug() << Q_FUNC_INFO << "Setting:" << (Amazfish::Settings)s << (int)s;

    if (m_device) {
        m_device->applyDeviceSetting((Amazfish::Settings)s);
    }
}

void DeviceInterface::requestManualHeartrate()
{
    qDebug() << Q_FUNC_INFO;

    if (m_device) {
        m_device->requestManualHeartrate();
    }
}

void DeviceInterface::onRefreshTimer()
{
    //qDebug() << Q_FUNC_INFO;

    auto config = AmazfishConfig::instance();

    QDateTime currentDate = QDateTime::currentDateTime();

    if (m_lastWeatherSync.secsTo(currentDate) >= (config->appRefreshWeather() * 60)) {
        // qDebug() << Q_FUNC_INFO << "weather interval reached";
        m_lastWeatherSync = currentDate;
        m_currentWeather.refresh();
    }

    if (m_lastCalendarSync.secsTo(currentDate) >= (config->appRefreshCalendar() * 60)) {
        // qDebug() << Q_FUNC_INFO << "calendar interval reached";
        m_lastCalendarSync = currentDate;
        updateCalendar();
    }

    if (config->appAutoSyncData()) {
        if (m_lastActivitySync.secsTo(currentDate) >= (60*60)) {
            // qDebug() << Q_FUNC_INFO << "Auto syncing activity data";
            m_lastActivitySync = currentDate;
            fetchData((int)Amazfish::DataType::TYPE_ACTIVITY);
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

    QList<watchfish::CalendarEvent> eventlist = m_calendarSource.fetchEvents(QDate::currentDate(), QDate::currentDate().addDays(14), true);

#if 0 // TEST ONLY: change to #if 1, then AMAZFISH_FAKE_EVENTS=N adds N generated events
    int fakeCount = qEnvironmentVariableIntValue("AMAZFISH_FAKE_EVENTS");
    for (int i = 0; i < fakeCount; ++i) {
        watchfish::CalendarEvent event;
        QDateTime start = QDateTime::currentDateTime().addSecs(3600 * (fakeCount - i));
        event.setUid(QString("fake-event-%1").arg(i));
        event.setStart(start);
        event.setEnd(start.addSecs(1800));
        event.setTitle(QString("Fake event %1 - porada o rozpo\u010Dtu a pl\u00E1nov\u00E1n\u00ED t\u00FDmu").arg(i));
        event.setLocation(QString("Zasedac\u00ED m\u00EDstnost %1, Brno, \u010Cesk\u00E1 republika").arg(i));
        event.setDescription(QString("Popis ud\u00E1losti %1. ").arg(i).repeated(20));
        event.setAllDay(false);
        eventlist.append(event);
    }
    qDebug() << Q_FUNC_INFO << "events:" << eventlist.count() << "fake:" << fakeCount;
#endif

    if (AmazfishConfig::instance()->appTransliterate()) {
        for (watchfish::CalendarEvent &event : eventlist) {
            event.setTitle(Transliterator::convert(event.title()));
            event.setLocation(Transliterator::convert(event.location()));
            event.setDescription(Transliterator::convert(event.description()));
        }
    }

    if (supportsFeature(int(Amazfish::Feature::FEATURE_EVENT_REMINDER))) {
        if (m_device) {
            m_device->syncCalendar(eventlist);
        }
    } else if (AmazfishConfig::instance()->appSimulateEventSupport()){
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
        m_device->enableFeature((Amazfish::Feature)feature);
    }
}

void DeviceInterface::fetchData(int dataType)
{
    if (m_device) {
        m_device->fetchData((Amazfish::DataTypes)dataType);
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
    bool result = (supportedFeatures() & f);
    //qDebug() << Q_FUNC_INFO << (Amazfish::Feature)f << result;
    return result;
}

bool DeviceInterface::supportsDataType(int t) {
    bool result = (supportedDataTypes() & t);
    //qDebug() << Q_FUNC_INFO << (Amazfish::DataType)t << result;
    return result;
}


int DeviceInterface::supportedFeatures()
{
    if (m_device) {
        return m_device->supportedFeatures();
    }
    return 0;
}

int DeviceInterface::supportedDataTypes()
{
    if (m_device) {
        return m_device->supportedDataTypes();
    }
    return 0;
}

void DeviceInterface::immediateAlert(int level)
{
    if (m_device) {
        m_device->immediateAlert(level);
    }
}

void DeviceInterface::agentPasskeyResponse(const uint response)
{
    if (m_agent) {
	m_agent->requestPasskeyResponse(response);
    }
}

void DeviceInterface::agentPinCodeResponse(const QString& response)
{
    if (m_agent) {
	m_agent->requestPinCodeResponse(response);
    }
}


void DeviceInterface::agentConfirmationResponse(const bool response)
{
    if (m_agent) {
	m_agent->requestConfirmationResponse(response);
    }
}
