#ifndef GARMINPROTOBUFMESSAGE_H
#define GARMINPROTOBUFMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"

#include <QObject>

class GarminProtobufMessage : public GarminGfdiMessage
{
public:
    GarminProtobufMessage(CommunicatorV2 *parent) {
        mCommunicator = parent;
    }
    void parse(const QByteArray& data);
    void parseResponse(const QByteArray& data);
private:
    void handleAuthenticationRequest(quint16 requestId);
    void handleCalendarRequest(const QByteArray& data, quint16 requestID, quint32 dataOffset);
    void sendGenericAck(const QByteArray& data);
};

#endif // GARMINPROTOBUFMESSAGE_H
