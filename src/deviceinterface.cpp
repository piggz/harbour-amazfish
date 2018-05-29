#include "deviceinterface.h"

#include <QDir>
#include <KDb3/KDbDriverManager>

DeviceInterface::DeviceInterface()
{
    m_notificationListener = new NotificationsListener(this);

    m_bipDevice = new BipDevice();
    connect(m_bipDevice, &BipDevice::connectionStateChanged, this, &DeviceInterface::onConnectionStateChanged);
    connect(m_bipDevice, &BipDevice::message, this, &DeviceInterface::message);

    m_adapter.setAdapterPath("/org/bluez/hci0");

    QDir d;
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    setupDatabase();

    //m_db = QSqlDatabase::addDatabase("QSQLITE");
    //m_db.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+ "/amazfish.sqlite");
    //qDebug() << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/amazfish.sqlite";
    //m_db.open();
    //createTables();

    //Notifications
    connect(m_notificationListener, &NotificationsListener::notificationReceived, this, &DeviceInterface::notificationReceived);

    // Calls
    m_voiceCallManager = new VoiceCallManager(this);
    connect(m_voiceCallManager, &VoiceCallManager::activeVoiceCallChanged, this, &DeviceInterface::onActiveVoiceCallChanged);

    emit connectionStateChanged();
}

void DeviceInterface::connectToDevice(const QString &address)
{
    qDebug() << "BipInterface::connectToDevice:" << address;

    if (m_adapter.deviceIsValid(address)) {
        m_deviceAddress = address;
        m_bipDevice->setDevicePath(address);
        m_bipDevice->connectToDevice();
    }
    else {
        qDebug() << "BipInterface::connectToDevice:device was not valid";
    }
}

QString DeviceInterface::pair(const QString &address)
{
    qDebug() << "BipInterface::pair:" << address;
    m_deviceAddress = address;
    m_bipDevice->setDevicePath(address);
    return m_bipDevice->pair();
}

void DeviceInterface::disconnect()
{
    qDebug() << "BipInterface::disconnect";
    m_bipDevice->disconnectFromDevice();
}

bool DeviceInterface::ready() const
{
    return m_bipDevice->connectionState() == "authenticated";
}

QString DeviceInterface::connectionState() const
{
    return m_bipDevice->connectionState();
}

DeviceInfoService *DeviceInterface::infoService() const
{
    return qobject_cast<DeviceInfoService*>(m_bipDevice->service(BipDevice::UUID_SERVICE_DEVICEINFO));
}

MiBandService *DeviceInterface::miBandService() const
{
    return qobject_cast<MiBandService*>(m_bipDevice->service(BipDevice::UUID_SERVICE_MIBAND));
}

MiBand2Service *DeviceInterface::miBand2Service() const
{
    return qobject_cast<MiBand2Service*>(m_bipDevice->service(BipDevice::UUID_SERVICE_MIBAND2));
}

AlertNotificationService *DeviceInterface::alertNotificationService() const
{
    return qobject_cast<AlertNotificationService*>(m_bipDevice->service(BipDevice::UUID_SERVICE_ALERT_NOTIFICATION));
}

HRMService *DeviceInterface::hrmService() const
{
    return qobject_cast<HRMService*>(m_bipDevice->service(BipDevice::UUID_SERVICE_HRM));
}



void DeviceInterface::notificationReceived(const QString &appName, const QString &summary, const QString &body)
{
    if (alertNotificationService()){
        alertNotificationService()->sendAlert(appName, summary, body);
    }
}

void DeviceInterface::onActiveVoiceCallChanged()
{

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
    VoiceCallHandler* handler = m_voiceCallManager->activeVoiceCall();

    if (!handler || handler->handlerId().isNull()) {
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
            alertNotificationService()->incomingCall(m_voiceCallManager->findPersonByNumber(handler->lineId()));
        }
        break;
    case VoiceCallHandler::STATUS_NULL:
    case VoiceCallHandler::STATUS_DISCONNECTED:
        qDebug() << "Endphone " << handler->handlerId();
        if(handler->getState() < VoiceCallHandler::StateCleanedUp) {
            handler->setState(VoiceCallHandler::StateCleanedUp);
            //emit callEnded(qHash(handler->handlerId()), false);
        }
        break;
    case VoiceCallHandler::STATUS_ACTIVE:
        qDebug() << "Startphone" << handler->handlerId();
        if(handler->getState() < VoiceCallHandler::StateAnswered) {
            handler->setState(VoiceCallHandler::StateAnswered);
            //emit callStarted(qHash(handler->handlerId()));
        }
        break;
    case VoiceCallHandler::STATUS_HELD:
        qDebug() << "OnHold" << handler->handlerId();
        break;
    }
}

void DeviceInterface::createTables()
{
#if 0
    if (m_db.isOpen()) {
        QSqlQuery qry;

        qry.prepare( "CREATE TABLE \"MI_BAND_ACTIVITY_SAMPLE\" ("  //
                     "\"TIMESTAMP\" INTEGER  NOT NULL ,"  // 0: timestamp
                     "\"DEVICE_ID\" INTEGER  NOT NULL ,"  // 1: deviceId
                     "\"USER_ID\" INTEGER NOT NULL ,"  // 2: userId
                     "\"RAW_INTENSITY\" INTEGER NOT NULL ,"  // 3: rawIntensity
                     "\"STEPS\" INTEGER NOT NULL ,"  // 4: steps
                     "\"RAW_KIND\" INTEGER NOT NULL ,"  // 5: rawKind
                     "\"HEART_RATE\" INTEGER NOT NULL ,"  // 6: heartRate
                     "PRIMARY KEY ("
                     "\"TIMESTAMP\" ,"
                     "\"DEVICE_ID\" ) ON CONFLICT REPLACE);");

        bool ret = qry.exec();
        if(!ret) {
            qDebug() << qry.lastError();
        } else {
            qDebug() << "activity table created!";
        }
    } else {
        qDebug() << "Database not open";
    }
#endif

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
    if (connectionState() == "authenticated") {
        if (miBandService()) {
            miBandService()->setDatabase(dbConnection());
        }
    }
    emit connectionStateChanged();
}
