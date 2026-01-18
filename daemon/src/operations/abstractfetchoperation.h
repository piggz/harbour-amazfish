#ifndef ABSTRACTFETCHOPERATION_H
#define ABSTRACTFETCHOPERATION_H

#include "abstractoperation.h"

#include <QDateTime>
#include <QString>

class HuamiFetcher;

class AbstractFetchOperation : public AbstractOperation
{
public:
    explicit AbstractFetchOperation(HuamiFetcher *fetcher, bool isZeppOs = false);

    bool handleMetaData(const QByteArray &meta) override;
    void handleData(const QByteArray &data) override;
    bool characteristicChanged(const QString &characteristic, const QByteArray &value) override;
private:
    QDateTime m_startDate;
    QString m_lastSyncKey;
    bool m_abort = false;
    bool m_isZeppOs = false;

    bool handleStartDateResponse(const QByteArray &value);
    bool handleFetchDataResponse(const QByteArray &value);
    void sendAck();

protected:
    void setStartDate(const QDateTime &sd);
    QDateTime startDate() const;

    void setLastSyncKey(const QString &key);
    QDateTime lastActivitySync();
    void saveLastActivitySync(qint64 millis);

    void setAbort(bool abort);
    virtual bool processBufferedData() = 0;


    HuamiFetcher *m_fetcher = nullptr;
    QBLEService *m_service = nullptr;

    QByteArray m_buffer;
    bool m_valid = true;
    int m_lastPacketCounter = -1;
};

#endif // ABSTRACTFETCHOPERATION_H
