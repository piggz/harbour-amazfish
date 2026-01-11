#ifndef HUAMIFETCHER_H
#define HUAMIFETCHER_H


#include "abstractfetchoperation.h"
#include "amazfish.h"

class HuamiDevice;

class HuamiFetcher
{
public:
    HuamiFetcher(HuamiDevice *device);

    void startFetchData(Amazfish::DataTypes type);
    void fetchControl(const QByteArray &value);
    void fetchData(const QByteArray &value);

    void setNotifications(bool control, bool data);
    void writeControl(const QByteArray value);
    void message(const QString &string);

private:
    AbstractFetchOperation *m_currentOperation = nullptr;
    HuamiDevice *m_device = nullptr;

};

#endif // HUAMIFETCHER_H
