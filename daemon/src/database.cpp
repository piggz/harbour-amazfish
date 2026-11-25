#include "database.h"
#include "amazfishconfig.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

static bool createSchema()
{
    // The same tables as KDb created them, so databases made by older versions and new ones look the same.
    const QStringList statements = {
        "CREATE TABLE IF NOT EXISTS [mi_band_activity] ([id] INTEGER PRIMARY KEY, [timestamp] Integer UNSIGNED, "
        "[timestamp_dt] DateTime, [device_id] Integer UNSIGNED, [user_id] Integer UNSIGNED, "
        "[raw_intensity] Integer UNSIGNED, [steps] Integer UNSIGNED, [raw_kind] Integer UNSIGNED, "
        "[heartrate] Integer UNSIGNED)",
        "CREATE INDEX IF NOT EXISTS idx_mi_band_activity_timestamp ON mi_band_activity (timestamp)",
        "CREATE INDEX IF NOT EXISTS idx_mi_band_activity_timestamp_dt ON mi_band_activity (timestamp_dt)",
        "CREATE TABLE IF NOT EXISTS [info_log] ([id] INTEGER PRIMARY KEY, [timestamp] Integer, "
        "[timestamp_dt] DateTime, [key] Integer UNSIGNED, [value] Integer UNSIGNED)",
        "CREATE TABLE IF NOT EXISTS [sports_data] ([id] INTEGER PRIMARY KEY, [name] Text, [version] Integer UNSIGNED, "
        "[start_timestamp] Integer UNSIGNED, [start_timestamp_dt] DateTime, [end_timestamp] Integer UNSIGNED, "
        "[end_timestamp_dt] DateTime, [kind] Integer UNSIGNED, [base_longitude] Double, [base_latitude] Double, "
        "[base_altitude] Double, [device_id] Integer UNSIGNED, [user_id] Integer UNSIGNED, [gpx] CLOB)",
        "CREATE TABLE IF NOT EXISTS [sports_meta] ([id] INTEGER PRIMARY KEY, [sport_id] Integer UNSIGNED, [key] Text, "
        "[value] Text, [unit] Text)",
        "CREATE TABLE IF NOT EXISTS [pai] ([id] INTEGER PRIMARY KEY, [pai_date] Date, [pai_low] Float, "
        "[pai_moderate] Float, [pai_high] Float, [pai_low_time] ShortInteger, [pai_moderate_time] ShortInteger, "
        "[pai_high_time] ShortInteger, [pai_today] Float, [pai_total] Float)",
        "CREATE TABLE IF NOT EXISTS [hrv] ([hrv_id] INTEGER PRIMARY KEY, [hrv_timestamp] Integer, "
        "[hrv_timestamp_dt] DateTime, [hrv_value] Integer)",
        "CREATE TABLE IF NOT EXISTS [spo2] ([spo2_id] INTEGER PRIMARY KEY, [spo2_timestamp] Integer, "
        "[spo2_timestamp_dt] DateTime, [spo2_automatic] Boolean, [spo2_value] Integer)",
        "CREATE TABLE IF NOT EXISTS [spo2sleep] ([spo2sleep_id] INTEGER PRIMARY KEY, [spo2sleep_timestamp] Integer, "
        "[spo2sleep_timestamp_dt] DateTime, [spo2sleep_value] Integer, [spo2sleep_duration] Integer, "
        "[spo2sleep_high] Text, [spo2sleep_low] Text, [spo2sleep_quality] Text)",
        "CREATE TABLE IF NOT EXISTS [temperature] ([temperature_id] INTEGER PRIMARY KEY, "
        "[temperature_timestamp] Integer, [temperature_timestamp_dt] DateTime, [temperature_value] Double, "
        "[temperature_location] Text, [temperature_type] Text)",
        "CREATE TABLE IF NOT EXISTS [stress] ([stress_id] INTEGER PRIMARY KEY, [stress_timestamp] Integer, "
        "[stress_timestamp_dt] DateTime, [stress_value] Integer, [stress_type] ShortInteger)",
        // The UI reads time ranges by *_timestamp_dt; the daemon looks up *_timestamp before it inserts a record.
        "CREATE INDEX IF NOT EXISTS idx_info_log_key_timestamp_dt ON info_log (key, timestamp_dt)",
        "CREATE INDEX IF NOT EXISTS idx_sports_meta_sport_id ON sports_meta (sport_id)",
        "CREATE INDEX IF NOT EXISTS idx_pai_pai_date ON pai (pai_date)",
        "CREATE INDEX IF NOT EXISTS idx_hrv_hrv_timestamp ON hrv (hrv_timestamp)",
        "CREATE INDEX IF NOT EXISTS idx_hrv_hrv_timestamp_dt ON hrv (hrv_timestamp_dt)",
        "CREATE INDEX IF NOT EXISTS idx_spo2_spo2_timestamp ON spo2 (spo2_timestamp)",
        "CREATE INDEX IF NOT EXISTS idx_spo2_spo2_timestamp_dt ON spo2 (spo2_timestamp_dt)",
        "CREATE INDEX IF NOT EXISTS idx_spo2sleep_spo2sleep_timestamp ON spo2sleep (spo2sleep_timestamp)",
        "CREATE INDEX IF NOT EXISTS idx_spo2sleep_spo2sleep_timestamp_dt ON spo2sleep (spo2sleep_timestamp_dt)",
        "CREATE INDEX IF NOT EXISTS idx_temperature_temperature_timestamp ON temperature (temperature_timestamp)",
        "CREATE INDEX IF NOT EXISTS idx_temperature_temperature_timestamp_dt ON temperature (temperature_timestamp_dt)",
        "CREATE INDEX IF NOT EXISTS idx_stress_stress_timestamp ON stress (stress_timestamp)",
        "CREATE INDEX IF NOT EXISTS idx_stress_stress_timestamp_dt ON stress (stress_timestamp_dt)",
    };

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    QSqlQuery query;
    for (const QString& statement : statements) {
        if (!query.exec(statement)) {
            qWarning() << Q_FUNC_INFO << "Cannot create the database schema:" << query.lastError().text() << statement;
            db.rollback();
            return false;
        }
    }
    if (!db.commit()) {
        qWarning() << Q_FUNC_INFO << "Cannot create tables:" << db.lastError().text();
        return false;
    }
    return true;
}

