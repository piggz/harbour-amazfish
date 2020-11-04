#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QQuickView>
#include <QGuiApplication>
#include <QQmlContext>

#ifdef MER_EDITION_SAILFISH
#include <sailfishapp.h>
#else
#include <QQmlApplicationEngine>
#endif

#include "datasource.h"
#include "sportsdatamodel.h"
#include "sportsmetamodel.h"
#include "daemoninterface.h"

#include "weather/citysearchmodel.h"
#include "weather/citymanager.h"
#include "weather/city.h"

#include "trackloader.h"
#include "amazfishconfig.h"

int main(int argc, char *argv[])
{
    QGuiApplication *app;
#ifdef MER_EDITION_SAILFISH
    app = SailfishApp::application(argc, argv);
#else
    app = new QGuiApplication(argc, argv);
#endif

    QCoreApplication::setOrganizationName("harbour-amazfish");
    QCoreApplication::setOrganizationDomain("piggz.co.uk");
    QCoreApplication::setApplicationName("harbour-amazfish");

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    // add fallback icon path
    QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths() << ":custom-icons");

    qDebug() << QIcon::themeSearchPaths() << QIcon::fallbackSearchPaths();
#endif

    SportsDataModel sportsDataModel;
    SportsMetaModel sportsMetaModel;
    DaemonInterface daemonInterface;
    sportsDataModel.setConnection(daemonInterface.dbConnection());
    sportsMetaModel.setConnection(daemonInterface.dbConnection());

    qmlRegisterType<CitySearchModel>("org.SfietKonstantin.weatherfish", 1, 0, "CitySearchModel");
    qmlRegisterType<CityManager>("org.SfietKonstantin.weatherfish", 1, 0, "CityManager");
    qmlRegisterType<City>("org.SfietKonstantin.weatherfish", 1, 0, "City");
    qmlRegisterType<TrackLoader>("uk.co.piggz.amazfish", 1, 0, "TrackLoader");

    qmlRegisterUncreatableType<DataSource>("uk.co.piggz.amazfish", 1, 0, "DataSource", "Data Source type available only for enum datatypes");
    qmlRegisterUncreatableType<DaemonInterface>("uk.co.piggz.amazfish", 1, 0, "DaemonInterface", "DaemonInterface type available only for enum datatypes");

    qmlRegisterSingletonType<AmazfishConfig>("uk.co.piggz.amazfish", 1, 0, "AmazfishConfig", AmazfishConfig::qmlInstance);


#ifdef MER_EDITION_SAILFISH
    QQuickView *view = SailfishApp::createView();
#else
    QQmlApplicationEngine *view = new QQmlApplicationEngine();
#endif
    view->rootContext()->setContextProperty("DaemonInterfaceInstance", &daemonInterface);
    view->rootContext()->setContextProperty("SportsModel", &sportsDataModel);
    view->rootContext()->setContextProperty("SportsMeta", &sportsMetaModel);

#ifdef MER_EDITION_SAILFISH
    view->setSource(SailfishApp::pathTo("qml/harbour-amazfish.qml"));
    view->show();
#else
    view->load(QUrl("qrc:/qml/harbour-amazfish.qml"));
#endif


    return app->exec();
}
