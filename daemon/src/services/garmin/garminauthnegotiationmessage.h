#ifndef GARMINAUTHNEGOTIATIONMESSAGE_H
#define GARMINAUTHNEGOTIATIONMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"

#include <QObject>

class GarminAuthNegotiationMessage : public GarminGfdiMessage
{
public:
    GarminAuthNegotiationMessage(CommunicatorV2 *parent) {
        mCommunicator = parent;
        mMessageType = MessageId::AuthNegotiation;
    }
    void parse(const QByteArray& data);
    QByteArray generateOutgoing();
    QByteArray generateStatusMessage();
private:
    quint8 authNegotiationStatus;
    quint32 authFlags;
};

#endif // GARMINAUTHNEGOTIATIONMESSAGE_H
