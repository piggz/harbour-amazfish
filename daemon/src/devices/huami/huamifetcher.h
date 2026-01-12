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

    Q_SLOT bool busy() const;

signals:
    void busyChanged();

private:
    AbstractFetchOperation *m_currentOperation = nullptr;
    HuamiDevice *m_device = nullptr;
    bool m_busy = false;
    void setBusy(bool b);
};

#endif // HUAMIFETCHER_H
