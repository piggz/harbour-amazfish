#include "banglejsdevice.h"
#include "batteryservice.h"
#include "uartservice.h"
#include "deviceinfoservice.h"
#include "amazfishconfig.h"
#include "activitysample.h"

#include <QtXml/QtXml>

BangleJSDevice::BangleJSDevice(const QString &pairedName, QObject *parent) : AbstractDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO << pairedName;
    connect(this, &QBLEDevice::propertiesChanged, this, &BangleJSDevice::onPropertiesChanged, Qt::UniqueConnection);
}

void BangleJSDevice::pair()
{
    qDebug() << Q_FUNC_INFO;

    m_needsAuth = false;
    m_pairing = true;
    m_autoreconnect = true;
    //disconnectFromDevice();
    setConnectionState("pairing");
    emit connectionStateChanged();

    QBLEDevice::pair();
}

Amazfish::Features BangleJSDevice::supportedFeatures() const
{
    return Amazfish::Feature::FEATURE_STEPS |
           Amazfish::Feature::FEATURE_HRM |
           Amazfish::Feature::FEATURE_ALERT |
           Amazfish::Feature::FEATURE_MUSIC_CONTROL |
           Amazfish::Feature::FEATURE_WEATHER;
}

Amazfish::DataTypes BangleJSDevice::supportedDataTypes() const
{
    return Amazfish::DataType::TYPE_HEART_RATE;
}

QString BangleJSDevice::deviceType() const
{
    return "banglejs";
}

void BangleJSDevice::abortOperations()
{
    qDebug() << Q_FUNC_INFO;

}

void BangleJSDevice::sendAlert(const Amazfish::WatchNotification &notification)
{
    qDebug() << Q_FUNC_INFO;

    UARTService *uart = qobject_cast<UARTService*>(service(UARTService::UUID_SERVICE_UART));
    if (!uart){
        return;
    }

    //For mails, there is a double notification which is empty except for sender
    if(notification.body.isEmpty() && notification.summary.isEmpty())
    {
        return;
    }

    QString src = notification.appName;
    if (notification.appName == "") {
        // gadgedbridge does this https://codeberg.org/Freeyourgadget/Gadgetbridge/src/commit/3c8a9b5821160e80639a11554c42bba7dd4aece3/app/src/main/java/nodomain/freeyourgadget/gadgetbridge/service/devices/banglejs/BangleJSDeviceSupport.java#L1436
        // mapss probably at? https://github.com/espruino/BangleApps/blob/ec987e1ee2d4d6a3302baf9a26bbb2aae6f91737/apps/messageicons/icons/icon_names.json#L133
        src = "SMS Message";
    }

    QJsonObject o;
    o.insert("t", "notify");
    o.insert("id", notification.id); //id is necessary for some apps like messageui, and should be unique
    o.insert("src", src);
    o.insert("title", "");
    o.insert("subject", notification.summary);
    o.insert("body", notification.body);
    o.insert("sender", notification.appName);
    o.insert("tel", "");
    uart->txJson(o);
}

void BangleJSDevice::incomingCall(const QString &caller)
{
    qDebug() << Q_FUNC_INFO;

    UARTService *uart = qobject_cast<UARTService*>(service(UARTService::UUID_SERVICE_UART));
    if (!uart){
        return;
    }

    QJsonObject o;
    o.insert("t", "call");
    o.insert("cmd", "incoming");
    o.insert("name", caller);
    o.insert("number", "");
    uart->txJson(o);
}

void BangleJSDevice::incomingCallEnded()
{
    qDebug() << Q_FUNC_INFO;

    UARTService *uart = qobject_cast<UARTService*>(service(UARTService::UUID_SERVICE_UART));
    if (!uart){
        return;
    }

    QJsonObject o;
    o.insert("t", "call");
    o.insert("cmd", "end");
    o.insert("name", "");
    o.insert("number", "");
    uart->txJson(o);
}


void BangleJSDevice::parseServices()
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

            if (uuid == UARTService::UUID_SERVICE_UART && !service(UARTService::UUID_SERVICE_UART)) {
                addService(UARTService::UUID_SERVICE_UART, new UARTService(path, this));
            } else if (uuid == BatteryService::UUID_SERVICE_BATTERY && !service(BatteryService::UUID_SERVICE_BATTERY)) {
                addService(BatteryService::UUID_SERVICE_BATTERY, new BatteryService(path, this));
            } else if (uuid == DeviceInfoService::UUID_SERVICE_DEVICEINFO  && !service(DeviceInfoService::UUID_SERVICE_DEVICEINFO)) {
                addService(DeviceInfoService::UUID_SERVICE_DEVICEINFO, new DeviceInfoService(path, this));
            } else if ( !service(uuid)) {
                addService(uuid, new QBLEService(uuid, path, this));
            }
        }
    }
}