// True when the query returns a row.
static bool exists(const QString& sql)
{
    QSqlQuery query;
    bool found = query.exec(sql) && query.next();
    qDebug() << sql << found;
    return found;
}

bool Database::open(const QString& fileName)
{
    qDebug() << Q_FUNC_INFO << ": Database is: " << fileName;
    QDir().mkpath(QFileInfo(fileName).absolutePath());

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    db.setDatabaseName(fileName);
    if (!db.open()) {
        qWarning() << Q_FUNC_INFO << ": Could not open database:" << db.lastError().text();
        return false;
    }
    if (!createSchema()) {
        return false;
    }
    qDebug() << db.tables();
    return true;
}

QVariant Database::dateTimeToText(const QDateTime& dateTime)
{
    if (!dateTime.isValid()) {
        return QVariant();
    }
    if (dateTime.time().msec() == 0) {
        return dateTime.toString(QStringLiteral("yyyy-MM-dd'T'HH:mm:ss"));
    }
    return dateTime.toString(QStringLiteral("yyyy-MM-dd'T'HH:mm:ss.zzz"));
}

bool Database::storeActivitySamples(const QList<ActivitySample>& samples)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << Q_FUNC_INFO << "Database not connected, lost samples:" << samples.size();
        return false;
    }

    uint userId = qHash(AmazfishConfig::instance()->profileName());
    uint deviceId = qHash(AmazfishConfig::instance()->pairedAddress());

    db.transaction();
    QSqlQuery query;
    query.prepare("INSERT INTO mi_band_activity (timestamp, timestamp_dt, device_id, user_id, raw_intensity, steps, "
                  "raw_kind, heartrate) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    for (const ActivitySample& sample : samples) {
        query.addBindValue(sample.datetime().toMSecsSinceEpoch() / 1000);
        query.addBindValue(dateTimeToText(sample.datetime()));
        query.addBindValue(deviceId);
        query.addBindValue(userId);
        query.addBindValue(sample.intensity());
        query.addBindValue(sample.steps());
        query.addBindValue(sample.kind());
        query.addBindValue(sample.heartrate());
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << "Error inserting record" << sample << query.lastError().text();
            db.rollback();
            return false;
        }
    }
    if (!db.commit()) {
        qWarning() << Q_FUNC_INFO << "Cannot save samples:" << db.lastError().text();
        return false;
    }
    return true;
}

