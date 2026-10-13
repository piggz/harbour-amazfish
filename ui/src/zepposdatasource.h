#ifndef ZEPPOSDATASOURCE_H
#define ZEPPOSDATASOURCE_H

#include "datasource.h"

class ZeppOSDataSource : public DataSource
{
public:
    ZeppOSDataSource();

    QList<SleepSession> calculateSleep(const QDate &day) override;
};

#endif // ZEPPOSDATASOURCE_H
