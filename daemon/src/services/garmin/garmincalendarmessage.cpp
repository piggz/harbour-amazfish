#include "garmincalendarmessage.h"
#include "libwatchfish/calendarsource.h"

#include <QtCore/QDebug>


// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------
static QString truncateByChars(const QString& s, quint32 maxLen)
{
    if (maxLen == 0 || s.size() <= int(maxLen)) {
        return s;
    }
    return s.left(int(maxLen));
}

// -----------------------------------------------------------------------------
// Decode protobuf varint
// Returns: (value, bytesConsumed)
// -----------------------------------------------------------------------------
Result<QPair<quint64, int>> GarminCalendarMessage::decodeVarint(const QByteArray& data)
{
    quint64 result = 0;
    int shift = 0;
    int bytesRead = 0;

    for (auto ch : data) {
        const quint8 byte = quint8(ch);
        ++bytesRead;

        result |= (quint64(byte & 0x7F) << shift);

        if ((byte & 0x80) == 0) {
            return Result<QPair<quint64, int>>::isOk(qMakePair(result, bytesRead));
        }

        shift += 7;
        if (shift >= 64) {
            return Result<QPair<quint64, int>>::err(
                GarminError::invalidMessage(QStringLiteral("Varint too long")));
        }

        if (bytesRead >= 10) {
            break;
        }
    }

    return Result<QPair<quint64, int>>::err(
        GarminError::invalidMessage(QStringLiteral("Incomplete varint")));
}

// -----------------------------------------------------------------------------
// Encode protobuf varint
// -----------------------------------------------------------------------------
void GarminCalendarMessage::encodeVarint(QByteArray& buffer, quint64 value)
{
    while (true) {
        quint8 byte = quint8(value & 0x7F);
        value >>= 7;

        if (value != 0) {
            byte |= 0x80;
        }

        buffer.append(char(byte));

        if (value == 0) {
            break;
        }
    }
}

// -----------------------------------------------------------------------------
// Encode field key
// -----------------------------------------------------------------------------
void GarminCalendarMessage::encodeFieldKey(QByteArray& buffer, quint32 fieldNum, quint8 wireType)
{
    const quint32 key = (fieldNum << 3) | quint32(wireType);
    encodeVarint(buffer, key);
}

// -----------------------------------------------------------------------------
// Parse one protobuf field from data[startOffset...]
// Returns fieldNum, wireType, fieldData (payload only), nextCursor absolute offset
// -----------------------------------------------------------------------------
bool GarminCalendarMessage::parseField(const QByteArray& data,
                int startOffset,
                quint32& fieldNum,
                quint8& wireType,
                QByteArray& fieldData,
                int& nextCursor)
{
    if (startOffset >= data.size()) {
        return false;
    }

    const QByteArray tail = data.mid(startOffset);

    auto keyRes = decodeVarint(tail);
    if (!keyRes.ok) {
        return false;
    }

    const quint64 key = keyRes.value.first;
    const int keyLen = keyRes.value.second;

    fieldNum = quint32(key >> 3);
    wireType = quint8(key & 0x07);

    int cursor = startOffset + keyLen;

    switch (wireType) {
    case 0: {
        // varint payload
        auto valueRes = decodeVarint(data.mid(cursor));
        if (!valueRes.ok) {
            return false;
        }

        const int valueLen = valueRes.value.second;
        fieldData = data.mid(cursor, valueLen);
        nextCursor = cursor + valueLen;
        return true;
    }

    case 2: {
        // length-delimited
        auto lenRes = decodeVarint(data.mid(cursor));
        if (!lenRes.ok) {
            return false;
        }

        const quint64 length = lenRes.value.first;
        const int lenLen = lenRes.value.second;

        cursor += lenLen;

        const int endPos = cursor + int(length);
        if (endPos > data.size()) {
            qWarning().noquote()
                << QStringLiteral("Length-delimited field claims %1 bytes but only %2 available (field %3, cursor %4)")
                       .arg(length)
                       .arg(data.size() - cursor)
                       .arg(fieldNum)
                       .arg(cursor);
            return false;
        }

        fieldData = data.mid(cursor, int(length));
        nextCursor = endPos;
        return true;
    }

    default:
        qWarning() << "Unknown wire type:" << wireType;
        return false;
    }
}

