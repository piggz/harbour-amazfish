#include "garminprotobufmessage.h"
#include "garmindevicestatusmessage.h"
#include "garmincalendarmessage.h"
#include "garminconfigurationmessage.h"
#include "garminauthnegotiationmessage.h"


GarminProtobufMessage::GarminProtobufMessage(CommunicatorV2 *com, int requestId, int dataOffset, int totalProtobufLength, int protobufDataLength, QByteArray messageBytes, bool sendOutgoing) {
    mCommunicator = com;
    mRequestId=requestId;
    mDataOffset=dataOffset;
    mTotalProtobufLength=totalProtobufLength;
    mProtobufDataLength=protobufDataLength;
    mMessageBytes=messageBytes;
    mSendOutgoing=sendOutgoing;
    mMessageType=MessageId::ProtobufRequest;
}

void GarminProtobufMessage::setStatusMessage(QSharedPointer<GarminProtobufStatusMessage> protobufStatusMessage) {
    qDebug() << Q_FUNC_INFO;
    mStatusMessage=protobufStatusMessage;
}


QSharedPointer<GarminProtobufMessage> GarminProtobufMessage::parse()
{
    qDebug() << Q_FUNC_INFO << "Garmin: Protobuf request " << mMessageBytes.toHex();
    QByteArray data = mMessageBytes;
    quint16 requestID = u16le(data,0);
    quint32 dataOffset = u32le(data,2);
    quint32 totalProtobufLength = u32le(data,6);
    quint32 protobufDataLength = u32le(data,10);
    QByteArray messageBytes = data.mid(14,protobufDataLength);

    qDebug() << Q_FUNC_INFO << "Garmin: Protobuf request " << requestID << " total length " << totalProtobufLength << " << data " << messageBytes.toHex();
    return QSharedPointer<GarminProtobufMessage>(new GarminProtobufMessage(mCommunicator, requestID, dataOffset, totalProtobufLength, protobufDataLength, messageBytes, false));

 }

void GarminProtobufMessage::handleAuthenticationRequest(quint16 requestId) {
    qDebug() << Q_FUNC_INFO << "Garmin: Sending back 0 as we don't know what else to do";

     QByteArray responsePayload;
     writeU16le(responsePayload, requestId);
     responsePayload.append(char(0));
     writeU32le(responsePayload,0);

     const QByteArray response =
         wrapInGfdiEnvelope(5101, responsePayload);

     if (mCommunicator) {
         bool queueRes=mCommunicator->sendMessage("AUTHENTICATION SERVICE RESPONSE",response);
         if (!queueRes) {
             qDebug() << Q_FUNC_INFO << "Garmin: Failed to queue ACK:";
         } else {
             qDebug() << Q_FUNC_INFO << "Garmin: Authentication Service Response sent";
         }

     }
}

void GarminProtobufMessage::handleCalendarRequest(const QByteArray& data, quint16 requestID, quint32 dataOffset)
{
    GarminCalendarMessage* calendarMsg = new GarminCalendarMessage(mCommunicator);
    calendarMsg->parse(data, requestID, dataOffset);
}

void GarminProtobufMessage::sendGenericAck(const QByteArray& data) {
    const quint16 requestId = u16le(data, 0);
    const quint32 dataOffset = u32le(data, 2);

    QByteArray responsePayload;
    responsePayload.append(char(0xB3));
    responsePayload.append(char(0x13));
    responsePayload.append(char(0x00)); // ACK
    writeU16le(responsePayload, requestId);
    writeU32le(responsePayload, dataOffset);
    responsePayload.append(char(0x00)); // KEPT
    responsePayload.append(char(0x00)); // NO_ERROR

    const QByteArray response =
        wrapInGfdiEnvelope(5000, responsePayload);

    qDebug() << Q_FUNC_INFO << "Garmin: Unknown Protobuf detected, sending generic ACK";

    if (mCommunicator){
        bool queueRes = mCommunicator->sendMessage("GENERIC ACK",response);
        if (!queueRes) {
            qDebug() << Q_FUNC_INFO << "Garmin: Failed to send ACK:";
        } else {
            qDebug() << Q_FUNC_INFO << "Garmin: Generc ACK sent - watch should continue!";
        }
    }
}

