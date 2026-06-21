#ifndef GARMINPROTOBUFMESSAGE_H
#define GARMINPROTOBUFMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"

#include <QObject>

enum ProtobufChunkStatus { //based on the observations of the combination with the StatusCode
        KEPT,
        DISCARDED,
        };

enum ProtobufStatusCode {
    NO_ERROR = 0,
    UNKNOWN_REQUEST_ID= 100,
    DUPLICATE_PACKET = 101,
    MISSING_PACKET = 102,
    EXCEEDED_TOTAL_PROTOBUF_LENGTH = 103,
    PROTOBUF_PARSE_ERROR = 200,
    UNKNOWN = 201,
};

class GarminProtobufStatusMessage : public GarminGfdiMessage
{
public:
    /*
    GarminProtobufStatusMessage(CommunicatorV2 *parent) {
        mCommunicator = parent;
    }
*/
    explicit GarminProtobufStatusMessage( QByteArray data=QByteArray(), CommunicatorV2* parent=nullptr)
    {
        mMessageBytes=data;
        mCommunicator=parent;
    }

    GarminProtobufStatusMessage(CommunicatorV2 *com, int requestId, int dataOffset, int totalProtobufLength, int protobufDataLength, QByteArray messageBytes, bool sendOutgoing=true);

    void parse();
    int getRequestId() { return mRequestId; };
    int getDataOffset() { return mDataOffset; };
    MessageId getMessageType() {return MessageId::ProtobufResponse; };

    CommunicatorV2* getCommunicator() { return mCommunicator; };


private:
    int mRequestId;
    int mDataOffset;
    bool mSendOutgoing;
    int mTotalProtobufLength;
    int mProtobufDataLength;
    //QByteArray mMessageBytes;
};

class GarminProtobufMessage : public GarminGfdiMessage
{
public:
    /*
    GarminProtobufMessage(CommunicatorV2 *parent) {
        mCommunicator = parent;
    };
    */
    explicit GarminProtobufMessage( QByteArray data=QByteArray(), CommunicatorV2* parent=nullptr)
    {
        mMessageBytes=data;
        mCommunicator=parent;
        mMessageType=MessageId::ProtobufResponse;

    }
    GarminProtobufMessage(CommunicatorV2 *com, int requestId, int dataOffset, int totalProtobufLength, int protobufDataLength, QByteArray messageBytes, bool sendOutgoing=true);
    void setStatusMessage(QSharedPointer<GarminProtobufStatusMessage> protobufStatusMessage);

    QSharedPointer<GarminProtobufMessage> parse();

    int getDataOffset() { return mDataOffset; };
    //QByteArray getMessageBytes() { return mMessageBytes; };
    int getTotalProtobufLength() { return mTotalProtobufLength; };
    bool isChunked();
    bool isComplete();
    int getRequestId() { return mRequestId; };
    CommunicatorV2* getCommunicator() { return mCommunicator; };
    MessageId getMessageType() {return MessageId::ProtobufRequest; };
    QByteArray getOutgoingMessage();


private:
    void handleAuthenticationRequest(quint16 requestId);
    void handleCalendarRequest(const QByteArray& data, quint16 requestID, quint32 dataOffset);
    void sendGenericAck(const QByteArray& data);

    bool mSendOutgoing;
    int mRequestId;
    int mDataOffset;
    int mTotalProtobufLength;
    int mProtobufDataLength;
    QSharedPointer<GarminProtobufStatusMessage> mStatusMessage;
    QSharedPointer<GarminGfdiMessage> mGarminMessage;
};



#endif // GARMINPROTOBUFMESSAGE_H
