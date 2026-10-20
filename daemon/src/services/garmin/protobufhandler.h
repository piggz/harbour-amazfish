#ifndef PROTOBUFHANDLER_H
#define PROTOBUFHANDLER_H

#include "../devices/garmin/garmindevice.h"
#include "communicator_v2.h"
#include "garmingfdimessage.h"

#include <QObject>
#include <QMap>

class GarminProtobufMessage;
class GarminProtobufStatusMessage;
class CommunicatorV2;
class GarminDevice;



class ProtobufFragment
{
public:
    ProtobufFragment(QByteArray fragmentBytes);
    ProtobufFragment(QSharedPointer<GarminProtobufMessage> message);
    ProtobufFragment(QSharedPointer<ProtobufFragment> existing, QSharedPointer<GarminProtobufMessage> toMerge);
    QSharedPointer<GarminProtobufMessage> getNextChunk(QSharedPointer<GarminProtobufStatusMessage> protobufStatusMessage);
    QByteArray getFragmentBytes(){ return mFragmentBytes; };
    int getTotalLength() { return mTotalLength; };
    bool isComplete();
private:
    QByteArray mFragmentBytes;
    int mTotalLength;
};

class ProtobufHandler
{
public:
    ProtobufHandler(CommunicatorV2* com):mCommunicator(com){
    };
    int getNextProtobufRequestId();
    QSharedPointer<GarminProtobufMessage> processIncoming(QSharedPointer<GarminProtobufMessage> message);
    QSharedPointer<GarminProtobufMessage> processIncoming(QSharedPointer<GarminProtobufStatusMessage> message);
    QSharedPointer<ProtobufFragment> processChunkedMessage(QSharedPointer<GarminProtobufMessage> message);
    QSharedPointer<GarminProtobufMessage> prepareProtobufRequest(QByteArray protobufPayload);
        QSharedPointer<GarminProtobufMessage> prepareProtobufResponse(QByteArray protobufPayload, int requestId);
    QSharedPointer<GarminProtobufMessage> prepareProtobufMessage(QByteArray bytes, MessageId type, int requestId);

private:
    GarminDevice* mDevice;
    QMap<int, QSharedPointer<ProtobufFragment>> mChunkedFragmentsMap;
    int mMaxChunkSize = 375; //tested on Vívomove Style
    int mLastProtobufRequestId;
    CommunicatorV2* mCommunicator;
    void sendAck(QString taskName, QSharedPointer<GarminGfdiMessage> msg);
    //AppConfigHandler appConfigHandler;
    //HttpHandler httpHandler;
    //DataTransferHandler dataTransferHandler;
    //FileSyncServiceHandler fileSyncServiceHandler;
    //EcgServiceHandler ecgServiceHandler;

};

#endif // PROTOBUFHANDLER_H