int Database::stepsOnDay(const QDate& day)
{
    if (!QSqlDatabase::database().isOpen()) {
        qDebug() << Q_FUNC_INFO << "database is not open";
        return -1;
    }

    uint userId = qHash(AmazfishConfig::instance()->profileName());
    uint deviceId = qHash(AmazfishConfig::instance()->pairedAddress());

    QString sql = QString("SELECT SUM(steps) FROM mi_band_activity "
                          "WHERE device_id = '%1' AND user_id = '%2' AND DATE(timestamp_dt) = '%3'")
                      .arg(deviceId)
                      .arg(userId)
                      .arg(day.toString("yyyy-MM-dd"));
    qDebug() << sql;

    QSqlQuery query;
    if (!query.exec(sql)) {
        qWarning() << Q_FUNC_INFO << "Error executing query" << query.lastError().text();
        return 0;
    }
    int steps = 0;
    if (query.next()) {
        steps = query.value(0).toString().toInt();
    }
    return steps;
}

bool Database::changeDeviceId(const QString& oldAddress, const QString& newAddress)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << Q_FUNC_INFO << "no database, deferring address migration";
        return false;
    }

    db.transaction();
    QSqlQuery query;
    bool ok = query.exec(QString("UPDATE mi_band_activity SET device_id=%1 WHERE device_id=%2")
                             .arg(qHash(newAddress))
                             .arg(qHash(oldAddress)));
    ok = query.exec(QString("UPDATE sports_data SET device_id=%1 WHERE device_id=%2")
                        .arg(qHash(newAddress))
                        .arg(qHash(oldAddress)))
        && ok;
    if (!ok) {
        qWarning() << Q_FUNC_INFO << "Error updating device id" << query.lastError().text();
        db.rollback();
        return false;
    }
    return db.commit();
}

bool Database::storeInfo(int key, int value)
{
    if (!QSqlDatabase::database().isOpen()) {
        qDebug() << Q_FUNC_INFO << "Database not connected";
        return false;
    }

    QDateTime now = QDateTime::currentDateTime();
    QSqlQuery query;
    query.prepare("INSERT INTO info_log (timestamp, timestamp_dt, key, value) VALUES (?, ?, ?, ?)");
    query.addBindValue(now.toMSecsSinceEpoch() / 1000);
    query.addBindValue(dateTimeToText(now));
    query.addBindValue(key);
    query.addBindValue(value);
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "error inserting record" << query.lastError().text();
        return false;
    }
    return true;
}

QVariant Database::lastInfoValue(int key)
{
    QSqlQuery query;
    if (query.exec(QString("SELECT value FROM info_log WHERE key = %1 ORDER BY id DESC LIMIT 1").arg(key))
        && query.next()) {
        return query.value(0);
    }
    return QVariant();
}

bool Database::storeActivitySummary(ActivitySummary& summary)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << Q_FUNC_INFO << "Database not connected";
        return false;
    }

    db.transaction();
    QSqlQuery query;
    query.prepare("INSERT INTO sports_data (name, version, start_timestamp, start_timestamp_dt, end_timestamp, "
                  "end_timestamp_dt, device_id, user_id, kind, base_longitude, base_latitude, base_altitude, gpx) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(summary.name());
    query.addBindValue(summary.version());
    query.addBindValue(summary.startTime().toMSecsSinceEpoch() / 1000);
    query.addBindValue(dateTimeToText(summary.startTime()));
    query.addBindValue(summary.endTime().toMSecsSinceEpoch() / 1000);
    query.addBindValue(dateTimeToText(summary.endTime()));
    query.addBindValue(summary.deviceId());
    query.addBindValue(summary.profileId());
    query.addBindValue(summary.activityKind());
    if (summary.hasBaseLocation()) {
        query.addBindValue(summary.baseLongitude());
        query.addBindValue(summary.baseLatitude());
        query.addBindValue(summary.baseAltitude());
    } else {
        query.addBindValue(QVariant());
        query.addBindValue(QVariant());
        query.addBindValue(QVariant());
    }
    query.addBindValue(summary.gpx());
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "Error inserting record" << query.lastError().text();
        db.rollback();
        return false;
    }

    long lastId = query.lastInsertId().toLongLong();
    summary.setId(lastId);

    query.prepare("INSERT INTO sports_meta (sport_id, key, value, unit) VALUES (?, ?, ?, ?)");
    for (const ActivitySummary::meta& m : summary.metaDataList()) {
        query.addBindValue(QVariant::fromValue(lastId));
        query.addBindValue(m.key);
        query.addBindValue(m.value);
        query.addBindValue(m.unit);
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << "Error inserting meta record" << query.lastError().text();
            db.rollback();
            return false;
        }
    }

    if (!db.commit()) {
        qWarning() << Q_FUNC_INFO << "Cannot save the activity:" << db.lastError().text();
        return false;
    }
    return true;
}

