#include "garminauthnegotiationmessage.h"

enum AuthNegotiationStatus {
    GUESS_OK,
    GUESS_KO,
};

void GarminAuthNegotiationMessage::parse(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO << "Garmin: Authentication negotiation data " << data.toHex();
    // First two bytes are orignal message ID
    // first byte is protobuf byte
    // second byte is status
 //   status = static_cast<Status>(data[1]);

    authNegotiationStatus= data[0];
    if (authNegotiationStatus>1)
    {
        qDebug() << Q_FUNC_INFO << "Garmin: Unknown Authentication negotiation status " << authNegotiationStatus;
        return;
    }

    authFlags = u32le(data.constData(),1);

    //first send status message as ack
    QByteArray statusMsg=generateStatusMessage();
    statusMsg=wrapInGfdiEnvelope(MessageId::Response,statusMsg);
    if (mCommunicator) mCommunicator->sendMessage("AUTH NEGOTIATION STATUS",statusMsg);
    //now send response
    QByteArray respMsg=generateOutgoing();
    //respMsg=wrapInGfdiEnvelope(MessageId::AuthNegotiation,respMsg);
    respMsg=wrapInGfdiEnvelope(MessageId::Response,respMsg);
    if (mCommunicator) mCommunicator->sendMessage("AUTH NEGOTIATION RESPONSE",respMsg);
 }


QByteArray GarminAuthNegotiationMessage::generateStatusMessage() {
    QByteArray resp;
    writeU16le(resp,static_cast<quint16>(MessageId::AuthNegotiation));
    resp.append(static_cast<char>(Status::Ack));
    resp.append(static_cast<char>(AuthNegotiationStatus::GUESS_OK));
    resp.append(authNegotiationStatus);
    writeU32le(resp,authFlags);
    return resp;

}

QByteArray GarminAuthNegotiationMessage::generateOutgoing() {
    QByteArray resp;
    resp.append(static_cast<char>(0));
    writeU32le(resp,authFlags);
    return resp;
}
