#include "dk08device.h"
#include "amazfishconfig.h"
#include "database.h"
#include "dk08nusservice.h"
#include "dk08wechatservice.h"

DK08Device::DK08Device(const QString &pairedName, QObject *parent ) : AbstractDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO << pairedName;
    connect(this, &QBLEDevice::propertiesChanged, this, &DK08Device::onPropertiesChanged, Qt::UniqueConnection);
}

Amazfish::Features DK08Device::supportedFeatures() const
{
    return Amazfish::Feature::FEATURE_STEPS
        | Amazfish::Feature::FEATURE_HRM
        | Amazfish::Feature::FEATURE_ALERT
        | Amazfish::Feature::FEATURE_ALARMS
        | Amazfish::Feature::FEATURE_WEATHER
        | Amazfish::Feature::FEATURE_ACTIVITY;
}

Amazfish::DataTypes DK08Device::supportedDataTypes() const
{
    return Amazfish::DataType::TYPE_HEART_RATE | Amazfish::DataType::TYPE_ACTIVITY | Amazfish::DataType::TYPE_SLEEP;
}

QString DK08Device::deviceType() const 
{
    return "dk08";
}

void DK08Device::sendAlert(const Amazfish::WatchNotification &notification) {
    qDebug() << Q_FUNC_INFO;
    if (m_operationRunning) {
        qDebug() << "Operation is running, alert not sent";
        return;
    }
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

void DK08Device::fetchData(Amazfish::DataTypes dataTypes)
{
    if (m_operationRunning) {
        emit message(tr("An operation is currently running, please try later"));
        return;
    }
    if (connectionState() != "authenticated") {
        qWarning() << Q_FUNC_INFO << "Not connected:" << connectionState();
        return;
    }
    if (dataTypes & Amazfish::DataType::TYPE_ACTIVITY) {
        downloadActivityData();
    }
}

AbstractFirmwareInfo* DK08Device::firmwareInfo(const QByteArray &bytes, const QString &path) {
    qDebug() << Q_FUNC_INFO;
    return nullptr;
}

void DK08Device::onPropertiesChanged(QString interface, QVariantMap map, QStringList list)
{
    qDebug() << Q_FUNC_INFO << interface << map << list;

    if (interface == "org.bluez.Device1") {
        if (deviceProperty("ServicesResolved").toBool() ) {
            initialise();
        }
        if (map.contains("Connected")) {
            bool value = map["Connected"].toBool();

            if (!value) {
                DK08NUSService* nus = qobject_cast<DK08NUSService*>(service(DK08NUSService::UUID_SERVICE_NUS));
                if (nus) {
                    nus->cancelHistory();
                }
                setOperationRunning(false);
                setConnectionState("disconnected");
            } else {
                setConnectionState("connected");
            }
        }
    }

}

QBLEService *DK08Device::drv_createService(const QString &uuid, const QString &path)
{
    if (uuid == DK08NUSService::UUID_SERVICE_NUS  && !service(DK08NUSService::UUID_SERVICE_NUS)) {
        return new DK08NUSService(path, this);
    } else if (uuid == DK08WechatService::UUID_SERVICE_WECHAT  && !service(DK08WechatService::UUID_SERVICE_WECHAT)) {
        return new DK08WechatService(path, this);
    }

    return nullptr;
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
        connect(nus, &DK08NUSService::historyDownloaded, this, &DK08Device::saveActivitySamples, Qt::UniqueConnection);
        connect(nus, &DK08NUSService::workoutDownloaded, this, &DK08Device::saveWorkout, Qt::UniqueConnection);
        nus->setTime();
        nus->getBattery();
        // nus->test();
    }

    setConnectionState("authenticated");
}

void DK08Device::refreshInformation()
{
    qDebug() << Q_FUNC_INFO;
    if (m_operationRunning) {
        return;
    }
    DK08WechatService *wechat = qobject_cast<DK08WechatService*>(service(DK08WechatService::UUID_SERVICE_WECHAT));
    if (wechat) {
        wechat->refreshInformation();
    }

    DK08NUSService *nus = qobject_cast<DK08NUSService*>(service(DK08NUSService::UUID_SERVICE_NUS));
    if (nus) {
        nus->refreshInformation();
    }

}

QString DK08Device::information(Amazfish::Info i) const
{
    DK08WechatService *wechat = qobject_cast<DK08WechatService*>(service(DK08WechatService::UUID_SERVICE_WECHAT));
    if (!wechat) {
        qWarning() << "Wechat service doesn't exists";
        return QString();
    }

    switch(i) {
    case Amazfish::Info::INFO_STEPS:
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
        emit message(tr("Downloading activity data"));
        setOperationRunning(true);
        nus->getSportInfo(DK08NUSService::InfoType::Current);
        nus->getSleepStats();
        nus->downloadHistory();
    }

}

void DK08Device::applyDeviceSetting(Amazfish::Settings settings) {
    DK08NUSService *nus = qobject_cast<DK08NUSService*>(service(DK08NUSService::UUID_SERVICE_NUS));
    if (!nus) {
        qWarning() << "NUSService not available";
        return;
    }
    if (m_operationRunning) {
        qDebug() << Q_FUNC_INFO << "Operation is running, setting not applied" << settings;
        return;
    }
    switch(settings) {
    case Amazfish::Settings::SETTING_ALARMS:
        nus->setAlarms();
        break;
    case Amazfish::Settings::SETTING_USER_PROFILE:
        nus->setUserInfo();
        break;
    case Amazfish::Settings::SETTING_USER_GOAL:
        nus->setSportTarget();
        break;
    case Amazfish::Settings::SETTING_DEVICE_UNIT:
        nus->setUnit();
        break;
    case Amazfish::Settings::SETTING_DEVICE_TIME:
        nus->setTimeFormat();
        break;
    case Amazfish::Settings::SETTING_USER_DISPLAY_ON_LIFT:
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
    if (m_operationRunning) {
        return;
    }
    nus->sendWeather(weather);
}

void DK08Device::saveActivitySamples(const QList<ActivitySample>& samples, bool complete)
{
    setOperationRunning(false);
    if (!Database::storeActivitySamples(samples)) {
        return;
    }
    qDebug() << Q_FUNC_INFO << "Saved samples:" << samples.count();
    if (complete) {
        emit message(tr("Activity data downloaded"));
    } else {
        emit message(tr("Activity data download was interrupted"));
    }
}

bool DK08Device::operationRunning()
{
    return m_operationRunning;
}

void DK08Device::setOperationRunning(bool running)
{
    if (m_operationRunning == running) {
        return;
    }
    m_operationRunning = running;
    emit operationRunningChanged();
}

void DK08Device::saveWorkout(ActivitySummary summary)
{
    summary.setProfileId(qHash(AmazfishConfig::instance()->profileName()));
    summary.setDeviceId(qHash(AmazfishConfig::instance()->pairedAddress()));
    if (!Database::storeActivitySummary(summary)) {
        qWarning() << Q_FUNC_INFO << "Error saving workout" << summary.name();
    }
}
