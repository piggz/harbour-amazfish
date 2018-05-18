#ifndef BLUEZADAPTER_H
#define BLUEZADAPTER_H

#include <QObject>
#include <QtDBus>

class BluezAdapter : public QObject
{
    Q_OBJECT
public:
    explicit BluezAdapter(QObject *parent = nullptr);

    Q_INVOKABLE void setAdapterPath(const QString &path);
    Q_INVOKABLE void startDiscovery();
    Q_INVOKABLE void stopDiscovery();
    Q_INVOKABLE QString matchDevice(const QString &match);
    Q_INVOKABLE bool deviceIsValid(const QString &path);

private:
    QString m_adapterPath = "/org/bluez/hci0";
    QDBusInterface *m_deviceInterface = nullptr;
};

#endif // BLUEZADAPTER_H
