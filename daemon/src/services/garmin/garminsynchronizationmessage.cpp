#include "garminsynchronizationmessage.h"

void GarminSynchronizationMessage::parse(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO << "Garmin: parsing synchronization";
    if (data.size() < 2) {
        return; // Result<GfdiMessage>::err(GarminError(GarminError::Code::InvalidMessage, "Synchronization message too short"));
    }
    const quint8 syncType = quint8(data[0]);
    const int size = quint8(data[1]);

    if (data.size() < 2 + size) {
        return;// Result<GfdiMessage>::err(GarminError(GarminError::Code::InvalidMessage, "Synchronization message truncated"));
    }

    quint64 bitmask = 0;
    if (size == 8) {
        bitmask = u64le(data, 2);
    } else if (size == 4) {
        bitmask = quint64(u32le(data, 2));
    } else {
        return;
    }

    SynchronizationMessage msg;
    msg.synchronizationType = syncType;
    msg.fileTypeBitmask = bitmask;
    //TODO: handle the message in Commuicator!
    // first need to send synch ack
    qDebug() << Q_FUNC_INFO << "Garmin: Send synchronization ack";
    sendSynchronizationAck();
    const quint64 workout= 1 <<3;
    const quint64 activities = 1 << 5;
    const quint64 activity_summary = 1 <<21;
    const quint64 sleep = 1 <<26;

    if ((bitmask & ( workout | activities | activity_summary | sleep))) {
        //should proceed
        qDebug() << Q_FUNC_INFO << "Garmin: Synchronization should proceed";
        // Now should send filter message
        sendFilterMessage(3);
     }
}

void GarminSynchronizationMessage::sendSynchronizationAck() {
    QByteArray r;

    // Original message ID: SYNCHRONIZATION (5037)
    writeU16le(r, 5037);
    // Status: ACK
    r.append(char(quint8(Status::Ack)));


    r=wrapInGfdiEnvelope(5000,r);
    if(mCommunicator) mCommunicator->sendMessage("SYNCHRONIZATION ACK",r);
}

void GarminSynchronizationMessage::sendFilterMessage(quint8 filterType) {
    QByteArray m;

    // Filter type
    m.append(char(filterType));

   // Add checksum
    m = wrapInGfdiEnvelope(5007,m);
    if (mCommunicator) mCommunicator->sendMessage("FILTER MESSAGE",m);
}
