#ifndef BLUETOOTHLEINTERFACE_H
#define BLUETOOTHLEINTERFACE_H

#include <QObject>
#include <QLowEnergyController>
#include <QBluetoothDeviceDiscoveryAgent>
//#include "gato.h"

class BipPair : public QObject
{
    Q_OBJECT
public:
    BipPair();

    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_INVOKABLE void startScan();

    const QString UUID_SERVICE_BIP_AUTH = "{0000fee1-0000-1000-8000-00805f9b34fb}";
    const QString UUID_CHARACTERISTEC_AUTH = "{00000009-0000-3512-2118-0009af100700}";

    const char AUTH_SEND_KEY = 0x01;
    const char AUTH_REQUEST_RANDOM_AUTH_NUMBER = 0x02;
    const char AUTH_SEND_ENCRYPTED_AUTH_NUMBER = 0x03;
    const QByteArray AUTH_SECRET_KEY = "0123456789@ABCDE";
    const char AUTH_RESPONSE = 0x10;
    const char AUTH_SUCCESS = 0x01;
    const char AUTH_FAIL = 0x04;
    const char AUTH_BYTE = 0x08;

    QString status() const;
    Q_INVOKABLE QString watchName() const;
    Q_INVOKABLE QString watchAddress() const;

    Q_SIGNAL void pairComplete();
    Q_SIGNAL void statusChanged();

private:
    QBluetoothDeviceDiscoveryAgent *m_deviceDiscoveryAgent;
    QLowEnergyController *m_control = nullptr;
    //GatoPeripheral *m_peripheral;
    QLowEnergyService *m_service;
    QLowEnergyDescriptor m_notificationDesc;
    QLowEnergyCharacteristic m_authChar;

//GatoCentralManager manager;

    QString m_status;
    QString m_pairedName;
    QString m_pairedAddress;

    void setStatus(const QString &s);

    Q_SLOT void scanFinished();
    Q_SLOT void scanError();
    Q_SLOT void foundDevice(const QBluetoothDeviceInfo &device);
    Q_SLOT void serviceDiscovered(const QBluetoothUuid &gatt);
    Q_SLOT void serviceStateChanged(QLowEnergyService::ServiceState s);
    Q_SLOT void authCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);

    //Q_SLOT void discoveredPeripheral(GatoPeripheral *peripheral, int rssi);

    QByteArray handleAesAuth(QByteArray data, QByteArray secretKey);
};

#endif // BLUETOOTHLEINTERFACE_H
