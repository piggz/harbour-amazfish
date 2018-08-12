#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QQuickView>
#include <QGuiApplication>
#include <QQmlContext>

#include <sailfishapp.h>
#include "qble/bluezadapter.h"
#include "deviceinterface.h"
#include "datasource.h"
#include "sportsdatamodel.h"

#include "weather/citysearchmodel.h"
#include "weather/citymanager.h"
#include "weather/city.h"
#include "weather/currentweather.h"


int main(int argc, char *argv[])
{
    // SailfishApp::main() will display "qml/harbour-amazfish.qml", if you need more
    // control over initialization, you can use:
    //
    //   - SailfishApp::application(int, char *[]) to get the QGuiApplication *
    //   - SailfishApp::createView() to get a new QQuickView * instance
    //   - SailfishApp::pathTo(QString) to get a QUrl to a resource file
    //   - SailfishApp::pathToMainQml() to get a QUrl to the main QML file
    //
    // To display the view, call "show()" (will show fullscreen on device).

    //return SailfishApp::main(argc, argv);

    QGuiApplication *app = SailfishApp::application(argc, argv);

    DeviceInterface devInterface;
    BluezAdapter bluezAdapter;
    SportsDataModel sportsDataModel;
    sportsDataModel.setConnection(devInterface.dbConnection());

    qmlRegisterType<DeviceInfoService>();
    qmlRegisterType<MiBandService>();
    qmlRegisterType<MiBand2Service>();
    qmlRegisterType<AlertNotificationService>();
    qmlRegisterType<HRMService>();
    qmlRegisterType<BipFirmwareService>();

    qmlRegisterType<CitySearchModel>("org.SfietKonstantin.weatherfish", 1, 0, "CitySearchModel");
    qmlRegisterType<CityManager>("org.SfietKonstantin.weatherfish", 1, 0, "CityManager");
    qmlRegisterType<City>("org.SfietKonstantin.weatherfish", 1, 0, "City");
    qmlRegisterType<CurrentWeatherModel>("org.SfietKonstantin.weatherfish", 1, 0, "CurrentWeather");

    qmlRegisterUncreatableType<DataSource>("uk.co.piggz.amazfish", 1, 0, "DataSource", "Data Source type available only for enum datatypes");

    QQuickView *view = SailfishApp::createView();
    view->rootContext()->setContextProperty("BluezAdapter", &bluezAdapter);
    view->rootContext()->setContextProperty("DeviceInterface", &devInterface);
    view->rootContext()->setContextProperty("SportsModel", &sportsDataModel);

    view->setSource(SailfishApp::pathTo("qml/harbour-amazfish.qml"));
    view->show();

    return app->exec();
}
