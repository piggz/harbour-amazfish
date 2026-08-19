#ifndef DATAPROVIDERFACTORY_H
#define DATAPROVIDERFACTORY_H

#include "datasource.h"

class DataProviderFactory
{
public:
    static DataSource* dataSource(const QString &deviceType);

};

#endif // DATAPROVIDERFACTORY_H