// -----------------------------------------------------------------------------
// Parse GarminCalendarMessage from raw Smart protobuf payload
// -----------------------------------------------------------------------------
void GarminCalendarMessage::parse(const QByteArray& data, quint16 requestId, quint32 dataOffset)
{
    qDebug() << "Parsing calendar request from" << data.size() << "bytes";

    CalendarServiceRequest request;


    int cursor = 0;
    QByteArray calendarServiceData;
    int loopCount = 0;

    // Find Smart.field1 = CalendarService
    while (cursor < data.size()) {
        ++loopCount;
        if (loopCount > 100) {
            break;
        }

        const int oldCursor = cursor;
        quint32 fieldNum = 0;
        quint8 wireType = 0;
        QByteArray fieldData;
        int nextCursor = 0;

        if (parseField(data, cursor, fieldNum, wireType, fieldData, nextCursor)) {
            qDebug() << Q_FUNC_INFO << "Garmin: Smart message field" << fieldNum << ", wire type" << wireType;

            // field 1 is CalendarService
            if (fieldNum == 1) {
                qDebug() << Q_FUNC_INFO << "Garmin: Found CalendarService field 1";
                calendarServiceData = fieldData;
                break;
            }

            cursor = nextCursor;
            if (cursor == oldCursor) {
                qDebug() << Q_FUNC_INFO << "Garmin: Parse Calendar Request: cursor not advancing in Smart wrapper";
                break;
            }
        } else {
            qDebug() << Q_FUNC_INFO << "Garmin: parseCalendarRequest: failed parsing Smart wrapper field";
            break;
        }
    }

    if (calendarServiceData.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Garmin: No CalendarServiceField found";
        return;
    }

    request.useCoreServiceEnvelope = false;

    qDebug() << "Found CalendarService payload:" << calendarServiceData.size() << "bytes";

    // Parse CalendarService.field1 = GarminCalendarMessage
    cursor = 0;
    QByteArray requestBytes;
    loopCount = 0;

    while (cursor < calendarServiceData.size()) {
        ++loopCount;
        if (loopCount > 100) {
            break;
        }

        const int oldCursor = cursor;
        quint32 fieldNum = 0;
        quint8 wireType = 0;
        QByteArray fieldData;
        int nextCursor = 0;

        if (parseField(calendarServiceData, cursor, fieldNum, wireType, fieldData, nextCursor)) {
            if (fieldNum == 1) {
                requestBytes = fieldData;
                break;
            }

            cursor = nextCursor;
            if (cursor == oldCursor) {
                qDebug() << Q_FUNC_INFO << "Garmin: Parse Calendar Request: cursor not advancing in CalendarService";
                break;
            }
        } else {
            break;
        }
    }

    if (requestBytes.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Garmin: No CalendarMessage field found";
        return;
    }

    // Parse request fields
    cursor = 0;
    loopCount = 0;

    while (cursor < requestBytes.size()) {
        ++loopCount;
        if (loopCount > 100) {
            break;
        }

        const int oldCursor = cursor;
        quint32 fieldNum = 0;
        quint8 wireType = 0;
        QByteArray fieldData;
        int nextCursor = 0;

        if (parseField(requestBytes, cursor, fieldNum, wireType, fieldData, nextCursor)) {
            auto varintRes = decodeVarint(fieldData);
            if (varintRes.ok) {
                const quint64 value = varintRes.value.first;

                switch (fieldNum) {
                case 1: request.startDate = value; break;
                case 2: request.endDate = value; break;
                case 3: request.includeOrganizer = (value != 0); break;
                case 4: request.includeTitle = (value != 0); break;
                case 5: request.includeLocation = (value != 0); break;
                case 6: request.includeDescription = (value != 0); break;
                case 7: request.includeStartDate = (value != 0); break;
                case 8: request.includeEndDate = (value != 0); break;
                case 9: request.includeAllDay = (value != 0); break;
                case 10: request.maxOrganizerLength = quint32(value); break;
                case 11: request.maxTitleLength = quint32(value); break;
                case 12: request.maxLocationLength = quint32(value); break;
                case 13: request.maxDescriptionLength = quint32(value); break;
                case 14: request.maxEvents = quint32(value); break;
                default:
                    qDebug() << "Unknown GarminCalendarMessage field:" << fieldNum;
                    break;
                }
            }

            cursor = nextCursor;
            if (cursor == oldCursor) {
                qCritical() << "parseCalendarRequest: cursor not advancing in request body";
                break;
            }
        } else {
            break;
        }
    }

    qDebug().noquote()
        << QStringLiteral("Parsed calendar request: start_date=%1, end_date=%2, max_events=%3, envelope=CalendarService(field 1)")
               .arg(request.startDate)
               .arg(request.endDate)
               .arg(request.maxEvents);

    // send Ack for the request
    qDebug() << Q_FUNC_INFO << "Garmin: Sending ACK for calendar request...";

    QByteArray ackPayload;
    ackPayload.append(char(0xB3));
    ackPayload.append(char(0x13));
    ackPayload.append(char(0x00)); // ACK
    writeU16le(ackPayload, requestId);
    writeU32le(ackPayload, dataOffset);
    ackPayload.append(char(0x00)); // ProtobufChunkStatus: KEPT
    ackPayload.append(char(0x00)); // ProtobufStatusCode: NO_ERROR

    const QByteArray ackResponse =
        wrapInGfdiEnvelope(5000, ackPayload);

    if (mCommunicator) mCommunicator->sendMessage("CALENDER REQUEST ACK", ackResponse);

    //Get calendar items from watchfish
    auto calendarProto = handleCalendarRequest(request);
    if (calendarProto.ok)
    {
        QByteArray responseData = encodeCalendarResponse(calendarProto.value,CalendarResponseStatus::Ok,requestId,request.useCoreServiceEnvelope);
        QByteArray response =  wrapInGfdiEnvelope(0x13B4, responseData);
        if (mCommunicator) mCommunicator->sendMessage("CALENDAR RESPONSE", response);
    }
    else {
        const QByteArray responseData =
            encodeCalendarResponse(
                {},
                CalendarResponseStatus::InvalidDateRange,
                requestId,
                request.useCoreServiceEnvelope);

        QByteArray response = wrapInGfdiEnvelope(0x13B4, responseData);
        if (mCommunicator) mCommunicator->sendMessage("EMPTY CALENDAR RESPONSE",response);
    }
}

