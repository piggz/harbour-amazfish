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
    return wrapInGfdiEnvelope((quint16)MessageId::ProtobufRequest, message);
}


void GarminProtobufStatusMessage::parse() {
    QByteArray data = mMessageBytes;
    if (data.size() >= 18) {
        const quint16 requestId = u16le(data, 0);
        const quint32 dataOffset = u32le(data, 2);
        const quint32 totalProtobufLength = u32le(data, 6);
        const quint32 protobufDataLength = u32le(data, 10);
        QByteArray messageBytes = data.mid(14,protobufDataLength);

        QSharedPointer<GarminProtobufMessage> message = QSharedPointer<GarminProtobufMessage>( new GarminProtobufMessage(mCommunicator, requestId, dataOffset, totalProtobufLength, protobufDataLength, messageBytes, false));


        qDebug() << Q_FUNC_INFO << "Garmin: Request ID:" << requestId;
        qDebug() << Q_FUNC_INFO << "Garmin: Data Offset:" << dataOffset;
        qDebug() << Q_FUNC_INFO << "Garmin: Total Protobuf Length:" << totalProtobufLength;
        qDebug() << Q_FUNC_INFO << "Garmin: Protobuf Data Length:" << protobufDataLength;

        const int protobufStart = 14;
        const int protobufEnd = protobufStart + static_cast<int>(protobufDataLength);

        if (data.size() >= protobufEnd) {
            const QByteArray protobufPayload = data.mid(protobufStart, protobufDataLength);
            const bool isComplete =
                (dataOffset == 0 && totalProtobufLength == protobufDataLength);

            if (isComplete) {
                qDebug() << Q_FUNC_INFO << "Garmin: Complete protobuf message - attempting to parse";

                if (!protobufPayload.isEmpty()) {
                    const quint8 firstTag = static_cast<quint8>(protobufPayload[0]);
                    const quint8 fieldNumber = firstTag >> 3;
                    const quint8 wireType = firstTag & 0x07;


                    qDebug() << Q_FUNC_INFO << "Garmin: First protobuf field:" << fieldNumber
                            << "(wire type:" << wireType << ") Payload: " << protobufPayload.toHex();

                    if (fieldNumber==8 && wireType ==2)
                    { //get length of inner protobuf
                        quint8  innerLength=protobufPayload[1];
                        GarminDeviceStatusMessage* msg = new GarminDeviceStatusMessage(mCommunicator);
                        msg->parse(protobufPayload.mid(2,innerLength));
                  }
                }
            }
        }
    }
}

