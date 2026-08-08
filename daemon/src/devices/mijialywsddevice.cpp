#include "mijialywsddevice.h"
#include "services/mijialywsdservice.h"
#include "services/deviceinfoservice.h"
#include "services/batteryservice.h"
#include <QDebug>

MijiaLywsdDevice::MijiaLywsdDevice(const QString &pairedName, QObject *parent)
    : AbstractDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO;
    m_needsAuth = false;
    m_autoreconnect = true;
    connect(this, &QBLEDevice::propertiesChanged, this, &MijiaLywsdDevice::onPropertiesChanged, Qt::UniqueConnection);
}

void MijiaLywsdDevice::pair()
{
    qDebug() << Q_FUNC_INFO;
    m_pairing = true;
    setConnectionState("pairing");

    connectToDevice();
}

Amazfish::Features MijiaLywsdDevice::supportedFeatures() const
{
    return Amazfish::Feature::FEATURE_NONE;
}

Amazfish::DataTypes MijiaLywsdDevice::supportedDataTypes() const
{
    return Amazfish::DataType::TYPE_TEMPERATURE; 
}

QString MijiaLywsdDevice::deviceType() const
{
    return "Xiaomi Thermometer";
}

AbstractFirmwareInfo *MijiaLywsdDevice::firmwareInfo(const QByteArray &bytes, const QString &fileName)
{
    Q_UNUSED(bytes)
    Q_UNUSED(fileName)
    return nullptr;
}

void MijiaLywsdDevice::refreshInformation()
{
    qDebug() << Q_FUNC_INFO;
    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (info) {
        info->refreshInformation();
    }

    MijiaLywsdService *lywsd = qobject_cast<MijiaLywsdService*>(service(MijiaLywsdService::UUID_SERVICE_MIJIA_LYWSD));
    if (lywsd) {
        lywsd->refreshInformation();
    }
}

void MijiaLywsdDevice::sendAlert(const Amazfish::WatchNotification &notification)
{
    Q_UNUSED(notification)
}

void MijiaLywsdDevice::incomingCall(const QString &caller)
{
    Q_UNUSED(caller)
}

void MijiaLywsdDevice::incomingCallEnded()
{
}


QBLEService* MijiaLywsdDevice::drv_createService(const QString &uuid, const QString &path)
{
    qDebug() << Q_FUNC_INFO << uuid;

    if (uuid == MijiaLywsdService::UUID_SERVICE_MIJIA_LYWSD && !service(MijiaLywsdService::UUID_SERVICE_MIJIA_LYWSD)) {
        return new MijiaLywsdService(path, this);
    } else if (uuid == DeviceInfoService::UUID_SERVICE_DEVICEINFO && !service(DeviceInfoService::UUID_SERVICE_DEVICEINFO)) {
        return new DeviceInfoService(path, this);
    } else if (uuid == BatteryService::UUID_SERVICE_BATTERY && !service(BatteryService::UUID_SERVICE_BATTERY)) {
        return new BatteryService(path, this);
    }

    return nullptr;
}

void MijiaLywsdDevice::onPropertiesChanged(QString interface, QVariantMap map, QStringList list)
{
    qDebug() << Q_FUNC_INFO << interface << map << list;

    if (interface == "org.bluez.Device1") {
        if (deviceProperty("ServicesResolved").toBool()) {
            initialise();
        }
        
        if (map.contains("Connected")) {
            bool connected = map["Connected"].toBool();
            if (connected) {
                setConnectionState("connected");
            } else {
                setConnectionState("disconnected");
            }
        }
    }
}

void MijiaLywsdDevice::initialise() {
    setConnectionState("connected");
    parseServices();

    MijiaLywsdService *lywsd = qobject_cast<MijiaLywsdService*>(service(MijiaLywsdService::UUID_SERVICE_MIJIA_LYWSD));
    if (lywsd) {
        connect(lywsd, &MijiaLywsdService::informationChanged, this, &AbstractDevice::informationChanged, Qt::UniqueConnection);
        lywsd->enableNotifications();
        lywsd->refreshInformation();
    }

    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (info) {
        connect(info, &DeviceInfoService::informationChanged, this, &AbstractDevice::informationChanged, Qt::UniqueConnection);
    }

    setConnectionState("authenticated");
}
