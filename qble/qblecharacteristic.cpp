#include "qblecharacteristic.h"

QBLECharacteristic::QBLECharacteristic(const QString &path, QObject *parent) : QObject(parent)
{
    m_characteristicInterface = new QDBusInterface("org.bluez", path, "org.bluez.GattCharacteristic1", QDBusConnection::systemBus());

    m_uuid = m_characteristicInterface->property("UUID").toString();

    QStringList argumentMatch;
    argumentMatch << "org.bluez.GattCharacteristic1";
    QDBusConnection::systemBus().connect("org.bluez", path, "org.freedesktop.DBus.Properties","PropertiesChanged", argumentMatch, QString(),
                            this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
}

void QBLECharacteristic::onPropertiesChanged(const QString &interface, const QVariantMap &map, const QStringList &list)
{
    qDebug() << "Characteristic property changed" << interface << map;

    if (map.contains("Value")) {
        emit characteristicChanged(m_uuid, map["Value"].toByteArray());
    }
}

void QBLECharacteristic::writeValue(const QByteArray &val) const
{
    m_characteristicInterface->call("WriteValue", val, QVariantMap());
}

QByteArray QBLECharacteristic::readValue() const
{
    QDBusReply<QByteArray> reply = m_characteristicInterface->call("ReadValue", QVariantMap());
    qDebug() << reply.error().message();
    return reply.value();
}

void QBLECharacteristic::readAsync()
{
    QDBusPendingCall async = m_characteristicInterface->asyncCall("ReadValue", QVariantMap());
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);

    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(readFinished(QDBusPendingCallWatcher*)));
}

void QBLECharacteristic::readFinished(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QByteArray> reply = *call;
    if (reply.isError()) {
        qDebug() << "QBLECharacteristic::readFinished:" << reply.error().message();
    } else {
        m_value = reply.value();
        emit characteristicRead(m_uuid, m_value);
    }
    call->deleteLater();
}

void QBLECharacteristic::startNotify() const
{
    m_characteristicInterface->call("StartNotify");
}

void QBLECharacteristic::stopNotify() const
{
    m_characteristicInterface->call("StopNotify");
}

QByteArray QBLECharacteristic::value() const
{
    return m_value;
}
