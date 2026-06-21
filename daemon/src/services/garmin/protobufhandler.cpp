#include "protobufhandler.h"
#include "garminprotobufmessage.h"
#include "garmindevicestatusmessage.h"

const int MAX_CHUNK_SIZE = 375;

ProtobufFragment::ProtobufFragment(QByteArray fragmentBytes)
{
    mFragmentBytes=fragmentBytes;
    mTotalLength = fragmentBytes.size();
}

ProtobufFragment::ProtobufFragment(QSharedPointer<GarminProtobufMessage> message)
{
    if (message->getDataOffset() !=0)
    {
        qDebug() << Q_FUNC_INFO << "Garmin: Cannot create fragment if message is not the first of the sequence";
        return;
    }
    mFragmentBytes = message->getMessageBytes();
    mTotalLength = message->getTotalProtobufLength();
}

ProtobufFragment::ProtobufFragment(QSharedPointer<ProtobufFragment> existing, QSharedPointer<GarminProtobufMessage> toMerge)
{
    if (toMerge->getDataOffset() != existing->getFragmentBytes().size())
    {
        qDebug() << Q_FUNC_INFO << "Garmin: Cannot merge fragment: incoming message has different offset than needed";
        return;
    }
    mFragmentBytes=existing->getFragmentBytes();
    mFragmentBytes.append(toMerge->getMessageBytes());
    mTotalLength=existing->getTotalLength(); // Check if this is correct - could be mFragementbytes.size?
}


bool  ProtobufFragment::isComplete() {
    return mTotalLength == mFragmentBytes.size();
}


QSharedPointer<GarminProtobufMessage> ProtobufFragment::getNextChunk(QSharedPointer<GarminProtobufStatusMessage> protobufStatusMessage) {
    int start = protobufStatusMessage->getDataOffset() + MAX_CHUNK_SIZE;
    int length = std::min(MAX_CHUNK_SIZE, mFragmentBytes.size() - start);

    return QSharedPointer<GarminProtobufMessage>(new GarminProtobufMessage(protobufStatusMessage->getCommunicator(),
            protobufStatusMessage->getRequestId(),
            start,
            mTotalLength,
            length,
            mFragmentBytes.mid(start, start + length)));
}




int ProtobufHandler::getNextProtobufRequestId() {
        mLastProtobufRequestId = (mLastProtobufRequestId + 1) % 65536;
        return mLastProtobufRequestId;
    }

QSharedPointer<GarminProtobufMessage> ProtobufHandler::processIncoming(QSharedPointer<GarminProtobufMessage> message) {
    qDebug() << Q_FUNC_INFO;
    QSharedPointer<ProtobufFragment> fragment = processChunkedMessage(message);
    if (fragment->isComplete()) {
        qDebug() << Q_FUNC_INFO << "Garmin: Protobuf message is complete" << message->getMessageBytes().toHex();
        mChunkedFragmentsMap.remove(message->getRequestId());
        // Message is complet now, start parsing
        QByteArray protobufPayload = fragment->getFragmentBytes();
        const quint8 firstTag = static_cast<quint8>(protobufPayload[0]);
        const quint8 fieldNumber = firstTag >> 3;
        const quint8 wireType = firstTag & 0x07;

        qDebug() << Q_FUNC_INFO << "Garmin: First protobuf field:" << fieldNumber
                << "(wire type:" << wireType << ") Payload: " << protobufPayload.toHex();

        bool processed=false;
         if (fieldNumber==8 && wireType ==2) {
             // Device Status Message
             processed = true;
             quint8  innerLength=protobufPayload[1];
             GarminDeviceStatusMessage* msg = new GarminDeviceStatusMessage(mCommunicator.data());
             msg->parse(protobufPayload.mid(2,innerLength));
         }
         if (fieldNumber==49 && wireType ==2) {
             // Notification Service
             qDebug() << Q_FUNC_INFO << "Garmin: Got notifiction service messge";
         }
         if (fieldNumber==1 && wireType ==2) {
             // Calendar Service
             qDebug() << Q_FUNC_INFO << "Garmin: Got calendar service messge";
         }
         if (processed) {
             QByteArray responsePayload;
             responsePayload.append(char(0xB3));
             responsePayload.append(char(0x13));
             responsePayload.append(char(0x00)); // ACK
             writeU16le(responsePayload, message->getRequestId());
             writeU32le(responsePayload, message->getDataOffset());
             responsePayload.append(char(ProtobufChunkStatus::KEPT)); // KEPT
             responsePayload.append(char(ProtobufStatusCode::NO_ERROR)); // NO_ERROR
             if (mCommunicator) mCommunicator->sendMessage("PROTOBUF ACK",responsePayload);
         } else {
             QByteArray responsePayload;
             responsePayload.append(char(0xB3));
             responsePayload.append(char(0x13));
             responsePayload.append(char(0x00)); // ACK
             writeU16le(responsePayload, message->getRequestId());
             writeU32le(responsePayload, message->getDataOffset());
             responsePayload.append(char(ProtobufChunkStatus::DISCARDED));
             responsePayload.append(char(ProtobufStatusCode::UNKNOWN_REQUEST_ID));
             if (mCommunicator) mCommunicator->sendMessage("PROTOBUF UNKNOWN ACK",responsePayload);
         }
      }
    return QSharedPointer<GarminProtobufMessage>();
}

