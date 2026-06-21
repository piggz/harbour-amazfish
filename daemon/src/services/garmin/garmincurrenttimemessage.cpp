#include "garmincurrenttimemessage.h"

#include "garmintypes.h"

#include "QDateTime"

void GarminCurrentTimeMessage::parse(const QByteArray& data) {
    // no parsing needed as no incoming information
    // simply send back the current time as response
    QByteArray response = generateOutgoing(data);
    response=wrapInGfdiEnvelope(5000,response);
    if (mCommunicator) mCommunicator->sendMessage("CURRENT TIME", response);
}

QByteArray GarminCurrentTimeMessage::generateOutgoing(const QByteArray& data) {
    // Generate a CurrentTimeRequest response with current time
    QByteArray r;
    // Unix seconds -> Garmin epoch (Dec 31 1989) offset 631065600

    quint32 unixNow = quint32(QDateTime::currentMSecsSinceEpoch()/1000);
    quint32 garminTime = unixNow - 631065600u;
    quint32 refid=u32le(data,0);

    //TODO: Fix this.
    QTimeZone tz = QTimeZone::systemTimeZone();
    int timeZoneOffset = tz.offsetFromUtc(QDateTime::currentDateTime());
    int nextTransitionEndsGarminTs =0;
    int nextTransitionStartsGarminTs = 0;


    // Original message ID: CURRENT_TIME_REQUEST (5052)
    writeU16le(r, 5052);
    // Status: ACK
    r.append(char(quint8(Status::Ack)));
    // Now referenceid(32bit)
    writeU32le(r,refid);
    //Now Garmin Time
    writeU32le(r, garminTime);
    //now timezoneoffset
    writeU32le(r,timeZoneOffset);
    //nexttransitionendsgarmints
    writeU32le(r,nextTransitionEndsGarminTs);
    //nexttransitionstartsgarmints
    writeU32le(r,nextTransitionStartsGarminTs);
    return r;
}
