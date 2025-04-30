#ifndef ABSTRACTFETCHOPERATION_H
#define ABSTRACTFETCHOPERATION_H

#include "abstractoperation.h"

#include <QDateTime>
#include <QString>

class AbstractFetchOperation : public AbstractOperation
{
public:
    explicit AbstractFetchOperation();

    bool handleMetaData(const QByteArray &meta) override;

private:
    QDateTime m_startDate;
    QString m_lastSyncKey;
    bool m_abort = false;

    bool handleStartDateResponse(const QByteArray &value);
    bool handleFetchDataResponse(const QByteArray &value);

protected:
    void setStartDate(const QDateTime &sd);
    QDateTime startDate() const;

    void setLastSyncKey(const QString &key);
    QDateTime lastActivitySync();
    void saveLastActivitySync(qint64 millis);

    void setAbort(bool abort);

    QBLEService *m_service = nullptr;
};

#endif // ABSTRACTFETCHOPERATION_H
