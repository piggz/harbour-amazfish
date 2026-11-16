#include "demodevice.h"
#include "activitykind.h"
#include "amazfishconfig.h"
#include "database.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QtMath>

// Sleep phases in mi_band_activity.raw_kind, as the UI reads them
static const int LIGHT_SLEEP = 9;
static const int DEEP_SLEEP = 11;

// A GPX 1.1 track for the sports page: a loop around the center, one point per minute.
static QString demoGpx(const QDateTime& start, int minutes, double latitude, double longitude, double radius)
{
    QString gpx = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                 "<gpx version=\"1.1\" creator=\"Amazfish demo\"><trk><trkseg>\n");
    for (int i = 0; i <= minutes; i++) {
        const double angle = 2 * M_PI * i / minutes;
        // At 50 degrees north, one degree of longitude is about 1.5 times shorter than one degree of latitude.
        const double pointLatitude = latitude + radius * qSin(angle);
        const double pointLongitude = longitude + radius * 1.5 * qCos(angle);
        gpx += QStringLiteral("<trkpt lat=\"%1\" lon=\"%2\"><ele>200</ele><time>%3</time></trkpt>\n")
                   .arg(pointLatitude, 0, 'f', 6)
                   .arg(pointLongitude, 0, 'f', 6)
                   .arg(start.addSecs(i * 60).toUTC().toString(Qt::ISODate));
    }
    gpx += QStringLiteral("</trkseg></trk></gpx>\n");
    return gpx;
}

DemoDevice::DemoDevice(const QString& pairedName, QObject* parent)
    : AbstractDevice(pairedName, parent)
{
    // DeviceInterface opens the database before it creates the device.
    QSqlQuery query;
    if (query.exec("SELECT count(*) FROM mi_band_activity") && query.next() && query.value(0).toInt() == 0) {
        addSampleData();
    }
}

Amazfish::Features DemoDevice::supportedFeatures() const
{
    return Amazfish::Feature::FEATURE_HRM | Amazfish::Feature::FEATURE_WEATHER | Amazfish::Feature::FEATURE_ACTIVITY
        | Amazfish::Feature::FEATURE_STEPS | Amazfish::Feature::FEATURE_ALARMS | Amazfish::Feature::FEATURE_ALERT
        | Amazfish::Feature::FEATURE_EVENT_REMINDER | Amazfish::Feature::FEATURE_MUSIC_CONTROL
        | Amazfish::Feature::FEATURE_BUTTON_ACTION | Amazfish::Feature::FEATURE_SCREENSHOT
        | Amazfish::Feature::FEATURE_FILE_INSTALL | Amazfish::Feature::FEATURE_SPO2;
}

Amazfish::DataTypes DemoDevice::supportedDataTypes() const
{
    return Amazfish::DataType::TYPE_DEBUGLOG | Amazfish::DataType::TYPE_ACTIVITY | Amazfish::DataType::TYPE_GPS_TRACK
        | Amazfish::DataType::TYPE_STRESS | Amazfish::DataType::TYPE_SPO2 | Amazfish::DataType::TYPE_PAI
        | Amazfish::DataType::TYPE_HEART_RATE | Amazfish::DataType::TYPE_SLEEP_RESPIRATORY_RATE
        | Amazfish::DataType::TYPE_TEMPERATURE | Amazfish::DataType::TYPE_SLEEP
        | Amazfish::DataType::TYPE_HUAMI_STATISTICS | Amazfish::DataType::TYPE_HRV;
}

QString DemoDevice::deviceType() const
{
    return "demo";
}

AbstractFirmwareInfo* DemoDevice::firmwareInfo(const QByteArray& bytes, const QString& fileName)
{
    Q_UNUSED(bytes);
    Q_UNUSED(fileName);
    return nullptr;
}

void DemoDevice::sendAlert(const Amazfish::WatchNotification& notification)
{
    qDebug() << Q_FUNC_INFO << notification.summary;
}

void DemoDevice::incomingCall(const QString& caller)
{
    qDebug() << Q_FUNC_INFO << caller;
}

void DemoDevice::incomingCallEnded()
{
    qDebug() << Q_FUNC_INFO;
}

void DemoDevice::connectToDevice()
{
    // No Bluetooth: the watch is there at once.
    setConnectionState("authenticated");
}

void DemoDevice::disconnectFromDevice()
{
    setConnectionState("disconnected");
}

void DemoDevice::fetchData(Amazfish::DataTypes dataTypes)
{
    Q_UNUSED(dataTypes);
    emit message(tr("Demo device: nothing to download"));
}

