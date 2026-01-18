#include <QtXml/QtXml>

#include "dk08device.h"
#include "dk08nusservice.h"
#include "dk08wechatservice.h"

DK08Device::DK08Device(const QString &pairedName, QObject *parent ) : AbstractDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO << pairedName;
    connect(this, &QBLEDevice::propertiesChanged, this, &DK08Device::onPropertiesChanged, Qt::UniqueConnection);
}

Amazfish::Features DK08Device::supportedFeatures() const
{
    return Amazfish::Feature::FEATURE_STEPS | Amazfish::Feature::FEATURE_HRM  | Amazfish::Feature::FEATURE_ALERT |
            Amazfish::Feature::FEATURE_ALARMS | Amazfish::Feature::FEATURE_WEATHER;
}

Amazfish::DataTypes DK08Device::supportedDataTypes() const
{
    return Amazfish::DataType::TYPE_HEART_RATE;
}

QString DK08Device::deviceType() const 
{
    return "dk08";
}

void DK08Device::pair()
{
    qDebug() << Q_FUNC_INFO;

    m_needsAuth = true;
    m_pairing = true;
    m_autoreconnect = true;
    //disconnectFromDevice();
    setConnectionState("pairing");
    emit connectionStateChanged();

    QBLEDevice::pair();
}

void DK08Device::sendAlert(const Amazfish::WatchNotification &notification) {
    qDebug() << Q_FUNC_INFO;
    DK08NUSService *nus = qobject_cast<DK08NUSService*>(service(DK08NUSService::UUID_SERVICE_NUS));
    m_alert_id = (m_alert_id + 1) % 32;
    if (nus) {

        std::map<QString, DK08NUSService::AlertType> AppToType =
            {
             /* Facebook */
             {"facebook", DK08NUSService::AlertType::Facebook},
             /* Facebook Messenger */
             {"messenger", DK08NUSService::AlertType::Facebook},
             /* Instagram */
             {"instagram", DK08NUSService::AlertType::Instagram},
             /* Twitter Clients */
             {"piepmatz", DK08NUSService::AlertType::Twitter},
             {"tweetian", DK08NUSService::AlertType::Twitter},
             {"twitter", DK08NUSService::AlertType::Twitter},
             /* Skype */
             {"skype", DK08NUSService::AlertType::Skype},
             /* WhatsApp */
             {"whatsapp", DK08NUSService::AlertType::Whatsapp},
             };


        DK08NUSService::AlertType type = DK08NUSService::AlertType::SMSAlert;
        auto icon = AppToType.find(notification.appName);
        if ( icon != AppToType.end() ) {
            type = icon->second;
        }

        nus->setSMSAlertName(m_alert_id, type, notification.appName);
        nus->setSMSAlertContext(m_alert_id, type, notification.summary + " " + notification.body);
    }
}
void DK08Device::incomingCall(const QString &caller) {
    qDebug() << Q_FUNC_INFO;
    DK08NUSService *nus = qobject_cast<DK08NUSService*>(service(DK08NUSService::UUID_SERVICE_NUS));
    if (nus) {
        // nus->setInfoAlert(DK08NUSService::AlertType::PhoneAlert);
        nus->setPhoneAlert(caller);
    }
}

void DK08Device::incomingCallEnded() {
    qDebug() << Q_FUNC_INFO;
}

AbstractFirmwareInfo* DK08Device::firmwareInfo(const QByteArray &bytes, const QString &path) {
    qDebug() << Q_FUNC_INFO;
    return nullptr;
}

void DK08Device::onPropertiesChanged(QString interface, QVariantMap map, QStringList list)
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