// -----------------------------------------------------------------------------
// Handle calendar request via CalendarManager
//
// -----------------------------------------------------------------------------
Result<QVector<CalendarEventProto>> GarminCalendarMessage::handleCalendarRequest(
    const CalendarServiceRequest& request)
{
    watchfish::CalendarSource* calendarManager = new watchfish::CalendarSource();
    qInfo() << "Handling calendar request:" << request.startDate << "to" << request.endDate;

    if (calendarManager == nullptr) {
        qWarning() << "No calendar manager available";
        return Result<QVector<CalendarEventProto>>::isOk({});
    }

    static constexpr quint64 SECONDS_PER_DAY = 86400;
    const quint64 endOfLastDay =
        ((request.endDate / SECONDS_PER_DAY) + 1) * SECONDS_PER_DAY - 1;

    QDate start= QDateTime::fromMSecsSinceEpoch(request.startDate).date();

    QDate end= QDateTime::fromMSecsSinceEpoch(endOfLastDay).date();

    QList<watchfish::CalendarEvent> events = calendarManager->fetchEvents(
        start,
        end
    );


    if (events.isEmpty()) {
         qDebug() << Q_FUNC_INFO << "Failed to fetch calendar events";
         return Result<QVector<CalendarEventProto>>::err(
             GarminError::invalidMessage(
                 QStringLiteral("Failed to fetch calendar events:")));
    }

    qInfo() << "Fetched" << events.size() << "calendar events";

    QVector<CalendarEventProto> protoEvents;
    protoEvents.reserve(events.size());

    for ( watchfish::CalendarEvent event : events) {
        CalendarEventProto proto;

        // title
        const QString title = truncateByChars(event.title(), request.maxTitleLength);
        proto.title = request.includeTitle ? title : QString();

        // location
        if (request.includeLocation && !event.location().isEmpty()) {
            proto.hasLocation = true;
            proto.location = truncateByChars(event.location(), request.maxLocationLength);
        }

        // description
        if (request.includeDescription && !event.description().isEmpty()) {
            proto.hasDescription = true;
            proto.description = truncateByChars(event.description(), request.maxDescriptionLength);
        }

        // organizer is not part of watchfish events so setting to false

        proto.hasOrganizer = true;

        proto.startDate = event.start();
        proto.endDate = event.end();
        proto.allDay = event.allDay();

        // reminders are not part of watchfish events so skipping
        /*
        proto.reminderTimes.reserve(event.reminders.size());
        for (auto reminder : event.reminders) {
            proto.reminderTimes.append(quint32(reminder));
        }
        */

        protoEvents.append(proto);

        if (protoEvents.size() >= int(request.maxEvents)) {
            qDebug() << "Reached maxEvents limit:" << request.maxEvents;
            break;
        }
    }

    qInfo() << "Returning" << protoEvents.size() << "calendar events";
    return Result<QVector<CalendarEventProto>>::isOk
                                              (protoEvents);
}

