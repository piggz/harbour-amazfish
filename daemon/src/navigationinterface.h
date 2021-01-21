#ifndef NAVIGATIONINTERFACE_H
#define NAVIGATIONINTERFACE_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusInterface>
#include <QDBusServiceWatcher>

#define SERVICE_NAME_MAPS "io.github.rinigus.PureMaps"
#define INTERFACE_NAME "io.github.rinigus.PureMaps.navigator"
#define OBJECT_PATH "/io/github/rinigus/PureMaps/navigator"

class NavigationInterface : public QObject
{
    Q_OBJECT
public:
    explicit NavigationInterface(QObject *parent = nullptr);

signals:

    void runningChanged(bool);
    void navigationChanged(const QString& icon, const QString& narrative, const QString &manDisk, int percent);

private:
    QDBusInterface *iface = nullptr;
    QDBusServiceWatcher *m_serviceWatcher = nullptr;

    void connectDaemon();
    void disconnectDaemon();

    Q_SLOT void slot_narrativeChanged();
    Q_SLOT void slot_runningChanged();
    Q_SLOT void slot_iconChanged();
    Q_SLOT void slot_manDistChanged();

    bool m_running = false;
    QString m_icon;
    QString m_narrative;
    QString m_manDist;
    int m_progress = 0;
};

#endif // NAVIGATIONINTERFACE_H
