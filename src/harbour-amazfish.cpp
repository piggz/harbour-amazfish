#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QQuickView>
#include <QGuiApplication>
#include <QQmlContext>

#include <sailfishapp.h>
#include "bippair.h"
#include "bipinterface.h"

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

    BipPair bipPair;
    BipInterface bipInterface;

    qmlRegisterType<BipInfoService>();
    qmlRegisterType<MiBandService>();
    qmlRegisterType<MiBand2Service>();
    qmlRegisterType<AlertNotificationService>();

    QQuickView *view = SailfishApp::createView();
    view->rootContext()->setContextProperty("BipPair", &bipPair);
    view->rootContext()->setContextProperty("BipInterface", &bipInterface);

    view->setSource(SailfishApp::pathTo("qml/harbour-amazfish.qml"));
    view->show();
    return app->exec();
}
