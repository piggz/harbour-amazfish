#ifndef GARMINDEVICEINFORMATIONMESSAGE_H
#define GARMINDEVICEINFORMATIONMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"

#include <QObject>

// Forward declarations
//class CommunicatorV2;

class GarminDeviceInformationMessage : public GarminGfdiMessage
{
public:
    explicit GarminDeviceInformationMessage(CommunicatorV2* parent=nullptr)  {
        mCommunicator = parent;
    }
    void parse(const QByteArray& data);

};

#endif // GARMINDEVICEINFORMATIONMESSAGE_H