void BangleJSDevice::initialise()
{
    qDebug() << Q_FUNC_INFO;
    setConnectionState("connected");
    parseServices();

    UARTService *uart = qobject_cast<UARTService*>(service(UARTService::UUID_SERVICE_UART));
    if (uart){
        connect(uart, &UARTService::message, this, &BangleJSDevice::message, Qt::UniqueConnection);
        connect(uart, &UARTService::jsonRx, this, &BangleJSDevice::handleRxJson, Qt::UniqueConnection);

        uart->enableNotification(UARTService::UUID_CHARACTERISTIC_UART_RX);
        uart->tx(QByteArray(1, 0x03)); //Clear line)
    }

    BatteryService *battery = qobject_cast<BatteryService*>(service(BatteryService::UUID_SERVICE_BATTERY));
    if (battery) {
        connect(battery, &BatteryService::informationChanged, this, &BangleJSDevice::informationChanged, Qt::UniqueConnection);
    }

    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (info) {
        connect(info, &DeviceInfoService::informationChanged, this, &BangleJSDevice::informationChanged, Qt::UniqueConnection);
    }

    setConnectionState("authenticated");

    setTime();
}

void BangleJSDevice::setTime() {
    UARTService *uart = qobject_cast<UARTService*>(service(UARTService::UUID_SERVICE_UART));
    if (!uart){
        return;
    }

    qint64 ts;

    QDateTime now = QDateTime::currentDateTime();
    QTimeZone timeZone = QTimeZone::systemTimeZone();
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    ts = now.currentSecsSinceEpoch();
#else
    ts = now.currentDateTime().toTime_t();
#endif

    // Get the offset in seconds and convert to hours
    int offsetSeconds = timeZone.offsetFromUtc(now);
    double offsetHours = offsetSeconds / 3600.0;

    QString cmd = QString("setTime(%1);\nE.setTimeZone(%2);\n(s=>s&&(s.timezone=%2,require('Storage').write('setting.json',s)))(require('Storage').readJSON('setting.json',1));").arg(ts).arg(offsetHours);

    uart->tx(QByteArray(1, 0x10) + cmd.toUtf8());
}

void BangleJSDevice::onPropertiesChanged(QString interface, QVariantMap map, QStringList list)
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

void BangleJSDevice::authenticated(bool ready)
{
    qDebug() << Q_FUNC_INFO << ready;

    if (ready) {
        setConnectionState("authenticated");
    } else {
        setConnectionState("authfailed");
    }
}

AbstractFirmwareInfo *BangleJSDevice::firmwareInfo(const QByteArray &bytes, const QString &path)
{
    qDebug() << Q_FUNC_INFO;
    return nullptr;
}

void BangleJSDevice::navigationRunning(bool running)
{
    qDebug() << Q_FUNC_INFO;

}

void BangleJSDevice::navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress)
{
    qDebug() << Q_FUNC_INFO;
}

void BangleJSDevice::downloadActivityData() {
    qDebug() << Q_FUNC_INFO;
    UARTService *uart = qobject_cast<UARTService*>(service(UARTService::UUID_SERVICE_UART));
    if (!uart) {
        return;
    }

    qlonglong ls = AmazfishConfig::instance()->value("device/lastactivitysyncmillis").toLongLong();

    m_samples.clear();
    QJsonObject o;
    o.insert("t", "actfetch");
    o.insert("ts", ls);
    uart->txJson(o);


}

void BangleJSDevice::sendWeather(CurrentWeather *weather)
{
    UARTService *uart = qobject_cast<UARTService*>(service(UARTService::UUID_SERVICE_UART));
    if (uart){
        QJsonObject o;
        // t:"weather",
        // temp,hi,lo,hum,rain,uv,code,txt,wind,wdir,loc
        // weather report (current temp, days highest temp, days lowest temp, current humidity, rain/precip probability, UV Index, current condition code, current condition text, wind speed, wind direction)

        o.insert("t", "weather");
        o.insert("temp", weather->temperature());
        o.insert("hi", weather->maxTemperature());
        o.insert("lo", weather->minTemperature());
        o.insert("hum", weather->humidity());
        // rain
        // uv
        o.insert("code", weather->weatherCode());
        o.insert("txt", weather->description());
        o.insert("wind", weather->windSpeed());
        o.insert("wdir", weather->windDeg());
        o.insert("loc", weather->city()->name());


        uart->txJson(o);
    }
}

