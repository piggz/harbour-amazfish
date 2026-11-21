#include "database.h"
#include "amazfishconfig.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

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