QSharedPointer<GarminProtobufMessage> ProtobufHandler::processIncoming(QSharedPointer<GarminProtobufStatusMessage> message) {
    //Todo: inform DataTransferhandler
    //DataTransferHandler->onDataChunkSuccessfullyReceived(message->getRequestId())
    if (mChunkedFragmentsMap.contains(message->getRequestId())) {
        QSharedPointer<ProtobufFragment> fragment = mChunkedFragmentsMap.value(message->getRequestId());
        if (fragment->getTotalLength() <= (message->getDataOffset() + mMaxChunkSize)) {
            mChunkedFragmentsMap.remove(message->getRequestId());
            return QSharedPointer<GarminProtobufMessage>();
        }
        return fragment->getNextChunk(message);
    }
    return QSharedPointer<GarminProtobufMessage>();
}

QSharedPointer<ProtobufFragment> ProtobufHandler::processChunkedMessage(QSharedPointer<GarminProtobufMessage> message) {
    qDebug() << Q_FUNC_INFO;
    if (message->isComplete()) //comment this out if for any reason also smaller messages should end up in the map
        return QSharedPointer<ProtobufFragment>(new ProtobufFragment(message->getMessageBytes()));

    if (message->getDataOffset() == 0) { //store new messages beginning at 0, overwrite old messages
        mChunkedFragmentsMap.insert(message->getRequestId(),QSharedPointer<ProtobufFragment>(new ProtobufFragment(message)  ));
        qDebug() << Q_FUNC_INFO << "Garmin: Protobuf request put in queue";
    } else {
        if (mChunkedFragmentsMap.contains(message->getRequestId())) {
            QSharedPointer<ProtobufFragment> oldFragment = mChunkedFragmentsMap.value(message->getRequestId());
            mChunkedFragmentsMap.insert(message->getRequestId(),
                    QSharedPointer<ProtobufFragment>(new ProtobufFragment(oldFragment, message)));
        }
    }
    return mChunkedFragmentsMap.value(message->getRequestId());
}

void ProtobufHandler::sendAck(QString taskName, QSharedPointer<GarminGfdiMessage> msg) {
    if (msg.isNull())
        return;
    if (!msg->getAckByteStream().isEmpty())
        qDebug() << Q_FUNC_INFO << "Garmin: OUTGOING ACK "<< (quint16) msg->getMessageType() << ", " << msg->getAckByteStream();
    if (mCommunicator) mCommunicator->sendMessage(taskName, msg->getAckByteStream());
}