void DK08Device::parseServices()
{

    qDebug() << Q_FUNC_INFO;

    QDBusInterface adapterIntro("org.bluez", devicePath(), "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus(), 0);
    QDBusReply<QString> xml = adapterIntro.call("Introspect");

    // qDebug() << "Resolved services...";
    // qDebug().noquote() << xml.value();

    QDomDocument doc;
    doc.setContent(xml.value());

    QDomNodeList nodes = doc.elementsByTagName("node");

    // qDebug() << nodes.count() << "nodes";

    for (int x = 0; x < nodes.count(); x++)
    {
        QDomElement node = nodes.at(x).toElement();
        QString nodeName = node.attribute("name");

        if (nodeName.startsWith("service")) {
            QString path = devicePath() + "/" + nodeName;

            QDBusInterface devInterface("org.bluez", path, "org.bluez.GattService1", QDBusConnection::systemBus(), 0);
            QString uuid = devInterface.property("UUID").toString();

            qDebug() << "Creating service for: " << uuid;

            if (uuid == DK08NUSService::UUID_SERVICE_NUS  && !service(DK08NUSService::UUID_SERVICE_NUS)) {
                addService(DK08NUSService::UUID_SERVICE_NUS, new DK08NUSService(path, this));
            } else if (uuid == DK08WechatService::UUID_SERVICE_WECHAT  && !service(DK08WechatService::UUID_SERVICE_WECHAT)) {
                addService(DK08WechatService::UUID_SERVICE_WECHAT, new DK08WechatService(path, this));
            } else if ( !service(uuid)) {
                addService(uuid, new QBLEService(uuid, path, this));
            }
        }
    }
    setConnectionState("authenticated");
}

void DK08Device::initialise()
{
    qDebug() << Q_FUNC_INFO;
    setConnectionState("connected");
    parseServices();

    DK08WechatService *wechat = qobject_cast<DK08WechatService*>(service(DK08WechatService::UUID_SERVICE_WECHAT));
    if (wechat) {
        connect(wechat, &DK08WechatService::informationChanged, this, &DK08Device::informationChanged, Qt::UniqueConnection);
        // wechat->setTime();
    }

    DK08NUSService *nus = qobject_cast<DK08NUSService*>(service(DK08NUSService::UUID_SERVICE_NUS));
    if (nus) {
        connect(nus, &DK08NUSService::informationChanged, this, &DK08Device::informationChanged, Qt::UniqueConnection);
        nus->setTime();
        nus->getBattery();
        // nus->test();
    }

}

void DK08Device::refreshInformation()
{
    qDebug() << Q_FUNC_INFO;
    DK08WechatService *wechat = qobject_cast<DK08WechatService*>(service(DK08WechatService::UUID_SERVICE_WECHAT));
    if (wechat) {
        wechat->refreshInformation();
    }

    DK08NUSService *nus = qobject_cast<DK08NUSService*>(service(DK08NUSService::UUID_SERVICE_NUS));
    if (nus) {
        nus->refreshInformation();
    }

}

QString DK08Device::information(AbstractDevice::Info i) const
{
    DK08WechatService *wechat = qobject_cast<DK08WechatService*>(service(DK08WechatService::UUID_SERVICE_WECHAT));
    if (!wechat) {
        qWarning() << "Wechat service doesn't exists";
        return QString();
    }

    switch(i) {
    case AbstractDevice::INFO_STEPS:
        return QString::number(wechat->steps(), 10);
    // case AbstractDevice::INFO_BATTERY:
    // case INFO_MODEL:
    //     return info->modelNumber();
    //     break;
    // case INFO_SERIAL:
    //     return info->serialNumber();
    //     break;
    // case INFO_FW_REVISION:
    //     return info->fwRevision();
    //     break;
    // case INFO_HWVER:
    //     return info->hardwareRevision();
    //     break;
    // case INFO_MANUFACTURER:
    //     return info->manufacturerName();
    //     break;

    default:
        qDebug() << Q_FUNC_INFO << i;
    }
    // }
    return QString();
}

void DK08Device::downloadActivityData() {
    DK08NUSService *nus = qobject_cast<DK08NUSService*>(service(DK08NUSService::UUID_SERVICE_NUS));
    if (nus) {
        nus->getSportInfo(DK08NUSService::InfoType::Current);
        nus->getSleepStats();
        nus->getStepSection(DK08NUSService::InfoType::History);
        nus->getSportInfo(DK08NUSService::InfoType::History);
        nus->getSportInfo(DK08NUSService::InfoType::HistoryNum);
        nus->getSleep(DK08NUSService::InfoType::Current);
        nus->getSleep(DK08NUSService::InfoType::History);
        nus->getHrInfo(DK08NUSService::InfoType::History);

    }

}

void DK08Device::applyDeviceSetting(AbstractDevice::Settings settings) {
    DK08NUSService *nus = qobject_cast<DK08NUSService*>(service(DK08NUSService::UUID_SERVICE_NUS));
    if (!nus) {
        qWarning() << "NUSService not available";
        return;
    }
    switch(settings) {
    case AbstractDevice::Settings::SETTING_ALARMS:
        nus->setAlarms();
        break;
    case AbstractDevice::Settings::SETTING_USER_PROFILE:
        nus->setUserInfo();
        break;
    case AbstractDevice::Settings::SETTING_USER_GOAL:
        nus->setSportTarget();
        break;
    case AbstractDevice::Settings::SETTING_DEVICE_UNIT:
        nus->setUnit();
        break;
    case AbstractDevice::Settings::SETTING_DEVICE_TIME:
        nus->setTimeFormat();
        break;
    case AbstractDevice::Settings::SETTING_USER_DISPLAY_ON_LIFT:
        nus->setPalming();
        break;
    default:
        qDebug() << Q_FUNC_INFO << "Not implemented " << settings;
        break;
    }

}


void DK08Device::sendWeather(CurrentWeather *weather)
{
    DK08NUSService *nus = qobject_cast<DK08NUSService*>(service(DK08NUSService::UUID_SERVICE_NUS));
    if (!nus) {
        qWarning() << "NUSService not available";
        return;
    }
    nus->sendWeather(weather);
}
