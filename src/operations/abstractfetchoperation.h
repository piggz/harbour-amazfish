#ifndef ABSTRACTFETCHOPERATION_H
#define ABSTRACTFETCHOPERATION_H

#include "abstractoperation.h"

#include <QDateTime>
#include <QString>
#include "settingsmanager.h"

class AbstractFetchOperation : public AbstractOperation
{
public:
    AbstractFetchOperation(QBLEService *service);

private:
    QDateTime m_startDate;
    QString m_lastSyncKey;
    SettingsManager m_settings;

protected:
    void setStartDate(const QDateTime &sd);
    QDateTime startDate() const;

    void setLastSyncKey(const QString &key);
    QDateTime lastActivitySync();
    void saveLastActivitySync(qint64 millis);
};

#endif // ABSTRACTFETCHOPERATION_H