void BangleJSDevice::setMusicStatus(bool playing, const QString &title, const QString &artist, const QString &album, int duration, int position)
{
    qDebug() << Q_FUNC_INFO;

    UARTService *uart = qobject_cast<UARTService*>(service(UARTService::UUID_SERVICE_UART));
    if (!uart){
        return;
    }

    QJsonObject o;
    o.insert("t", "musicinfo");
    o.insert("artist", artist);
    o.insert("album", album);
    o.insert("track", title);
    o.insert("dur", duration);
    o.insert("c", 0);
    o.insert("n", 0);
    uart->txJson(o);

    QJsonObject p;
    p.insert("t", "musicstate");
    p.insert("state", playing? "play" : "pause");
    p.insert("position", position);
    p.insert("shuffle", 0);
    p.insert("repeat", 0);
    uart->txJson(p);

}

void BangleJSDevice::prepareFirmwareDownload(const AbstractFirmwareInfo *info)
{
    qDebug() << Q_FUNC_INFO;

}

void BangleJSDevice::startDownload()
{
    qDebug() << Q_FUNC_INFO;
}

void BangleJSDevice::refreshInformation()
{
    qDebug() << Q_FUNC_INFO;
    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (info) {
        info->refreshInformation();
    }

    BatteryService *bat = qobject_cast<BatteryService*>(service(BatteryService::UUID_SERVICE_BATTERY));
    if (bat) {
        bat->refreshInformation();
    }


}

QString BangleJSDevice::information(Amazfish::Info i) const
{
    qDebug() << Q_FUNC_INFO << i;

    switch (i) {
    case Amazfish::Info::INFO_BATTERY:
        return QString::number(m_infoBatteryLevel);
    case Amazfish::Info::INFO_SWVER:
        return m_firmwareVersion;
    case Amazfish::Info::INFO_HWVER:
        return m_hardwareVersion;
    case Amazfish::Info::INFO_STEPS:
        return QString::number(m_steps);
    case Amazfish::Info::INFO_HEARTRATE:
        return QString::number(m_heartrate);
    default:
        break;
    }

    return QString();
}

void BangleJSDevice::serviceEvent(uint8_t event)
{
    qDebug() << Q_FUNC_INFO;

}

