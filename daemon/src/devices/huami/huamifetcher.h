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

    void startFetchData(Amazfish::DataTypes type);
    void fetchControl(const QByteArray &value);
    void fetchData(const QByteArray &value);

    void setNotifications(bool control, bool data);
    void writeControl(const QByteArray value);
    void message(const QString &string);
    void jumpQueue(AbstractFetchOperation* operation);

    Q_SLOT bool busy() const;

signals:
    void busyChanged();
    void fetchOperationComplete(AbstractFetchOperation* operation);

private:
    AbstractFetchOperation *m_currentOperation = nullptr;
    HuamiDevice *m_device = nullptr;
    bool m_busy = false;
    void setBusy(bool b);
    void triggerNextOperation();

    QList<AbstractFetchOperation*> m_operations;
};

#endif // HUAMIFETCHER_H
