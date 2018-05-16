#ifndef QBLEDEVICE_H
#define QBLEDEVICE_H

#include <QObject>
#include "qbleservice.h"
#include <QtDBus>

class QBLEDevice : public QObject
{
    Q_OBJECT
public:
    explicit QBLEDevice(QObject *parent = nullptr);


    QBLEService *service(const QString &uuid);

    void connectToDevice(const QString &path);
    void disconnectFromDevice();

private:
    QString m_devicePath;
    QMap<QString, QBLEService*> m_serviceMap;
    QDBusInterface *m_deviceService;

    void introspect();
};

#endif // QBLEDEVICE_H
