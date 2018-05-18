#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QQuickView>
#include <QGuiApplication>
#include <QQmlContext>

#include <sailfishapp.h>
#include "qble/bluezadapter.h"
#include "deviceinterface.h"

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

    qmlRegisterType<DeviceInfoService>();
    qmlRegisterType<MiBandService>();
    qmlRegisterType<MiBand2Service>();
    qmlRegisterType<AlertNotificationService>();
    qmlRegisterType<HRMService>();

    QQuickView *view = SailfishApp::createView();
    view->rootContext()->setContextProperty("BluezAdapter", &bluezAdapter);
    view->rootContext()->setContextProperty("DeviceInterface", &devInterface);

    view->setSource(SailfishApp::pathTo("qml/harbour-amazfish.qml"));
    view->show();

    return app->exec();
}