void BangleJSDevice::handleRxJson(const QJsonObject &json)
{
    qDebug() << Q_FUNC_INFO << json;

    QString t = json.value("t").toString();
    if (t == "info") {
        emit message(json.value("msg").toString());
    } else if (t == "warn") {
        emit message(json.value("msg").toString());
    } else if (t == "error") {
        emit message(json.value("msg").toString());
    } else if (t == "ver") {
        if (json.keys().contains("fw")) {
            m_firmwareVersion = json.value("fw").toString();
        }
        if (json.keys().contains("fw1")) {
            m_firmwareVersion = json.value("fw1").toString();
        }
        if (json.keys().contains("fw2")) {
            m_firmwareVersion = json.value("fw2").toString();
        }
        emit informationChanged(Amazfish::Info::INFO_SWVER, m_firmwareVersion);

        if (json.keys().contains("hw")) {
            m_hardwareVersion = QString::number(json.value("hw").toInt());
        }
        emit informationChanged(Amazfish::Info::INFO_HWVER, m_hardwareVersion);

    } else if (t == "status") {
        m_infoBatteryLevel = json.value("bat").toInt();
        emit informationChanged(Amazfish::Info::INFO_BATTERY, QString::number(m_infoBatteryLevel));
    } else if (t == "findPhone") {
        bool running = json.value("n").toBool();
        qDebug() << "findPhone" << running;
        if (running) {
            emit deviceEvent(AbstractDevice::EVENT_FIND_PHONE);
        } else {
            emit deviceEvent(AbstractDevice::EVENT_CANCEL_FIND_PHONE);

        }

    } else if (t == "music") {
        QString music_action = json.value("n").toString();

        if (music_action == "play") {
            emit deviceEvent(AbstractDevice::EVENT_MUSIC_PLAY);
        } else if (music_action == "pause") {
            emit deviceEvent(AbstractDevice::EVENT_MUSIC_PAUSE);
        } else if (music_action == "next") {
            emit deviceEvent(AbstractDevice::EVENT_MUSIC_NEXT);
        } else if (music_action == "previous") {
            emit deviceEvent(AbstractDevice::EVENT_MUSIC_PREV);
        } else if (music_action == "volumeup") {
            emit deviceEvent(AbstractDevice::EVENT_MUSIC_VOLUP);
        } else if (music_action == "volumedown") {
            emit deviceEvent(AbstractDevice::EVENT_MUSIC_VOLDOWN);
        }

//            emit deviceEvent(AbstractDevice::EVENT_APP_MUSIC);

//    } else if (t == "call") {
//    } else if (t == "notify") {
    } else if (t == "actfetch") {
        int sample_count = json.value("count").toInt();
        QString fetch_state = json.value("state").toString();
        if ((fetch_state == "end") && (sample_count > 0) && (m_samples.count() > 0)) {
            QDateTime lastItemDateTime = m_samples.last().datetime(); // grab timestamp before m_samples.clear()
            saveActivitySamples();
            AmazfishConfig::instance()->setValue("device/lastactivitysyncmillis", lastItemDateTime.toMSecsSinceEpoch());
        }
    } else if (t == "act") {

        long ts = json.value("ts").toVariant().toLongLong();      // timestamp

#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
        QDateTime actDate= QDateTime::fromMSecsSinceEpoch(ts);
#else
        QDateTime actDate = QDateTime::fromTime_t(ts / 1000);
#endif

        m_heartrate = json.value("hrm").toInt(); // heart rate,
        m_steps = json.value("stp").toInt();     // steps
        int mov = json.value("mov").toInt();     // movement intensity
        int rt = json.value("rt").toInt();       // realtime
        QString act;
        if (json.keys().contains("act")) {
            act = json.value("act").toString();
        }
        ActivityKind::Type kind = convertToActivityKind(act);

        if (rt) {
            qDebug() << "Realtime activity " << actDate << kind << mov << m_steps << m_heartrate  ;
            emit informationChanged(Amazfish::Info::INFO_HEARTRATE, QString::number(m_heartrate));
            emit informationChanged(Amazfish::Info::INFO_STEPS, QString::number(m_steps));
        } else {
            m_samples << ActivitySample(actDate, kind, mov, m_steps, m_heartrate);
        }
    } else {
        qDebug() << "Gadgetbridge type " << t;
    }
}

ActivityKind::Type BangleJSDevice::convertToActivityKind(const QString &bangle_kind) {
    static const QHash<QString, ActivityKind::Type> map =
    {
        {"UNKNOWN", ActivityKind::Unknown},
        {"NOT_WORN", ActivityKind::NotWorn},
        {"WALKING", ActivityKind::Walking},
        {"EXERCISE", ActivityKind::Exercise},
        {"LIGHT_SLEEP", ActivityKind::LightSleep},
        {"DEEP_SLEEP", ActivityKind::DeepSleep},
    };
    return map.value(bangle_kind, ActivityKind::Unknown);
}

bool BangleJSDevice::saveActivitySamples() {
    uint user_id = qHash(AmazfishConfig::instance()->profileName());
    uint device_id = qHash(AmazfishConfig::instance()->pairedAddress());

    KDbTransaction transaction = m_conn->beginTransaction();
    KDbTransactionGuard tg(transaction);

    KDbFieldList fields;
    auto mibandActivity = m_conn->tableSchema("mi_band_activity");

    fields.addField(mibandActivity->field("timestamp"));
    fields.addField(mibandActivity->field("timestamp_dt"));
    fields.addField(mibandActivity->field("device_id"));
    fields.addField(mibandActivity->field("user_id"));
    fields.addField(mibandActivity->field("raw_intensity"));
    fields.addField(mibandActivity->field("steps"));
    fields.addField(mibandActivity->field("raw_kind"));
    fields.addField(mibandActivity->field("heartrate"));

    for (int i = 0; i < m_samples.count(); ++i) {
        QList<QVariant> values;

        values << m_samples[i].datetime().toMSecsSinceEpoch() / 1000;
        values << m_samples[i].datetime();
        values << device_id;
        values << user_id;
        values << m_samples[i].intensity();
        values << m_samples[i].steps();
        values << m_samples[i].kind();
        values << m_samples[i].heartrate();

        if (!m_conn->insertRecord(&fields, values)) {
            qDebug() << Q_FUNC_INFO << "error inserting record";
            return false;
        }
    }
    tg.commit();
    m_samples.clear();
    return true;
}
