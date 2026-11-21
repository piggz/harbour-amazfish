#include <QDir>
#include <QQmlContext>
#include <QQmlEngine>
#include <QtQuickTest>

// Gives the QML tests the list of component files in the app resources.
class Setup : public QObject {
    Q_OBJECT

public slots:
    void qmlEngineAvailable(QQmlEngine* engine)
    {
        QStringList files = QDir(":/qml/components").entryList({ "*.qml" }, QDir::Files);
        const QStringList platformFiles = QDir(":/qml/components/platform").entryList({ "*.qml" }, QDir::Files);
        for (const QString& file : platformFiles) {
            files.append("platform/" + file);
        }
        engine->rootContext()->setContextProperty("componentFiles", files);
    }
};

QUICK_TEST_MAIN_WITH_SETUP(amazfish, Setup)
#include "tst_qml.moc"
