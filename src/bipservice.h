#ifndef BIPSERVICE_H
#define BIPSERVICE_H

#include <QObject>
#include <QLowEnergyController>

class BipService : public QObject
{
    Q_OBJECT

public:
    BipService(const QString &uuid, QObject *parent);
    QString serviceUUID() const;

    Q_INVOKABLE bool ready() const;
    Q_SIGNAL void characteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);
    Q_SIGNAL void characteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value);

    void connectToService();
    void setController(QLowEnergyController *controller);

protected:
    QLowEnergyService *service() const;

private:
    QLowEnergyController *m_controller = nullptr;
    QLowEnergyService *m_service = nullptr;
    QString m_serviceUUID;
    bool m_ready = false;

    Q_SLOT void serviceStateChanged(QLowEnergyService::ServiceState s);
    Q_SLOT void characteristicChangedInt(const QLowEnergyCharacteristic &c, const QByteArray &value);
    Q_SLOT void characteristicReadInt(const QLowEnergyCharacteristic &c, const QByteArray &value);

};

#endif // BIPSERVICE_H