bool GarminProtobufMessage::isChunked() {
      return (mTotalProtobufLength != mProtobufDataLength);
  }

bool GarminProtobufMessage::isComplete() {
        return (mDataOffset == 0 && !isChunked());
    }

QByteArray GarminProtobufMessage::getOutgoingMessage(){
    QByteArray message;
    writeU16le(message, mRequestId);
    writeU32le(message, mDataOffset);
    writeU32le(message,mTotalProtobufLength);
    writeU32le(message,mProtobufDataLength);
    message.append(mMessageBytes);
    qDebug() << Q_FUNC_INFO << "Garmin: outgoing protobuf message " << message.toHex();
    return wrapInGfdiEnvelope((quint16)MessageId::ProtobufRequest, message);
}

QByteArray GarminProtobufMessage::getAckByteStream() {
    qDebug()<< Q_FUNC_INFO << "Garmin: getting Ack Byte Stream";
    if (mStatusMessage.isNull()) {
        qDebug()<< Q_FUNC_INFO << "Garmin: No status message";
        return QByteArray();
    }
    qDebug()<< Q_FUNC_INFO << "Garmin: status message is " << mStatusMessage->getOutgoingMessage().toHex();
    return mStatusMessage->getOutgoingMessage();
}
GarminProtobufStatusMessage::GarminProtobufStatusMessage(Status status, int requestId, int dataOffset, ProtobufChunkStatus chunkStatus, ProtobufStatusCode  code, bool sendoutgoing)
{
    qDebug() << Q_FUNC_INFO;
    mStatus = status;
    mRequestId=requestId;
    mDataOffset=dataOffset;
    mProtobufStatusCode=code;
    mProtobufChunkStatus=chunkStatus;
    mSendOutgoing=sendoutgoing;

}


QSharedPointer<GarminProtobufStatusMessage> GarminProtobufStatusMessage::parse() {
    qDebug() << Q_FUNC_INFO << "Garmin: Status message data " << mMessageBytes.toHex();

    Status status = (Status) mMessageBytes.data()[0];


    const quint16 requestId = u16le(mMessageBytes, 1);
    const quint32 dataOffset = u32le(mMessageBytes, 3);
    char s = mMessageBytes[7];
    ProtobufChunkStatus protobufStatus = (ProtobufChunkStatus)s;
    s=mMessageBytes[8];
    ProtobufStatusCode error = (ProtobufStatusCode)s;
    qDebug() << Q_FUNC_INFO << "Garmin: Processing protobuf status message " << (char)status  << "#" <<  requestId<< "status " << (char)error;

    QSharedPointer<GarminProtobufStatusMessage> statusMessage = QSharedPointer<GarminProtobufStatusMessage>( new GarminProtobufStatusMessage(status, requestId, dataOffset, protobufStatus, error, true));
    if (!statusMessage.isNull())
        qDebug() << Q_FUNC_INFO << "Garmin: Processing protobuf status message " << statusMessage  << "#" <<  getRequestId() << "status " <<  statusMessage->getProtobufChunkStatus() << " error=" << statusMessage->getProtobufStatusCode();
    return statusMessage;
}

QByteArray GarminProtobufStatusMessage::getOutgoingMessage() {
    if (!mSendOutgoing) return QByteArray();
    QByteArray response;
    writeU16le(response,(quint16) mMessageType);
    response.append((char)mStatus);
    writeU16le(response, mRequestId);
    writeU32le(response,mDataOffset);
    response.append((char)mProtobufChunkStatus);
    response.append((char)mProtobufStatusCode);
    response=wrapInGfdiEnvelope((quint16)MessageId::Response,response);
    return response;
   }
