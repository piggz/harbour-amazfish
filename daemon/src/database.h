#ifndef DATABASE_H
#define DATABASE_H

#include <QByteArray>
#include <QDate>
#include <QDateTime>
#include <QList>
#include <QString>
#include <QVariant>
#include <QVector>

#include "activitysample.h"
#include "activitysummary.h"

// All SQL of the daemon. The functions use the default connection, which open() creates.
// The functions that save records with a time skip the records that are already in the database.
namespace Database {

struct HrvRecord {
    QDateTime timestamp;
    int value;
};

struct Spo2Record {
    QDateTime timestamp;
    bool automatic;
    uint8_t value;
};

struct Spo2SleepRecord {
    QDateTime timestamp;
    uint8_t value;
    uint8_t duration;
    QByteArray spo2High; // 6
    QByteArray spo2Low; // 6
    QByteArray spo2Quality; // 8
};

struct StressRecord {
    QDateTime timestamp;
    uint8_t value;
    uint8_t type;
};

struct TemperatureRecord {
    QDateTime timestamp;
    float value;
    QString type;
    QString location;
};

struct PaiRecord {
    QDate day;
    float low;
    float moderate;
    float high;
    uint16_t time_low;
    uint16_t time_moderate;
    uint16_t time_high;
    float total_today;
    float total;
};

// Opens the database file, creates it if needed, and creates the tables and indexes that are missing.
bool open(const QString& fileName);

// Date and time as text, in the format KDb used: yyyy-MM-ddTHH:mm:ss, with .zzz only when milliseconds are not 0.
// The UI compares these texts, so new rows must look like the old ones. An invalid value gives NULL, as with KDb.
QVariant dateTimeToText(const QDateTime& dateTime);

// Saves the samples of the paired device and the current profile in one transaction. Returns false on error;
// then no sample is saved.
bool storeActivitySamples(const QList<ActivitySample>& samples);

// Steps of the paired device and the current profile on the day; -1 when the database is not open.
int stepsOnDay(const QDate& day);

// Moves the activity data and the workouts of one watch to its new address.
bool changeDeviceId(const QString& oldAddress, const QString& newAddress);

// Saves a value like the battery level (key is an Amazfish::Info) with the current time.
bool storeInfo(int key, int value);

// The last value saved with storeInfo(); an invalid QVariant when there is none.
QVariant lastInfoValue(int key);

// Saves the workout and its metadata in one transaction and sets the id of the summary.
bool storeActivitySummary(ActivitySummary& summary);

// Return false when the database is not open, a record cannot be saved, or the commit fails.
bool storeHrvRecords(const QVector<HrvRecord>& records);
bool storeSpo2Records(const QVector<Spo2Record>& records);
bool storeSpo2SleepRecords(const QVector<Spo2SleepRecord>& records);
bool storeStressRecords(const QVector<StressRecord>& records);
bool storeTemperatureRecords(const QVector<TemperatureRecord>& records);

// A day that is already saved gets the new values. lastNewDay is the last day that was not saved before.
bool storePaiRecords(const QVector<PaiRecord>& records, QDate& lastNewDay);
}

#endif // DATABASE_H