bool Database::storeHrvRecords(const QVector<HrvRecord>& records)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << Q_FUNC_INFO << "Database not connected";
        return false;
    }

    bool success = true;
    db.transaction();
    QSqlQuery query;
    for (const HrvRecord& r : records) {
        qDebug() << "Processing record:" << r.timestamp << r.value;
        if (exists(QString("SELECT hrv_id FROM hrv WHERE hrv_timestamp=%1").arg(r.timestamp.toMSecsSinceEpoch() / 1000))) {
            qDebug() << "HRV record exists, skipping";
            continue;
        }
        query.prepare("INSERT INTO hrv (hrv_timestamp, hrv_timestamp_dt, hrv_value) VALUES (?, ?, ?)");
        query.addBindValue(r.timestamp.toMSecsSinceEpoch() / 1000);
        query.addBindValue(dateTimeToText(r.timestamp));
        query.addBindValue(r.value);
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << "Error inserting hrv record" << query.lastError().text();
            success = false;
        }
    }
    if (!db.commit()) {
        qWarning() << Q_FUNC_INFO << "Cannot save records:" << db.lastError().text();
        return false;
    }
    return success;
}

bool Database::storeSpo2Records(const QVector<Spo2Record>& records)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << Q_FUNC_INFO << "Database not connected";
        return false;
    }

    bool success = true;
    db.transaction();
    QSqlQuery query;
    for (const Spo2Record& r : records) {
        qDebug() << "Processing record:" << r.timestamp << r.value;
        if (exists(QString("SELECT spo2_id FROM spo2 WHERE spo2_timestamp=%1").arg(r.timestamp.toMSecsSinceEpoch() / 1000))) {
            qDebug() << "SPO2 record exists, skipping";
            continue;
        }
        query.prepare("INSERT INTO spo2 (spo2_timestamp, spo2_timestamp_dt, spo2_automatic, spo2_value) "
                      "VALUES (?, ?, ?, ?)");
        query.addBindValue(r.timestamp.toMSecsSinceEpoch() / 1000);
        query.addBindValue(dateTimeToText(r.timestamp));
        query.addBindValue(r.automatic);
        query.addBindValue(r.value);
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << "Error inserting spo2 record" << query.lastError().text();
            success = false;
        }
    }
    if (!db.commit()) {
        qWarning() << Q_FUNC_INFO << "Cannot save records:" << db.lastError().text();
        return false;
    }
    return success;
}

bool Database::storeSpo2SleepRecords(const QVector<Spo2SleepRecord>& records)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << Q_FUNC_INFO << "Database not connected";
        return false;
    }

    bool success = true;
    db.transaction();
    QSqlQuery query;
    for (const Spo2SleepRecord& r : records) {
        qDebug() << "Processing record:" << r.timestamp << r.value;
        // The column is spo2sleep_timestamp, so this lookup always fails and every record is inserted, as before.
        if (exists(QString("SELECT spo2sleep_id FROM spo2sleep WHERE spo2_timestamp=%1")
                       .arg(r.timestamp.toMSecsSinceEpoch() / 1000))) {
            qDebug() << "SPO2 Sleep record exists, skipping";
            continue;
        }
        query.prepare("INSERT INTO spo2sleep (spo2sleep_timestamp, spo2sleep_timestamp_dt, spo2sleep_value, "
                      "spo2sleep_duration, spo2sleep_high, spo2sleep_low, spo2sleep_quality) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(r.timestamp.toMSecsSinceEpoch() / 1000);
        query.addBindValue(dateTimeToText(r.timestamp));
        query.addBindValue(r.value);
        query.addBindValue(r.duration);
        query.addBindValue(r.spo2High);
        query.addBindValue(r.spo2Low);
        query.addBindValue(r.spo2Quality);
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << "Error inserting spo2 record" << query.lastError().text();
            success = false;
        }
    }
    if (!db.commit()) {
        qWarning() << Q_FUNC_INFO << "Cannot save records:" << db.lastError().text();
        return false;
    }
    return success;
}

