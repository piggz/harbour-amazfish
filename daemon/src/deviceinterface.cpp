#include "deviceinterface.h"

#include "deviceinfoservice.h"
#include "mibandservice.h"
#include "miband2service.h"
#include "alertnotificationservice.h"
#include "hrmservice.h"
#include "bipfirmwareservice.h"
#include "devicefactory.h"

#include <QDir>
#include <KDb3/KDbDriverManager>

static const char *SERVICE = SERVICE_NAME;
static const char *PATH = "/application";

DeviceInterface::DeviceInterface()
{
    //Start by registering on DBUS
    registerDBus();

    //Intercept notificaions
    m_notificationListener = new NotificationsListener(this);

    //Create a device object
    m_device = DeviceFactory::createDevice(m_settings.value("/uk/co/piggz/amazfish/pairedName").toString());
    if (m_device) {
        connect(m_device, &AbstractDevice::connectionStateChanged, this, &DeviceInterface::onConnectionStateChanged);
        connect(m_device, &AbstractDevice::message, this, &DeviceInterface::message);
        connect(m_device, &AbstractDevice::downloadProgress, this, &DeviceInterface::downloadProgress);
        connect(m_device, &QBLEDevice::operationRunningChanged, this, &DeviceInterface::operationRunningChanged);
        connect(m_device, &AbstractDevice::buttonPressed, this, &DeviceInterface::buttonPressed);
        connect(m_device, &AbstractDevice::informationChanged, this, &DeviceInterface::slot_informationChanged);
    }

    m_adapter.setAdapterPath("/org/bluez/hci0");

    //Create the DBUS HRM Interface
    m_dbusHRM = new DBusHRM(this);

    setupDatabase();

    //Notifications
    connect(m_notificationListener, &NotificationsListener::notificationReceived, this, &DeviceInterface::notificationReceived);

    // Calls
    m_voiceCallManager = new VoiceCallManager(this);
    connect(m_voiceCallManager, &VoiceCallManager::activeVoiceCallChanged, this, &DeviceInterface::onActiveVoiceCallChanged);

    //Weather
    connect(&m_cityManager, &CityManager::citiesChanged, this, &DeviceInterface::onCitiesChanged);
    connect(&m_currentWeather, &CurrentWeather::ready, this, &DeviceInterface::onWeatherReady);
    if (m_cityManager.cities().count() > 0) {
        m_currentWeather.setCity(qobject_cast<City*>(m_cityManager.cities()[0]));
    }

    //Calendar

    updateCalendar();

    //Refresh timer
    m_refreshTimer = new QTimer();
    connect(m_refreshTimer, &QTimer::timeout, this, &DeviceInterface::onRefreshTimer);
    m_refreshTimer->start(60000);

    //Finally, connect to device if it is defined
    QString pairedAddress = m_settings.value("/uk/co/piggz/amazfish/pairedAddress").toString();
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

    if (m_device /* && m_adapter.deviceIsValid(address)*/) {
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
        m_device->pair();
        return "";
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

void DeviceInterface::notificationReceived(const QString &appName, const QString &summary, const QString &body)
{
    if (m_device && m_device->connectionState() == "authenticated" && m_device->supportsFeature(AbstractDevice::FEATURE_ALERT)){
        m_device->sendAlert(appName, summary, body);
    } else {
        qDebug() << "no notification service, buffering notification";
        WatchNotification n;
        n.appName = appName;
        n.summary = summary;
        n.body = body;
        m_notificationBuffer.enqueue(n);
        if (m_notificationBuffer.count() > 10) {
            m_notificationBuffer.dequeue();
        }
    }
}

void DeviceInterface::onActiveVoiceCallChanged()
{

    qDebug() << "onActiveVoiceCallChanged";

    VoiceCallHandler* handler = m_voiceCallManager->activeVoiceCall();
    if (handler) {
        connect(handler, SIGNAL(statusChanged()), SLOT(onActiveVoiceCallStatusChanged()));
        connect(handler, SIGNAL(destroyed()), SLOT(onActiveVoiceCallStatusChanged()));
        connect(miBandService(), &MiBandService::declineCall, handler, &VoiceCallHandler::hangup, Qt::UniqueConnection);
        connect(miBandService(), &MiBandService::ignoreCall, m_voiceCallManager, &VoiceCallManager::silenceRingtone, Qt::UniqueConnection);


        if (handler->status()) onActiveVoiceCallStatusChanged();
    }
}

void DeviceInterface::onActiveVoiceCallStatusChanged()
{
    qDebug() << "onActiveVoiceCallStatusChanged";

    VoiceCallHandler* handler = m_voiceCallManager->activeVoiceCall();

    if (!handler || handler->handlerId().isNull() || !m_device) {
        return;
    }

    switch ((VoiceCallHandler::VoiceCallStatus)handler->status()) {
    case VoiceCallHandler::STATUS_ALERTING:
    case VoiceCallHandler::STATUS_DIALING:
        qDebug() << "Tell outgoing:" << handler->lineId();
        //emit outgoingCall(handlerId, handler->lineId(), m_voiceCallManager->findPersonByNumber(handler->lineId()));
        break;
    case VoiceCallHandler::STATUS_INCOMING:
    case VoiceCallHandler::STATUS_WAITING:
        qDebug() << "Tell incoming:" << handler->lineId();
        if(handler->getState() < VoiceCallHandler::StateRinging) {
            handler->setState(VoiceCallHandler::StateRinging);
            m_device->incomingCall(m_voiceCallManager->findPersonByNumber(handler->lineId()));
        }
        break;
    case VoiceCallHandler::STATUS_NULL:
    case VoiceCallHandler::STATUS_DISCONNECTED:
        qDebug() << "Endphone " << handler->handlerId();
        if(handler->getState() < VoiceCallHandler::StateCleanedUp) {
            handler->setState(VoiceCallHandler::StateCleanedUp);
            if (m_device->service("00001802-0000-1000-8000-00805f9b34fb")){
                m_device->service("00001802-0000-1000-8000-00805f9b34fb")->writeValue("00002a06-0000-1000-8000-00805f9b34fb", QByteArray(1, 0x00)); //TODO properly abstract immediate notification service
            }
        }
        break;
    case VoiceCallHandler::STATUS_ACTIVE:
        qDebug() << "Startphone" << handler->handlerId();
        if(handler->getState() < VoiceCallHandler::StateAnswered) {
            handler->setState(VoiceCallHandler::StateAnswered);
            if (m_device->service("00001802-0000-1000-8000-00805f9b34fb")){
                m_device->service("00001802-0000-1000-8000-00805f9b34fb")->writeValue("00002a06-0000-1000-8000-00805f9b34fb", QByteArray(1, 0x00)); //TODO properly abstract immediate notification service
            }        }
        break;
    case VoiceCallHandler::STATUS_HELD:
        qDebug() << "OnHold" << handler->handlerId();
        break;
    }
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

        t_summary->addField(f = new KDbField("kind", KDbField::Integer, nullptr, KDbField::Unsigned));
        f->setCaption("Activity Kind");

        t_summary->addField(f = new KDbField("base_longitude", KDbField::Double));
        f->setCaption("Base Longitude");
        t_summary->addField(f = new KDbField("base_latitude", KDbField::Double));
        f->setCaption("Base Latitude");
        t_summary->addField(f = new KDbField("base_altitude", KDbField::Double));
        f->setCaption("Base Altitude");

        t_summary->addField(f = new KDbField("device_id", KDbField::Integer, nullptr, KDbField::Unsigned));
        f->setCaption("Device ID");
        t_summary->addField(f = new KDbField("user_id", KDbField::Integer, nullptr, KDbField::Unsigned));
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
        if (m_settings.value("/uk/co/piggz/amazfish/app/notifyconnect").toBool()) {
            sendAlert(tr("Amazfish"), tr("Connected"), tr("Phone and watch are connected"), true);
        }

        sendBufferedNotifications();
        updateCalendar();

        //TODO this enables music controls
        miBandService()->writeChunked(MiBandService::UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 3, QByteArray("\x01\x00\x01\x00\x00\x00\x01\x00", 8));
    } else {
        //Terminate running operations
        m_device->abortOperations();
    }
    emit connectionStateChanged();
}

void DeviceInterface::slot_informationChanged(AbstractDevice::Info key, const QString &val)
{
    qDebug() << "slot_informationChanged" << key << val;

    //Handle notification of low battery
    if (key == AbstractDevice::INFO_BATTERY) {
        if (val.toInt() != m_lastBatteryLevel) {
            if (val.toInt() <= 10 && val.toInt() < m_lastBatteryLevel && m_settings.value("/uk/co/piggz/amazfish/app/notifylowbattery").toBool()) {
                sendAlert("Amazfish", tr("Low Battery"), tr("Battery level now ") + QString::number(m_lastBatteryLevel) + "%");
            }
            m_lastBatteryLevel = val.toInt();
        }
    }

    emit informationChanged(key, val);
}

void DeviceInterface::sendBufferedNotifications()
{
    qDebug() << "Sending buffered notifications";
    while (m_notificationBuffer.count() > 0) {
        WatchNotification n = m_notificationBuffer.dequeue();
        if (m_device->supportsFeature(AbstractDevice::FEATURE_ALERT)){
            qDebug() << "Sending notification";
            sendAlert(n.appName, n.summary, n.body);
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
        m_device->prepareFirmwareDownload(m_firmwareInfo);
        return m_firmwareInfo->version();
    }
    return QString();
}

bool DeviceInterface::startDownload()
{
    if (!m_firmwareInfo->supportedOnDevice(m_device->deviceName())) {
        return false;
    }
    m_device->startDownload();
    return true;
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
    if (miBandService()) {
        miBandService()->sendWeather(weather);
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
    int hash = qHash(sender + subject + message);
    if (hash == m_lastAlertHash && !allowDuplicate) {
        qDebug() << "Discarded duplicate alert";
        return; //Do not send duplicate alerts
    }
    m_lastAlertHash = hash;

    notificationReceived(sender, subject, message);
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

    syncWeatherMinutes++;
    if (syncWeatherMinutes >= m_settings.value("/uk/co/piggz/amazfish/app/refreshweather").toInt()) {
        syncWeatherMinutes = 0;
        qDebug() << "weather interval reached";
        m_currentWeather.refresh();
    }
    syncCalendarMinutes++;
    if (syncCalendarMinutes >= m_settings.value("/uk/co/piggz/amazfish/app/refreshcalendar").toInt()) {
        syncCalendarMinutes = 0;
        qDebug() << "calendar interval reached";
        updateCalendar();
    }

    if (m_settings.value("/uk/co/piggz/amazfish/app/autosyncdata").toBool()) {
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
            QList<CalendarReader::EventData> eventlist = m_calendarReader.getEvents();

            int id=0;
            foreach (CalendarReader::EventData event, eventlist) {
                qDebug() << event.uniqueId << event.displayLabel << event.description << event.startTime;
                m_device->sendEventReminder(id, event.startTime, event.displayLabel);
                id++;
            }
        }
    }
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

