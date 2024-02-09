#include "bipdevice.h"
#include "bipfirmwareinfo.h"
#include "updatefirmwareoperation.h"

#include <QtXml/QtXml>

BipDevice::BipDevice(const QString &pairedName, QObject *parent) : HuamiDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO;
}


int BipDevice::supportedFeatures() const
{
    return FEATURE_HRM |
            FEATURE_WEATHER |
            FEATURE_ACTIVITY |
            FEATURE_STEPS |
            FEATURE_ALARMS |
            FEATURE_ALERT |
            FEATURE_BUTTON_ACTION;
}

QString BipDevice::deviceType() const
{
    return "amazfitbip";
}
    
void BipDevice::parseServices()
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

            if (uuid == AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION && !service(AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION)) {
                addService(AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION, new AlertNotificationService(path, this));
            } else if (uuid == DeviceInfoService::UUID_SERVICE_DEVICEINFO  && !service(DeviceInfoService::UUID_SERVICE_DEVICEINFO)) {
                addService(DeviceInfoService::UUID_SERVICE_DEVICEINFO, new DeviceInfoService(path, this));
            } else if (uuid == HRMService::UUID_SERVICE_HRM && !service(HRMService::UUID_SERVICE_HRM)) {
                addService(HRMService::UUID_SERVICE_HRM, new HRMService(path, this));
            } else if (uuid == MiBandService::UUID_SERVICE_MIBAND && !service(MiBandService::UUID_SERVICE_MIBAND)) {
                addService(MiBandService::UUID_SERVICE_MIBAND, new MiBandService(path, this));
            } else if (uuid == MiBand2Service::UUID_SERVICE_MIBAND2 && !service(MiBand2Service::UUID_SERVICE_MIBAND2)) {
                addService(MiBand2Service::UUID_SERVICE_MIBAND2, new MiBand2Service(path, 0x08, 0x00, false, this));
            } else if (uuid == BipFirmwareService::UUID_SERVICE_FIRMWARE && !service(BipFirmwareService::UUID_SERVICE_FIRMWARE)) {
                addService(BipFirmwareService::UUID_SERVICE_FIRMWARE, new BipFirmwareService(path, this));
            } else if ( !service(uuid)) {
                addService(uuid, new QBLEService(uuid, path, this));
            }
        }
    }
}

void BipDevice::sendWeather(CurrentWeather *weather)
{
    sendWeatherHuami(weather, (softwareRevision() > "V0.0.8.74"));
}

AbstractFirmwareInfo *BipDevice::firmwareInfo(const QByteArray &bytes)
{
    return new BipFirmwareInfo(bytes);
}

void BipDevice::initialise()
{
    qDebug() << Q_FUNC_INFO;
    setConnectionState("connected");
    parseServices();

    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_CONFIGURATION);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_DEVICE_EVENT);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_REALTIME_STEPS);

        connect(mi, &MiBandService::message, this, &HuamiDevice::message, Qt::UniqueConnection);
        connect(mi, &QBLEService::operationRunningChanged, this, &QBLEDevice::operationRunningChanged, Qt::UniqueConnection);
        connect(mi, &MiBandService::buttonPressed, this, &BipDevice::handleButtonPressed, Qt::UniqueConnection);
        connect(mi, &MiBandService::informationChanged, this, &BipDevice::informationChanged, Qt::UniqueConnection);
        connect(mi, &MiBandService::serviceEvent, this, &BipDevice::serviceEvent, Qt::UniqueConnection);
    }

    MiBand2Service *mi2 = qobject_cast<MiBand2Service*>(service(MiBand2Service::UUID_SERVICE_MIBAND2));
    if (mi2) {
        qDebug() << "Got MiBand2 service" << m_pairing << m_needsAuth;
        connect(mi2, &MiBand2Service::authenticated, this, &HuamiDevice::authenticated, Qt::UniqueConnection);
        connect(mi2, &QBLEService::operationRunningChanged, this, &QBLEDevice::operationRunningChanged, Qt::UniqueConnection);

        mi2->enableNotification(MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH);
        mi2->initialise(m_needsAuth);
    }

    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(BipFirmwareService::UUID_SERVICE_FIRMWARE));
    if (fw) {
        connect(fw, &BipFirmwareService::message, this, &BipDevice::message, Qt::UniqueConnection);
        connect(fw, &BipFirmwareService::downloadProgress, this, &BipDevice::downloadProgress, Qt::UniqueConnection);
        connect(fw, &QBLEService::operationRunningChanged, this, &QBLEDevice::operationRunningChanged, Qt::UniqueConnection);
    }

    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (info) {
        connect(info, &DeviceInfoService::informationChanged, this, &BipDevice::informationChanged, Qt::UniqueConnection);
    }

    HRMService *hrm = qobject_cast<HRMService*>(service(HRMService::UUID_SERVICE_HRM));
    if (hrm) {
        connect(hrm, &HRMService::informationChanged, this, &BipDevice::informationChanged, Qt::UniqueConnection);
    }
}

void BipDevice::navigationRunning(bool running)
{
    QString msg;
    if (running) {
        msg = tr("Navigation Started");
    } else {
        msg = tr("Navigation Stopped");
    }
    sendAlert("navigation", msg, "");
}

void BipDevice::navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress)
{
    Q_UNUSED(flag)
    sendAlert("navigation", tr("Progress") + ":" + QString::number(progress), narrative + "\n" + manDist);
}

QStringList BipDevice::supportedDisplayItems() const
{
    QStringList items;

    items << "status";
    items << "activity";
    items << "weather";
    items << "alarm";
    items << "timer";
    items << "compass";
    items << "settings";
    items << "alipay";

    return items;
}

void BipDevice::applyDeviceSetting(AbstractDevice::Settings s)
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (!mi) {
        return;
    }
    QMap<QString, uint8_t> keyPosMap;
    keyPosMap["status"] = 1;
    keyPosMap["activity"] = 2;
    keyPosMap["weather"] = 3;
    keyPosMap["alarm"] = 4;
    keyPosMap["timer"] = 5;
    keyPosMap["compass"] = 6;
    keyPosMap["settings"] = 7;
    keyPosMap["alipay"] = 8;

    if (s == SETTING_DEVICE_DISPLAY_ITEMS) {
        mi->setDisplayItemsOld(keyPosMap);
    } else {
        HuamiDevice::applyDeviceSetting(s);
    }
}