bool Database::storeStressRecords(const QVector<StressRecord>& records)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << Q_FUNC_INFO << "Database not connected";
        return false;
    }

    bool success = true;
    db.transaction();
    QSqlQuery query;
    for (const StressRecord& r : records) {
        qDebug() << "Processing record:" << r.timestamp << r.value;
        if (exists(QString("SELECT stress_id FROM stress WHERE stress_timestamp=%1 AND stress_type=%2")
                       .arg(r.timestamp.toMSecsSinceEpoch() / 1000)
                       .arg(r.type))) {
            qDebug() << "Stress record exists, skipping";
            continue;
        }
        query.prepare("INSERT INTO stress (stress_timestamp, stress_timestamp_dt, stress_value, stress_type) "
                      "VALUES (?, ?, ?, ?)");
        query.addBindValue(r.timestamp.toMSecsSinceEpoch() / 1000);
        query.addBindValue(dateTimeToText(r.timestamp));
        query.addBindValue(r.value);
        query.addBindValue(r.type);
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << "Error inserting stress record" << query.lastError().text();
            success = false;
        }
    }
    if (!db.commit()) {
        qWarning() << Q_FUNC_INFO << "Cannot save records:" << db.lastError().text();
        return false;
    }
    return success;
}

bool Database::storeTemperatureRecords(const QVector<TemperatureRecord>& records)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << Q_FUNC_INFO << "Database not connected";
        return false;
    }

    bool success = true;
    db.transaction();
    QSqlQuery query;
    for (const TemperatureRecord& r : records) {
        if (exists(QString("SELECT temperature_id FROM temperature WHERE temperature_timestamp=%1")
                       .arg(r.timestamp.toMSecsSinceEpoch() / 1000))) {
            qDebug() << "Temp record exists, skipping";
            continue;
        }
        query.prepare("INSERT INTO temperature (temperature_timestamp, temperature_timestamp_dt, "
                      "temperature_value, temperature_location, temperature_type) VALUES (?, ?, ?, ?, ?)");
        query.addBindValue(r.timestamp.toMSecsSinceEpoch() / 1000);
        query.addBindValue(dateTimeToText(r.timestamp));
        query.addBindValue(r.value);
        query.addBindValue(r.location);
        query.addBindValue(r.type);
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << "Error inserting temperature record" << query.lastError().text();
            success = false;
        }
    }
    if (!db.commit()) {
        qWarning() << Q_FUNC_INFO << "Cannot save records:" << db.lastError().text();
        return false;
    }
    return success;
}

bool Database::storePaiRecords(const QVector<PaiRecord>& records, QDate& lastNewDay)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << Q_FUNC_INFO << "Database not connected";
        return false;
    }

    bool success = true;
    db.transaction();
    QSqlQuery query;
    for (const PaiRecord& r : records) {
        qDebug() << "Processing record:" << r.day << r.total_today << r.total;
        if (exists(QString("SELECT id FROM pai WHERE pai_date='%1'").arg(r.day.toString(Qt::ISODate)))) {
            qDebug() << "PAI record exists, updating";
            query.prepare("UPDATE pai SET pai_low=?, pai_moderate=?, pai_high=?, pai_low_time=?, pai_moderate_time=?, "
                          "pai_high_time=?, pai_today=?, pai_total=? WHERE pai_date=?");
        } else {
            qDebug() << "PAI record does not exist, inserting";
            lastNewDay = r.day;
            query.prepare("INSERT INTO pai (pai_low, pai_moderate, pai_high, pai_low_time, pai_moderate_time, "
                          "pai_high_time, pai_today, pai_total, pai_date) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
        }
        query.addBindValue(r.low);
        query.addBindValue(r.moderate);
        query.addBindValue(r.high);
        query.addBindValue(r.time_low);
        query.addBindValue(r.time_moderate);
        query.addBindValue(r.time_high);
        query.addBindValue(r.total_today);
        query.addBindValue(r.total);
        query.addBindValue(r.day.toString(Qt::ISODate));
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << "Error saving pai record" << query.lastError().text();
            success = false;
        }
    }
    if (!db.commit()) {
        qWarning() << Q_FUNC_INFO << "Cannot save records:" << db.lastError().text();
        return false;
    }
    return success;
}
