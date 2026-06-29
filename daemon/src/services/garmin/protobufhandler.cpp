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
    if (fragment.isNull()) {
        qDebug() << Q_FUNC_INFO << "Garmin: processChunkedMessage returned Null";
        return  QSharedPointer<GarminProtobufMessage>();
    }
    if (fragment->isComplete()) {
        qDebug() << Q_FUNC_INFO << "Garmin: Protobuf message is complete" << message->getMessageBytes().toHex();
        mChunkedFragmentsMap.remove(message->getRequestId());
        // Message is complet now, start parsing
        QByteArray protobufPayload = fragment->getFragmentBytes();

        const quint8 firstTag = (quint8)protobufPayload[0];
        const quint8 fieldNumber = firstTag >> 3;
        const quint8 wireType = firstTag & 0x07;

        qDebug() << Q_FUNC_INFO << "Garmin: First protobuf field:" << fieldNumber
                << "(wire type:" << wireType << ") Payload: " << protobufPayload.toHex();

        bool processed=false;
        if (fieldNumber==8 && wireType ==2) {
             // Device Status Message
             processed = true;
             quint8  innerLength=protobufPayload[1];
             GarminDeviceStatusMessage* msg = new GarminDeviceStatusMessage(mCommunicator);
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
             qDebug() << Q_FUNC_INFO << "Garmin: Add ACK with No error";

             QSharedPointer<GarminProtobufStatusMessage> statusMessage = QSharedPointer<GarminProtobufStatusMessage>
                     (new GarminProtobufStatusMessage( Status::Ack,
                                                  message->getRequestId(),
                                                  message->getDataOffset(),
                                                  ProtobufChunkStatus::KEPT,
                                                  ProtobufStatusCode::NO_ERROR,
                                                  true));
             message->setStatusMessage(statusMessage);
            } else {
             qDebug() << Q_FUNC_INFO << "Garmin: Add ACK with Unknown Request ID";
             QSharedPointer<GarminProtobufStatusMessage> statusMessage = QSharedPointer<GarminProtobufStatusMessage>
                     (new GarminProtobufStatusMessage(Status::Ack,
                                                  message->getRequestId(),
                                                  message->getDataOffset(),
                                                  ProtobufChunkStatus::DISCARDED,
                                                  ProtobufStatusCode::UNKNOWN_REQUEST_ID,
                                                  true));
             message->setStatusMessage(statusMessage);
         }
         return message;
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
    qDebug() << Q_FUNC_INFO << "Garmin: Process chunked message with Request ID " << message->getRequestId() << " content " << message->getMessageBytes().toHex();

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

QSharedPointer<GarminProtobufMessage> ProtobufHandler::prepareProtobufRequest(QByteArray protobufPayload) {
    if (protobufPayload.isEmpty())
        return QSharedPointer<GarminProtobufMessage>();
    int requestId = getNextProtobufRequestId();
    return prepareProtobufMessage(protobufPayload, MessageId::ProtobufRequest, requestId);
}

QSharedPointer<GarminProtobufMessage> ProtobufHandler::prepareProtobufResponse(QByteArray protobufPayload, int requestId) {
    if (protobufPayload.isEmpty())
        return QSharedPointer<GarminProtobufMessage>();
    return prepareProtobufMessage(protobufPayload, MessageId::ProtobufResponse, requestId);
}

QSharedPointer<GarminProtobufMessage> ProtobufHandler::prepareProtobufMessage(QByteArray bytes, MessageId type, int requestId) {
    // This needs probably to be splitted into to functions to cater for request/response
    qDebug() << Q_FUNC_INFO << messageIdToString((quint16)type).value() << " ID " << requestId;
    if (bytes.isEmpty())
        return QSharedPointer<GarminProtobufMessage>();

    if (bytes.size() > mMaxChunkSize) {
        mChunkedFragmentsMap.insert(requestId, QSharedPointer<ProtobufFragment>(new ProtobufFragment(bytes)));
        return QSharedPointer<GarminProtobufMessage>(new GarminProtobufMessage(mCommunicator,
                requestId,
                0,
                bytes.size(),
                mMaxChunkSize,
                bytes.mid(0, mMaxChunkSize)));
    }
    return QSharedPointer<GarminProtobufMessage>(new GarminProtobufMessage(mCommunicator, requestId, 0, bytes.size(), bytes.size(), bytes));
}
