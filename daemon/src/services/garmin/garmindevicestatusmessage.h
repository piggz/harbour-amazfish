#ifndef GARMINDEVICESTATUSMESSAGE_H
#define GARMINDEVICESTATUSMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"

#include <QObject>

class GarminDeviceStatusMessage : public GarminGfdiMessage
{
public:
    GarminDeviceStatusMessage(CommunicatorV2 *parent) {
        mCommunicator = parent;
    }
        void parse(const QByteArray& data);
        QByteArray generateBatteryStatusRequest();
};

#endif // GARMINDEVICESTATUSMESSAGE_H
