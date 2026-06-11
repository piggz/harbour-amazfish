#include "garminauthnegotiationstatusmessage.h"

enum AuthNegotiationStatus {
    GUESS_OK,
    GUESS_KO,
};

void GarminAuthNegotiationStatusMessage::parse(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO << "Garmin: Authentication negotiation data " << data.toHex();
    // First two bytes are orignal message ID
    // Third byte is status
    status = static_cast<Status>(data[2]);
    quint8 authNegotiationStatusCode = data[3];
    if (authNegotiationStatusCode>1)
    {
        qDebug() << Q_FUNC_INFO << "Garmin: Unknown Authentication negotiation status " << authNegotiationStatusCode;
        return;
    }
    authNegotiationStatus = static_cast<AuthNegotiationStatus>(authNegotiationStatusCode);
    unk = data[4];
    authFlags = u32le(data.constData(),3);

    //now send response
    QByteArray respMsg=generateResponse();
    respMsg=wrapInGfdiEnvelope(static_cast<quint16>(MessageId::Response),respMsg);
    if (mCommunicator) mCommunicator->sendMessage("AUTH NEGOTIATION",respMsg);
 }

QByteArray GarminAuthNegotiationStatusMessage::generateResponse() {
    QByteArray resp;
    writeU16le(resp,static_cast<quint16>(MessageId::AuthNegotiation));
    resp.append(static_cast<char>(status));
    resp.append(static_cast<char>(authNegotiationStatus));
    resp.append(unk);
    writeU32le(resp,authFlags);
    return resp;
}
