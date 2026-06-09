#ifndef GARMINNOTIFICATIONCONTROLMESSAGE_H
#define GARMINNOTIFICATIONCONTROLMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"

#include <QObject>

class GarminNotificationControlMessage : public GarminGfdiMessage
{
public:
    GarminNotificationControlMessage(CommunicatorV2 *parent) {
        mCommunicator = parent;
    }
    void parse(const QByteArray& data);
    QByteArray generateResponse(const QByteArray& data);
};

#endif // GARMINNOTIFICATIONCONTROLMESSAGE_H