void DemoDevice::refreshInformation()
{
    // The UI updates its values from these signals.
    const Amazfish::Info infos[] = { Amazfish::Info::INFO_SWVER, Amazfish::Info::INFO_HWVER, Amazfish::Info::INFO_SERIAL,
        Amazfish::Info::INFO_GPSVER, Amazfish::Info::INFO_BATTERY, Amazfish::Info::INFO_STEPS,
        Amazfish::Info::INFO_HEARTRATE, Amazfish::Info::INFO_MODEL, Amazfish::Info::INFO_FW_REVISION,
        Amazfish::Info::INFO_MANUFACTURER, Amazfish::Info::INFO_SPO2 };
    for (Amazfish::Info i : infos) {
        emit informationChanged(i, information(i));
    }
}

QString DemoDevice::information(Amazfish::Info i) const
{
    switch (i) {
    case Amazfish::Info::INFO_SWVER:
    case Amazfish::Info::INFO_FW_REVISION:
        return "1.0.0";
    case Amazfish::Info::INFO_HWVER:
    case Amazfish::Info::INFO_GPSVER:
        return "1.0";
    case Amazfish::Info::INFO_SERIAL:
        return "DEMO-0001";
    case Amazfish::Info::INFO_MODEL:
        return "Demo watch";
    case Amazfish::Info::INFO_MANUFACTURER:
        return "Amazfish";
    case Amazfish::Info::INFO_BATTERY:
        return "80";
    case Amazfish::Info::INFO_STEPS:
        return "7342";
    case Amazfish::Info::INFO_HEARTRATE:
        return "72";
    case Amazfish::Info::INFO_SPO2:
        return "98";
    default:
        return QString();
    }
}

QStringList DemoDevice::supportedDisplayItems() const
{
    return { "status", "pai", "hr", "workout", "weather", "alarm", "music", "timer", "settings" };
}

QBLEService* DemoDevice::drv_createService(const QString& uuid, const QString& path)
{
    Q_UNUSED(uuid);
    Q_UNUSED(path);
    return nullptr;
}

bool DemoDevice::insert(const QString& table, const QStringList& fieldNames, const QList<QVariant>& values)
{
    QStringList placeholders;
    for (int i = 0; i < fieldNames.size(); i++) {
        placeholders << "?";
    }
    QSqlQuery query;
    query.prepare(QString("INSERT INTO %1 (%2) VALUES (%3)").arg(table, fieldNames.join(", "), placeholders.join(", ")));
    for (const QVariant& value : values) {
        // Dates as text in the format the UI compares, see Database::dateTimeToText()
        if (value.userType() == QMetaType::QDateTime) {
            query.addBindValue(Database::dateTimeToText(value.toDateTime()));
        } else if (value.userType() == QMetaType::QDate) {
            query.addBindValue(value.toDate().toString(Qt::ISODate));
        } else {
            query.addBindValue(value);
        }
    }
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << table << query.lastError().text();
        return false;
    }
    return true;
}

