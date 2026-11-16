#include "deviceinfoservice.h"
#include "pebbleservice.h"
#include "pebbleppogattservice.h"
#include "pebbledevice.h"
#include "amazfishconfig.h"
#include "qble/qblelocalapplication.h"

#include <QDateTime>
#include <QTimeZone>
#include <QtEndian>

static const QString GATT_APP_PATH = QStringLiteral("/uk/co/piggz/amazfish/pebble");

PebbleDevice::PebbleDevice(const QString &pairedName, QObject *parent ) : AbstractDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO << pairedName;
    connect(this, &QBLEDevice::propertiesChanged, this, &PebbleDevice::onPropertiesChanged, Qt::UniqueConnection);

    // The watch is the GATT client of PPoGATT, so the service must exist before the watch connects
    m_gattApp = new QBLELocalApplication(GATT_APP_PATH, this);
    m_ppogatt = new PebblePPoGATTService(QDBusConnection::systemBus(), GATT_APP_PATH, m_gattApp);
    m_gattApp->addService(m_ppogatt);
    m_gattApp->addService(new PebbleFakeService(QDBusConnection::systemBus(), GATT_APP_PATH, m_gattApp));
    m_gattApp->registerWithAdapter(AmazfishConfig::instance()->localAdapter());

    connect(m_ppogatt, &PebblePPoGATTService::sessionOpened, this, &PebbleDevice::onSessionOpened);
    connect(m_ppogatt, &PebblePPoGATTService::sessionClosed, this, [this]() { setConnectionState("connected"); });
    connect(m_ppogatt, &PebblePPoGATTService::messageReceived, this, &PebbleDevice::onMessageReceived);
}

PebbleDevice::~PebbleDevice()
{
    m_gattApp->unregisterFromAdapter(AmazfishConfig::instance()->localAdapter());
}

Amazfish::Features PebbleDevice::supportedFeatures() const
{
    return Amazfish::Feature::FEATURE_NONE;
}

Amazfish::DataTypes PebbleDevice::supportedDataTypes() const
{
    return Amazfish::DataType::TYPE_NONE;
}

QString PebbleDevice::deviceType() const 
{
    return "pebble";
}

void PebbleDevice::sendAlert(const Amazfish::WatchNotification &notification) {
    qDebug() << Q_FUNC_INFO;
}

void PebbleDevice::incomingCall(const QString &caller) {
    qDebug() << Q_FUNC_INFO;
}

void PebbleDevice::incomingCallEnded() {
    qDebug() << Q_FUNC_INFO;
}

AbstractFirmwareInfo* PebbleDevice::firmwareInfo(const QByteArray &bytes, const QString &path) {
    qDebug() << Q_FUNC_INFO;
    return nullptr;
}

//    virtual AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;
//    virtual void setMusicStatus(bool playing, const QString &title, const QString &artist, const QString &album, int duration = 0, int position = 0) override;
//    virtual void requestScreenshot() override;


