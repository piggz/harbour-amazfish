#ifndef DATABASE_H
#define DATABASE_H

#include <QDateTime>
#include <QList>
#include <QVariant>

#include "activitysample.h"

namespace Database {

// Date and time as text, in the format KDb used: yyyy-MM-ddTHH:mm:ss, with .zzz only when milliseconds are not 0.
// The UI compares these texts, so new rows must look like the old ones. An invalid value gives NULL, as with KDb.
QVariant dateTimeToText(const QDateTime& dateTime);

// Saves the samples of the paired device and the current profile in one transaction. Returns false on error;
// then no sample is saved.
bool storeActivitySamples(const QList<ActivitySample>& samples);

}

#endif // DATABASE_H