// One week of made-up data up to now. It has no randomness, so screenshots only change with the date.
void DemoDevice::addSampleData()
{
    qDebug() << Q_FUNC_INFO;

    auto config = AmazfishConfig::instance();
    uint userId = qHash(config->profileName());
    uint deviceId = qHash(config->pairedAddress());

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    const QDateTime now = QDateTime::currentDateTime();
    const QDate firstDay = now.date().addDays(-6);
    const QStringList activityFields = { "timestamp", "timestamp_dt", "device_id", "user_id", "raw_intensity", "steps",
        "raw_kind", "heartrate" };

    for (int day = 0; day < 7; ++day) {
        const QDate date = firstDay.addDays(day);

        // Activity per minute: sleep from 23:00 to 6:30, three walks, little movement otherwise
        for (int minute = 0; minute < 24 * 60; ++minute) {
            const QDateTime time(date, QTime(minute / 60, minute % 60));
            if (time > now) {
                break;
            }
            bool asleep = minute >= 23 * 60 || minute < 6 * 60 + 30;
            bool walking = (minute >= 7 * 60 + 30 && minute < 8 * 60) || (minute >= 12 * 60 && minute < 12 * 60 + 30)
                || (minute >= 17 * 60 + 30 && minute < 18 * 60 + day * 5);
            int kind = 1;
            int steps = 0;
            int intensity = 10 + minute % 10;
            int heartrate = 68 + minute % 9;
            if (asleep) {
                kind = ((minute + 60) / 45) % 3 == 0 ? DEEP_SLEEP : LIGHT_SLEEP;
                intensity = 2 + minute % 3;
                heartrate = 56 + minute % 4;
            } else if (walking) {
                steps = 95 + (minute % 7) * 3;
                intensity = 60 + minute % 20;
                heartrate = 95 + minute % 15;
            } else if (minute % 11 == 0) {
                steps = 12;
            }
            insert("mi_band_activity", activityFields,
                { time.toMSecsSinceEpoch() / 1000, time, deviceId, userId, intensity, steps, kind, heartrate });
        }

        // Measurements every hour
        for (int hour = 0; hour < 24; ++hour) {
            const QDateTime time(date, QTime(hour, 0));
            if (time > now) {
                break;
            }
            qint64 seconds = time.toMSecsSinceEpoch() / 1000;
            insert("info_log", { "timestamp", "timestamp_dt", "key", "value" },
                { seconds, time, int(Amazfish::Info::INFO_BATTERY), 95 - hour * 2 });
            insert("hrv", { "hrv_timestamp", "hrv_timestamp_dt", "hrv_value" }, { seconds, time, 35 + (hour * 7) % 25 });
            insert("temperature",
                { "temperature_timestamp", "temperature_timestamp_dt", "temperature_value", "temperature_location",
                    "temperature_type" },
                { seconds, time, 36.3 + (hour % 5) / 10.0, "wrist", "skin" });
            insert("stress", { "stress_timestamp", "stress_timestamp_dt", "stress_value", "stress_type" },
                { seconds, time, 20 + (hour * 13) % 45, 1 });
            if (hour % 2 == 0) {
                insert("spo2", { "spo2_timestamp", "spo2_timestamp_dt", "spo2_automatic", "spo2_value" },
                    { seconds, time, true, 95 + hour % 4 });
            }
        }

        // Once a day
        const QDateTime morning(date, QTime(6, 30));
        if (morning <= now) {
            insert("spo2sleep",
                { "spo2sleep_timestamp", "spo2sleep_timestamp_dt", "spo2sleep_value", "spo2sleep_duration",
                    "spo2sleep_high", "spo2sleep_low", "spo2sleep_quality" },
                { morning.toMSecsSinceEpoch() / 1000, morning, 96, 420, "99", "91", "good" });
        }
        const QDateTime noon(date, QTime(12, 0));
        if (noon <= now) {
            insert("stress", { "stress_timestamp", "stress_timestamp_dt", "stress_value", "stress_type" },
                { noon.toMSecsSinceEpoch() / 1000, noon, 35, 2 });
        }
        insert("pai",
            { "pai_date", "pai_low", "pai_moderate", "pai_high", "pai_low_time", "pai_moderate_time", "pai_high_time",
                "pai_today", "pai_total" },
            { date, 2.5, 5.0, 1.5, 30, 20, 5, 8.0 + day, 60.0 + day * 4 });
    }

    // Three sports sessions in Prague: days ago, start hour, minutes, kind, name, center of the loop, loop radius
    struct Sport {
        int daysAgo;
        int startHour;
        int minutes;
        ActivityKind::Type kind;
        const char* name;
        double latitude;
        double longitude;
        double radius;
    };
    const Sport sports[] = {
        { 1, 18, 45, ActivityKind::Running, "Evening run", 50.1055, 14.4200, 0.006 },
        { 3, 12, 70, ActivityKind::Walking, "Lunch walk", 50.0870, 14.4210, 0.004 },
        { 5, 17, 80, ActivityKind::Cycling, "Bike ride", 50.0700, 14.4000, 0.030 },
    };
    for (const Sport& sport : sports) {
        const QDateTime start(now.date().addDays(-sport.daysAgo), QTime(sport.startHour, 0));
        const QDateTime end = start.addSecs(sport.minutes * 60);
        insert("sports_data",
            { "name", "version", "start_timestamp", "start_timestamp_dt", "end_timestamp", "end_timestamp_dt", "kind",
                "base_longitude", "base_latitude", "base_altitude", "device_id", "user_id", "gpx" },
            { sport.name, 1, start.toMSecsSinceEpoch() / 1000, start, end.toMSecsSinceEpoch() / 1000, end,
                int(sport.kind), sport.longitude, sport.latitude, 200.0, deviceId, userId,
                demoGpx(start, sport.minutes, sport.latitude, sport.longitude, sport.radius) });
    }

    db.commit();
}
