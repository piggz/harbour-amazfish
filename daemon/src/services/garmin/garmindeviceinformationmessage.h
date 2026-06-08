#ifndef GARMINDEVICEINFORMATIONMESSAGE_H
#define GARMINDEVICEINFORMATIONMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"

#include <QObject>



class GarminDeviceInformationMessage : public GarminGfdiMessage
{
public:
    explicit GarminDeviceInformationMessage(CommunicatorV2* parent=nullptr)  {
        mCommunicator = parent;
    }
    void parse(const QByteArray& data);
    QByteArray generateResponse(const DeviceInformationMessage& incoming);
};

#endif // GARMINDEVICEINFORMATIONMESSAGE_H
