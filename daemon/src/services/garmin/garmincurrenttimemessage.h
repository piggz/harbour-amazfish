#ifndef GARMINCURRENTTIMEMESSAGE_H
#define GARMINCURRENTTIMEMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"

#include <QObject>

class GarminCurrentTimeMessage : public GarminGfdiMessage
{
public:
    explicit GarminCurrentTimeMessage(CommunicatorV2 *parent) {
        mCommunicator = parent;
    }
    void parse(const QByteArray& data);
    QByteArray generateResponse(const QByteArray& data);
};

#endif // GARMINCURRENTTIMEMESSAGE_H
