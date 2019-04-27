#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QQuickView>
#include <QGuiApplication>
#include <QQmlContext>

#include <sailfishapp.h>

#include "datasource.h"
#include "sportsdatamodel.h"
#include "daemoninterface.h"

#include "weather/citysearchmodel.h"
#include "weather/citymanager.h"
#include "weather/city.h"
#include "weather/currentweather.h"

#include "qble/bluezadapter.h"

int main(int argc, char *argv[])
{
    QGuiApplication *app = SailfishApp::application(argc, argv);

    BluezAdapter bluezAdapter;
    SportsDataModel sportsDataModel;
    DaemonInterface daemonInterface;
    //sportsDataModel.setConnection(devInterface.dbConnection());

    qmlRegisterType<CitySearchModel>("org.SfietKonstantin.weatherfish", 1, 0, "CitySearchModel");
    qmlRegisterType<CityManager>("org.SfietKonstantin.weatherfish", 1, 0, "CityManager");
    qmlRegisterType<City>("org.SfietKonstantin.weatherfish", 1, 0, "City");
    qmlRegisterType<CurrentWeather>("org.SfietKonstantin.weatherfish", 1, 0, "CurrentWeather");

    qmlRegisterUncreatableType<DataSource>("uk.co.piggz.amazfish", 1, 0, "DataSource", "Data Source type available only for enum datatypes");
    qmlRegisterUncreatableType<DaemonInterface>("uk.co.piggz.amazfish", 1, 0, "DaemonInterface", "DaemonInterface type available only for enum datatypes");

    QQuickView *view = SailfishApp::createView();
    view->rootContext()->setContextProperty("BluezAdapter", &bluezAdapter);
    view->rootContext()->setContextProperty("DaemonInterface", &daemonInterface);
    view->rootContext()->setContextProperty("SportsModel", &sportsDataModel);

    view->setSource(SailfishApp::pathTo("qml/harbour-amazfish.qml"));
    view->show();

    return app->exec();
}
