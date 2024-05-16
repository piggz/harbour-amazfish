#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QQuickView>
#include <QApplication>
#include <QQmlContext>

#ifdef MER_EDITION_SAILFISH
#include <sailfishapp.h>
#else
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#endif

#include <QTranslator>

#include "datasource.h"
#include "sportsdatamodel.h"
#include "sportsmetamodel.h"
#include "daemoninterface.h"
#include "amazfish.h"

#include "weather/citysearchmodel.h"
#include "weather/citymanager.h"
#include "weather/city.h"
#include "weather/currentweather.h"

#include "trackloader.h"
#include "amazfishconfig.h"
#include "qble/adaptermodel.h"

#include "o2/src/o2.h"

QByteArray encryptDecrypt(QByteArray toEncrypt) {
    char       key    = 'K';
    QByteArray output = toEncrypt;

    for (int i = 0; i < toEncrypt.size(); i++) {
        output[i] = toEncrypt.at(i) ^ key;
    }

    return output;
}

int main(int argc, char *argv[])
{
    QGuiApplication *app;
#ifdef MER_EDITION_SAILFISH
    app = SailfishApp::application(argc, argv);
#else
    app = new QApplication(argc, argv);

  {
    QString tr_path(TRANSLATION_FOLDER);
    if ( !tr_path.isEmpty() ) {
        QString locale = QLocale::system().name();
        QTranslator *translator = new QTranslator();

        if ( !translator->load(QLocale(), "harbour-amazfish-ui", "-", tr_path) ) {
            qWarning() << "Failed to load translation for " << locale << " " << tr_path;
        }

        app->installTranslator(translator);
      }

  }

#endif


    QCoreApplication::setOrganizationName("harbour-amazfish");
    QCoreApplication::setOrganizationDomain("piggz.co.uk");
    QCoreApplication::setApplicationName("harbour-amazfish");
    QGuiApplication::setApplicationDisplayName("Amazfish");

    // disable new QML connection syntax debug messages for as long as
    // older Qt versions (5.12 and older) are supported
    QLoggingCategory::setFilterRules(QStringLiteral("qt.qml.connections=false"));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    // add fallback icon path
    QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths() << ":qml/custom-icons");

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
    qmlRegisterType<CurrentWeather>("org.SfietKonstantin.weatherfish", 1, 0, "CurrentWeather");
    qmlRegisterType<TrackLoader>("uk.co.piggz.amazfish", 1, 0, "TrackLoader");
    qmlRegisterType<AdapterModel>("uk.co.piggz.amazfish", 1, 0, "AdapterModel");
    qmlRegisterType<O2>("com.pipacs.o2", 1, 0, "O2");

    qmlRegisterUncreatableType<DataSource>("uk.co.piggz.amazfish", 1, 0, "DataSource", "Data Source type available only for enum datatypes");
    qmlRegisterUncreatableType<Amazfish>("uk.co.piggz.amazfish", 1, 0, "Amazfish", "Amazfish type available only for enum datatypes");

    qmlRegisterSingletonType<AmazfishConfig>("uk.co.piggz.amazfish", 1, 0, "AmazfishConfig", AmazfishConfig::qmlInstance);


#ifdef MER_EDITION_SAILFISH
    QQuickView *view = SailfishApp::createView();
#else
    QQmlApplicationEngine *view = new QQmlApplicationEngine();
#endif
    view->rootContext()->setContextProperty("DaemonInterfaceInstance", &daemonInterface);
    view->rootContext()->setContextProperty("SportsModel", &sportsDataModel);
    view->rootContext()->setContextProperty("SportsMeta", &sportsMetaModel);

    view->rootContext()->setContextProperty("STRAVA_CLIENT_SECRET", encryptDecrypt("}{s{--z*.x{y{ss///x/x){*xz{(|yy/{syr-/})"));
    view->rootContext()->setContextProperty("STRAVA_CLIENT_ID", "13707");

#ifdef DISABLE_SYSTEMD
    view->rootContext()->setContextProperty("ENABLE_SYSTEMD", "NO");
#else
    view->rootContext()->setContextProperty("ENABLE_SYSTEMD", "YES");
#endif

#ifdef MER_EDITION_SAILFISH
    view->setSource(SailfishApp::pathTo("qml/harbour-amazfish.qml"));
    view->show();
#elif UUITK_EDITION
    view->load("./share/harbour-amazfish-ui/qml/harbour-amazfish.qml");
#else
    view->load(QUrl("qrc:/qml/harbour-amazfish.qml"));

    if (QQuickWindow* window = qobject_cast<QQuickWindow*>(view->rootObjects().at(0))) {
        window->setIcon(QIcon(":/icons/172x172/harbour-amazfish-ui.png"));
    }

#endif

    return app->exec();
}
