#ifndef GARMINSYNCHRONIZATIONMESSAGE_H
#define GARMINSYNCHRONIZATIONMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"

#include <QObject>

class GarminSynchronizationMessage : public GarminGfdiMessage
{
public:
    GarminSynchronizationMessage(CommunicatorV2 *parent) {
        mCommunicator = parent;
    }
    void parse(const QByteArray& data);
    QByteArray generateOutgoing(const QByteArray& data);
private:
    void sendSynchronizationAck();
    void sendFilterMessage(quint8 filterType);
};

#endif // GARMINSYNCHRONIZATIONMESSAGE_H
