#include "dataproviderfactory.h"
#include "huamidatasource.h"
#include "zepposdatasource.h"

using DataCreator = std::function<DataSource*()>;

static const QMap<QString, DataCreator> dataMap = {
    { "amazfitbip", []() { return new HuamiDataSource(); } },
    { "amazfitbiplite", []() { return new HuamiDataSource(); } },
    { "amazfitgtr", []() { return new HuamiDataSource(); } },
    { "amazfitgtr2", []() { return new HuamiDataSource(); } },
    { "amazfitgts", []() { return new HuamiDataSource(); } },
    { "amazfitgts2", []() { return new HuamiDataSource(); } },
    { "amazfitneo", []() { return new HuamiDataSource(); } },
    { "amazfitzeppos", []() { return new ZeppOSDataSource(); } },
};

DataSource *DataProviderFactory::dataSource(const QString &deviceType)
{
    qDebug() << Q_FUNC_INFO <<": requested data source of type:" << deviceType;

    if (dataMap.contains(deviceType)) {
        return dataMap.value(deviceType)();
    }

    qWarning() << "Returning default DataSourced";
    return new DataSource();
}
