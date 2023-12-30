#ifndef BIPDEVICE_H
#define BIPDEVICE_H

#include "huamidevice.h"
#include "qble/qbleservice.h"

#include "deviceinfoservice.h"
#include "mibandservice.h"
#include "miband2service.h"
#include "alertnotificationservice.h"
#include "hrmservice.h"
#include "bipfirmwareservice.h"

#include <QTimer>

/*
BIP Services and Characteristics

{0000FEE0-0000-1000-8000-00805f9b34fb} MiBand Service
--00002a2b-0000-1000-8000-00805f9b34fb //Current time
--00002a04-0000-1000-8000-00805f9b34fb //Peripheral Preferred Connection Parameters
--00000001-0000-3512-2118-0009af100700 //Unknown 1
--00000002-0000-3512-2118-0009af100700 //Unknown 2 Weather?
--00000003-0000-3512-2118-0009af100700 //Configuration
--00000004-0000-3512-2118-0009af100700 //Unknown 4
--00000005-0000-3512-2118-0009af100700 //Activity data
--00000006-0000-3512-2118-0009af100700 //Battery Info
--00000007-0000-3512-2118-0009af100700 //Realtime steps
--00000008-0000-3512-2118-0009af100700 //User settings
--00000010-0000-3512-2118-0009af100700 //Device event
--0000000e-0000-3512-2118-0009af100700 //Weather
--00000020-0000-3512-2118-0009af100700

{0000FEE1-0000-1000-8000-00805f9b34fb} MiBand2 Service
--00000009-0000-3512-2118-0009af100700 //Auth
--0000fedd-0000-1000-8000-00805f9b34fb //Unknown
--0000fede-0000-1000-8000-00805f9b34fb //Unknown
--0000fedf-0000-1000-8000-00805f9b34fb //Unknown
--0000fed0-0000-1000-8000-00805f9b34fb //Unknown
--0000fed1-0000-1000-8000-00805f9b34fb //Unknown
--0000fed2-0000-1000-8000-00805f9b34fb //Unknown
--0000fed3-0000-1000-8000-00805f9b34fb //Unknown

{00001530-0000-3512-2118-0009af100700} Firmware Service
--00001531-0000-3512-2118-0009af100700 //Firmware
--00001532-0000-3512-2118-0009af100700 //Firmware data

{00001800-0000-1000-8000-00805f9b34fb} Generic Access
--00002a00-0000-1000-8000-00805f9b34fb //Device name
--00002a01-0000-1000-8000-00805f9b34fb //Appearance
--00002a02-0000-1000-8000-00805f9b34fb //Peripheral Privacy Flag
--00002a04-0000-1000-8000-00805f9b34fb //Peripheral Preferred Connection Parameters

{00001801-0000-1000-8000-00805f9b34fb} Generic Attribute
--00002a05-0000-1000-8000-00805f9b34fb //Service Changed

{00001802-0000-1000-8000-00805f9b34fb} Immediate Alert
--00002a06-0000-1000-8000-00805f9b34fb //Alert level

{0000180d-0000-1000-8000-00805f9b34fb} Heart rate service
--00002a37-0000-1000-8000-00805f9b34fb //Heart rate measuremnet
--00002a39-0000-1000-8000-00805f9b34fb //Heart rate control point

{0000180a-0000-1000-8000-00805f9b34fb} Info Service
--00002a25-0000-1000-8000-00805f9b34fb //Serial number string
--00002a27-0000-1000-8000-00805f9b34fb //Hardware revision string
--00002a28-0000-1000-8000-00805f9b34fb //Software revision string
--00002a23-0000-1000-8000-00805f9b34fb //System Id
--00002a50-0000-1000-8000-00805f9b34fb //Pnp ID

{00001811-0000-1000-8000-00805f9b34fb} Alert notification service
--00002a46-0000-1000-8000-00805f9b34fb //New alert
--00002a44-0000-1000-8000-00805f9b34fb //Alert notification control poiont

{00003802-0000-1000-8000-00805f9b34fb} Unknown
--00004a02-0000-1000-8000-00805f9b34fb //Unknown

*/

class BipDevice : public HuamiDevice
{
    Q_OBJECT
public:
    explicit BipDevice(const QString &pairedName, QObject *parent = nullptr);

    int supportedFeatures() const override;
    QString deviceType() const override;
    QStringList supportedDisplayItems() const override;
    void applyDeviceSetting(AbstractDevice::Settings s) override;

    AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;

    void sendWeather(CurrentWeather *weather) override;

    void navigationRunning(bool running) override;
    void navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress) override;

protected:
    void initialise() override;

private:
    void parseServices();

};

#endif // BIPDEVICE_H
