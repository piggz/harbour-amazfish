#include "banglejsdevice.h"
#include "uartservice.h"

#include <QtXml/QtXml>

BangleJSDevice::BangleJSDevice(const QString &pairedName, QObject *parent) : AbstractDevice(pairedName, parent)
{
    qDebug() << "BangleJSDevice:: " << pairedName;
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

    QBLEDevice::connectToDevice();
}

int BangleJSDevice::supportedFeatures() const
{
    return FEATURE_HRM |
            FEATURE_ALERT |
            FEATURE_MUSIC_CONTROL |
            FEATURE_WEATHER;
}

QString BangleJSDevice::deviceType() const
{
    return "pinetimejf";
}

void BangleJSDevice::abortOperations()
{
    qDebug() << Q_FUNC_INFO;

}

void BangleJSDevice::sendAlert(const QString &sender, const QString &subject, const QString &message)
{
    qDebug() << Q_FUNC_INFO;

    UARTService *uart = qobject_cast<UARTService*>(service(UARTService::UUID_SERVICE_UART));
    if (!uart){
        return;
    }

    QJsonObject o;
    o.insert("t", "notify");
    o.insert("id", "");
    o.insert("src", "");
    o.insert("title", "");
    o.insert("subject", subject);
    o.insert("body", message);
    o.insert("sender", sender);
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
    o.insert("cmd", "accept");
    o.insert("name", caller);
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

    setConnectionState("authenticated");
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
                qDebug() << "DisConnected!";
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

AbstractFirmwareInfo *BangleJSDevice::firmwareInfo(const QByteArray &bytes)
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

void BangleJSDevice::sendWeather(CurrentWeather *weather)
{
    UARTService *uart = qobject_cast<UARTService*>(service(UARTService::UUID_SERVICE_UART));
    if (uart){
        QJsonObject o;
        o.insert("t", "weather");
        o.insert("temp", weather->temperature());
	o.insert("hum", weather->humidity());
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

}

QString BangleJSDevice::information(Info i) const
{
    qDebug() << Q_FUNC_INFO;

    if (i == INFO_BATTERY) {
        return QString::number(m_infoBatteryLevel);
    }
    if (i == INFO_SWVER) {
        return m_firmwareVersion;
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
        if (json.keys().contains("fw1")) {
            m_firmwareVersion = json.value("fw1").toString();
        }
        if (json.keys().contains("fw2")) {
            m_firmwareVersion = json.value("fw2").toString();
        }
        emit informationChanged(INFO_SWVER, m_firmwareVersion);
    } else if (t == "status") {
        m_infoBatteryLevel = json.value("bat").toInt();
        emit informationChanged(INFO_BATTERY, QString::number(m_infoBatteryLevel));
    } else if (t == "findPhone") {


    } else if (t == "music") {


    } else if (t == "call") {


    } else if (t == "notify") {


    } else if (t == "act") {


    } else {

    }
#if 0
    case "status": {
        if (json.has("volt"))
            gbDevice.setBatteryVoltage((float)json.getDouble("volt"));
        gbDevice.sendDeviceUpdateIntent(context);
    } break;
    case "findPhone": {
        boolean start = json.has("n") && json.getBoolean("n");
        GBDeviceEventFindPhone deviceEventFindPhone = new GBDeviceEventFindPhone();
        deviceEventFindPhone.event = start ? GBDeviceEventFindPhone.Event.START : GBDeviceEventFindPhone.Event.STOP;
        evaluateGBDeviceEvent(deviceEventFindPhone);
    } break;
    case "music": {
        GBDeviceEventMusicControl deviceEventMusicControl = new GBDeviceEventMusicControl();
        deviceEventMusicControl.event = GBDeviceEventMusicControl.Event.valueOf(json.getString("n").toUpperCase());
        evaluateGBDeviceEvent(deviceEventMusicControl);
    } break;
    case "call": {
        GBDeviceEventCallControl deviceEventCallControl = new GBDeviceEventCallControl();
        deviceEventCallControl.event = GBDeviceEventCallControl.Event.valueOf(json.getString("n").toUpperCase());
        evaluateGBDeviceEvent(deviceEventCallControl);
    } break;
    case "notify" : {
        GBDeviceEventNotificationControl deviceEvtNotificationControl = new GBDeviceEventNotificationControl();
        // .title appears unused
        deviceEvtNotificationControl.event = GBDeviceEventNotificationControl.Event.valueOf(json.getString("n").toUpperCase());
        if (json.has("id"))
            deviceEvtNotificationControl.handle = json.getInt("id");
        if (json.has("tel"))
            deviceEvtNotificationControl.phoneNumber = json.getString("tel");
        if (json.has("msg"))
            deviceEvtNotificationControl.reply = json.getString("msg");
        evaluateGBDeviceEvent(deviceEvtNotificationControl);
    } break;
    case "act": {
        BangleJSActivitySample sample = new BangleJSActivitySample();
        sample.setTimestamp((int) (GregorianCalendar.getInstance().getTimeInMillis() / 1000L));
        int hrm = 0;
        int steps = 0;
        if (json.has("hrm")) hrm = json.getInt("hrm");
        if (json.has("stp")) steps = json.getInt("stp");
        int activity = ActivityKind.TYPE_UNKNOWN;
        if (json.has("act")) {
            String actName = "TYPE_" + json.getString("act").toUpperCase();
            try {
                Field f = ActivityKind.class.getField(actName);
                try {
                    activity = f.getInt(null);
                } catch (IllegalAccessException e) {
                    LOG.info("JSON activity '"+actName+"' not readable");
                }
            } catch (NoSuchFieldException e) {
                LOG.info("JSON activity '"+actName+"' not found");
            }
        }
        sample.setRawKind(activity);
        sample.setHeartRate(hrm);
        sample.setSteps(steps);
        try (DBHandler dbHandler = GBApplication.acquireDB()) {
            Long userId = getUser(dbHandler.getDaoSession()).getId();
            Long deviceId = DBHelper.getDevice(getDevice(), dbHandler.getDaoSession()).getId();
            BangleJSSampleProvider provider = new BangleJSSampleProvider(getDevice(), dbHandler.getDaoSession());
            sample.setDeviceId(deviceId);
            sample.setUserId(userId);
            provider.addGBActivitySample(sample);
        } catch (Exception ex) {
            LOG.warn("Error saving activity: " + ex.getLocalizedMessage());
        }
    } break;
#endif
}
