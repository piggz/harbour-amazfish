#ifndef HUAMIDATASOURCE_H
#define HUAMIDATASOURCE_H

#include "datasource.h"

class HuamiDataSource : public DataSource
{
public:
    HuamiDataSource();

protected:
    QList<SleepSession> calculateSleep(const QDate &day) override;

private:
     bool isSleep(int kind);
};

#endif // HUAMIDATASOURCE_H
