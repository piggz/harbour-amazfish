#ifndef GARMINFILTERSTATUSMESSAGE_H
#define GARMINFILTERSTATUSMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"

#include <QObject>

class GarminFilterStatusMessage : public GarminGfdiMessage
{
public:
    GarminFilterStatusMessage(CommunicatorV2 *parent) {
        mCommunicator = parent;
    }
    void parse(const QByteArray& data);
    QByteArray generateOutgoing(const QByteArray& data);
};

#endif // GARMINFILTERSTATUSMESSAGE_H
