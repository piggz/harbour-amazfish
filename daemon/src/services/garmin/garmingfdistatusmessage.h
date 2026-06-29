#ifndef GARMINGFDISTATUSMESSAGE_H
#define GARMINGFDISTATUSMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"
#include <QObject>

class GarminGfdiStatusMessage : public GarminGfdiMessage
{
public:
    GarminGfdiStatusMessage(CommunicatorV2 *parent) {
        mCommunicator = parent;
    }
    void parse(const QByteArray& data);
private:
    void handleAuthNegotiation(const QByteArray& data);
    void handleNotificationData(const QByteArray& data);
    void handleProtobufResponse(const QByteArray& data);

};

#endif // GARMINGFDISTATUSMESSAGE_H
