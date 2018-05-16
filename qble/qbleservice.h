#ifndef QBLESERVICE_H
#define QBLESERVICE_H

#include <QObject>
#include <QtDBus>

#include "qblecharacteristic.h"

class QBLEService : public QObject
{
    Q_OBJECT
public:
    explicit QBLEService(const QString &uuid, QObject *parent = nullptr);

    void setPath(const QString &path);

    Q_SIGNAL void characteristicChanged(const QString &characteristic, const QByteArray &value);
    Q_SIGNAL void characteristicRead(const QString &characteristic, const QByteArray &value);
    Q_SIGNAL void descriptorWritten(const QString &characteristic, const QByteArray &value);

    void enableNotification(const QString &c);
    void disableNotification(const QString &c);

    QString serviceUUID() const;
    QVariant readCharacteristic(const QString &characteristic);
    void writeCharacteristic(const QString &characteristic, const QByteArray &value);

protected:
    void writeValue(const char* CHARACTERISTIC, const QByteArray &value);

private:
    QString m_servicePath;
    QString m_serviceUUID;

    QMap<QString, QBLECharacteristic*> m_characteristicMap;
    QDBusInterface *m_serviceService;

    void introspect();

    Q_SLOT void characteristicChangedInt(const QString &characteristic, const QByteArray &value);
    Q_SLOT void characteristicReadInt(const QString &characteristic, const QByteArray &value);
    Q_SLOT void characteristicWrittenInt(const QString &characteristic, const QByteArray &value);
    Q_SLOT void descriptorWrittenInt(const QString &descriptor, const QByteArray &value);

};

#endif // QBLESERVICE_H