// -----------------------------------------------------------------------------
// Encode one CalendarEvent protobuf
// -----------------------------------------------------------------------------
QByteArray GarminCalendarMessage::encodeCalendarEvent(const CalendarEventProto& event)
{
    QByteArray buf;

    // Field 1: organizer (optional string)
    if (event.hasOrganizer) {
        const QByteArray bytes = event.organizer.toUtf8();
        encodeFieldKey(buf, 1, 2);
        encodeVarint(buf, quint64(bytes.size()));
        buf.append(bytes);
    }

    // Field 2: title (string)
    {
        const QByteArray bytes = event.title.toUtf8();
        encodeFieldKey(buf, 2, 2);
        encodeVarint(buf, quint64(bytes.size()));
        buf.append(bytes);
    }

    // Field 3: location (optional string)
    if (event.hasLocation) {
        const QByteArray bytes = event.location.toUtf8();
        encodeFieldKey(buf, 3, 2);
        encodeVarint(buf, quint64(bytes.size()));
        buf.append(bytes);
    }

    // Field 4: description (optional string)
    if (event.hasDescription) {
        const QByteArray bytes = event.description.toUtf8();
        encodeFieldKey(buf, 4, 2);
        encodeVarint(buf, quint64(bytes.size()));
        buf.append(bytes);
    }

    // Field 5: start_date
    encodeFieldKey(buf, 5, 0);
    encodeVarint(buf, event.startDate.toMSecsSinceEpoch()*1000 - 631065600u);

    // Field 6: end_date
    encodeFieldKey(buf, 6, 0);
    encodeVarint(buf, event.endDate.toMSecsSinceEpoch()*1000 - 631065600u);

    // Field 7: all_day
    encodeFieldKey(buf, 7, 0);
    encodeVarint(buf, event.allDay ? 1 : 0);

    // Field 8: repeated reminder_time_in_secs
    for (quint32 reminder : event.reminderTimes) {
        encodeFieldKey(buf, 8, 0);
        encodeVarint(buf, quint64(reminder));
    }

    return buf;
}

// -----------------------------------------------------------------------------
// Encode full calendar response
//
// Format preserved from Rust:
// [requestId:2][dataOffset:4][totalProtobufLength:4][protobufDataLength:4][protobufPayload]
// -----------------------------------------------------------------------------
QByteArray GarminCalendarMessage::encodeCalendarResponse(
    const QVector<CalendarEventProto>& events,
    CalendarResponseStatus status,
    quint16 requestId,
    bool useCoreServiceEnvelope)
{
    Q_UNUSED(useCoreServiceEnvelope);

    qDebug().noquote()
        << QStringLiteral("Encoding calendar response with %1 events, envelope=CalendarService(field 1)")
               .arg(events.size());

    // CalendarEventsResponse
    QByteArray responseBuf;

    // Field 1: status
    encodeFieldKey(responseBuf, 1, 0);
    encodeVarint(responseBuf, quint64(status));

    // Field 2: repeated events
    for (const CalendarEventProto& event : events) {
        const QByteArray eventBuf = encodeCalendarEvent(event);
        encodeFieldKey(responseBuf, 2, 2);
        encodeVarint(responseBuf, quint64(eventBuf.size()));
        responseBuf.append(eventBuf);
    }

    // CalendarService.field2 = CalendarEventsResponse
    QByteArray calendarServiceBuf;
    encodeFieldKey(calendarServiceBuf, 2, 2);
    encodeVarint(calendarServiceBuf, quint64(responseBuf.size()));
    calendarServiceBuf.append(responseBuf);

    // Smart.field1 = CalendarService
    QByteArray smartBuf;
    encodeFieldKey(smartBuf, 1, 2);
    encodeVarint(smartBuf, quint64(calendarServiceBuf.size()));
    smartBuf.append(calendarServiceBuf);

    // Final protobuf response payload
    QByteArray message;

    // request_id LE16
    message.append(char(requestId & 0xFF));
    message.append(char((requestId >> 8) & 0xFF));

    // dataOffset LE32 = 0
    for (int i = 0; i < 4; ++i) {
        message.append(char(0));
    }

    const quint32 protobufLength = quint32(smartBuf.size());

    // totalProtobufLength LE32
    message.append(char(protobufLength & 0xFF));
    message.append(char((protobufLength >> 8) & 0xFF));
    message.append(char((protobufLength >> 16) & 0xFF));
    message.append(char((protobufLength >> 24) & 0xFF));

    // protobufDataLength LE32
    message.append(char(protobufLength & 0xFF));
    message.append(char((protobufLength >> 8) & 0xFF));
    message.append(char((protobufLength >> 16) & 0xFF));
    message.append(char((protobufLength >> 24) & 0xFF));

    // protobuf bytes
    message.append(smartBuf);

    qDebug().noquote()
        << QStringLiteral("Encoded calendar response: %1 bytes total (header: 14, protobuf: %2)")
               .arg(message.size())
               .arg(smartBuf.size());

    return message;
}

