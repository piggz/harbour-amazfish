#ifndef HUAMIFETCHER_H
#define HUAMIFETCHER_H


#include "abstractfetchoperation.h"
#include "amazfish.h"

class HuamiDevice;

class HuamiFetcher : public QObject
{
    Q_OBJECT
public:
    HuamiFetcher(HuamiDevice *device);
    ~HuamiFetcher();

    void startFetchData(Amazfish::DataTypes type);
    void fetchControl(const QByteArray &value);
    void fetchData(const QByteArray &value);

    void setNotifications(bool control, bool data);
    void writeControl(const QByteArray value);
    void message(const QString &string);
    void jumpQueue(AbstractFetchOperation* operation);
    void reset();

    Q_SLOT bool busy() const;

signals:
    void busyChanged();
    void fetchOperationComplete(AbstractFetchOperation* operation);

private:
    AbstractFetchOperation *m_currentOperation = nullptr;
    HuamiDevice *m_device = nullptr;
    bool m_busy = false;
    QTimer *m_operationTimeout = nullptr;
    void setBusy(bool b);
    void triggerNextOperation();

    QList<AbstractFetchOperation*> m_operations;
    void operationTimeout();
};

#endif // HUAMIFETCHER_H
