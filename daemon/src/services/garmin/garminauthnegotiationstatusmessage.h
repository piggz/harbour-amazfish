#ifndef GARMINAUTHNEGOTIATIOSTATUSNMESSAGE_H
#define GARMINAUTHNEGOTIATIONSTATUSMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"

#include <QObject>

class GarminAuthNegotiationStatusMessage : public GarminGfdiMessage
{
public:
    GarminAuthNegotiationStatusMessage(CommunicatorV2 *parent) {
        mCommunicator = parent;
    }
    void parse(const QByteArray& data);
    QByteArray generateResponse();
private:
    quint8 authNegotiationStatus;
    Status status;
    quint16 authFlags;
    char unk;
};

#endif // GARMINAUTHNEGOTIATIONSTATUSMESSAGE_H
