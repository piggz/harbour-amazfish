#ifndef MIBANDSERVICE_H
#define MIBANDSERVICE_H

#include "bipservice.h"

/*
{0000FEE0-0000-1000-8000-00805f9b34fb} MiBand Service
--00002a2b-0000-1000-8000-00805f9b34fb //Current time
--00002a04-0000-1000-8000-00805f9b34fb //Peripheral Preferred Connection Parameters
--00000001-0000-3512-2118-0009af100700 //Unknown 1
--00000002-0000-3512-2118-0009af100700 //Unknown 2
--00000003-0000-3512-2118-0009af100700 //Configuration
--00000004-0000-3512-2118-0009af100700 //Unknown 4
--00000005-0000-3512-2118-0009af100700 //Activity data
--00000006-0000-3512-2118-0009af100700 //Battery Info
--00000007-0000-3512-2118-0009af100700 //Realtime steps
--00000008-0000-3512-2118-0009af100700 //User settings
--00000010-0000-3512-2118-0009af100700 //Device event
--0000000e-0000-3512-2118-0009af100700 //Weather
--00000020-0000-3512-2118-0009af100700
*/

class MiBandService : public BipService
{
    Q_OBJECT
public:
    MiBandService(QObject *parent);
    static const char* UUID_SERVICE_MIBAND;
    static const char* UUID_CHARACTERISTIC_MIBAND_CONFIGURATION;

    Q_PROPERTY(QString gpsVersion READ gpsVersion NOTIFY gpsVersionChanged())

    const char CHAR_RESPONSE = 0x10;
    const char CHAR_SUCCESS = 0x01;
    const char COMMAND_REQUEST_GPS_VERSION = 0x0e;

    Q_INVOKABLE void requestGPSVersion();
    Q_INVOKABLE QString gpsVersion();

    Q_SIGNAL void gpsVersionChanged();
private:
    Q_SLOT void characteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value);
    Q_SLOT void characteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);

    void setGPSVersion(const QString& v);

    QString m_gpsVersion;
};

#endif // MIBANDSERVICE_H