void PebbleDevice::onPropertiesChanged(const QString &interface, QVariantMap map, const QStringList &list)
{
    qDebug() << Q_FUNC_INFO << interface << map << list;

    if (interface == "org.bluez.Device1") {
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

QBLEService *PebbleDevice::drv_createService(const QString &uuid, const QString &path)
{
    if (uuid == DeviceInfoService::UUID_SERVICE_DEVICEINFO  && !service(DeviceInfoService::UUID_SERVICE_DEVICEINFO)) {
        return new DeviceInfoService(path, this);
    } else if (uuid == PebbleService::UUID_SERVICE_PEBBLE && !service(PebbleService::UUID_SERVICE_PEBBLE)) {
        return new PebbleService(path, this);
    }

    return nullptr;
}

void PebbleDevice::initialise()
{
    qDebug() << Q_FUNC_INFO;
    // Called again on every property change of the connected device
    if (m_ppogatt->isOpen()) {
        return;
    }

    setConnectionState("connected");
    parseServices();

    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (info) {
        connect(info, &DeviceInfoService::informationChanged, this, &PebbleDevice::informationChanged, Qt::UniqueConnection);
    }

    // A paired watch opens PPoGATT by itself
    PebbleService *pebble = qobject_cast<PebbleService*>(service(PebbleService::UUID_SERVICE_PEBBLE));
    if (pebble && !deviceProperty("Paired").toBool()) {
        pebble->triggerPairing();
    }
}

void PebbleDevice::onSessionOpened()
{
    qDebug() << Q_FUNC_INFO;
    setConnectionState("authenticated");
    setTime();
}

void PebbleDevice::onMessageReceived(quint16 endpoint, const QByteArray &payload)
{
    if (endpoint == EndpointPhoneVersion && payload.startsWith('\0')) {
        sendPhoneVersion();
    } else if (endpoint == EndpointTime && payload.size() >= 5 && payload.at(0) == 1) {
        quint32 time = qFromBigEndian<quint32>(reinterpret_cast<const uchar *>(payload.constData() + 1));
        qDebug() << Q_FUNC_INFO << "Watch time" << QDateTime::fromMSecsSinceEpoch(qint64(time) * 1000, Qt::UTC).toString(Qt::ISODate);
    } else {
        qDebug() << Q_FUNC_INFO << "Unhandled endpoint" << endpoint << payload.toHex();
    }
}

void PebbleDevice::sendPhoneVersion()
{
    // Same answer as Gadgetbridge and RockWork: an Android phone, no extra capabilities
    QByteArray payload(25, '\0');
    uchar *data = reinterpret_cast<uchar *>(payload.data());
    data[0] = 1;
    qToBigEndian<quint32>(0xffffffff, data + 1);
    qToBigEndian<quint32>(2, data + 9); // platform: Android
    data[13] = 2;                       // response version
    data[14] = 4;                       // app version 4.4.2
    data[15] = 4;
    data[16] = 2;
    m_ppogatt->sendMessage(EndpointPhoneVersion, payload);
}

void PebbleDevice::setTime()
{
    QDateTime now = QDateTime::currentDateTime();
    QByteArray zone = QTimeZone::systemTimeZoneId();

    QByteArray payload(7, '\0');
    uchar *data = reinterpret_cast<uchar *>(payload.data());
    data[0] = 3; // set UTC time
    qToBigEndian<quint32>(now.toMSecsSinceEpoch() / 1000, data + 1);
    qToBigEndian<qint16>(now.offsetFromUtc() / 60, data + 5);
    payload.append(static_cast<char>(zone.size()));
    payload.append(zone);
    m_ppogatt->sendMessage(EndpointTime, payload);

    // Read the time back for the log
    m_ppogatt->sendMessage(EndpointTime, QByteArray(1, '\0'));
}

void PebbleDevice::applyDeviceSetting(Amazfish::Settings s)
{
    if (s == Amazfish::Settings::SETTING_DEVICE_TIME && m_ppogatt->isOpen()) {
        setTime();
    }
}

void PebbleDevice::refreshInformation()
{
    qDebug() << Q_FUNC_INFO;
    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (info) {
        info->refreshInformation();
    }
}

QString PebbleDevice::information(Amazfish::Info i) const
{
    qDebug() << Q_FUNC_INFO << i;

    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (!info) {
        qWarning() << "Device info service doesn't exists";
        return QString();
    }

    switch(i) {
    case Amazfish::Info::INFO_MODEL:
        return info->modelNumber();
        break;
    case Amazfish::Info::INFO_SERIAL:
        return info->serialNumber();
        break;
    case Amazfish::Info::INFO_FW_REVISION:
        return info->fwRevision();
        break;
    case Amazfish::Info::INFO_HWVER:
        return info->hardwareRevision();
        break;
    case Amazfish::Info::INFO_SWVER:
        return info->softwareRevision();
        break;
    case Amazfish::Info::INFO_MANUFACTURER:
        return info->manufacturerName();
        break;
    }
    return QString();
}
