#include "garmingfdimessage.h"
#include <QChar>

#include "communicator_v2.h"



Result<QString> GarminGfdiMessage::readLengthPrefixedString(const QByteArray& data, int& consumedBytes)
{
    consumedBytes = 0;
    if (data.isEmpty()) {
        return Result<QString>::err(GarminError(GarminError::Code::InvalidMessage,
                                                     "No length byte for string"));
    }
    const int len = quint8(data[0]);
    if (data.size() < 1 + len) {
        return Result<QString>::err(GarminError(GarminError::Code::InvalidMessage,
            QString("String data too short: need %1 bytes, have %2")
                .arg(1 + len).arg(data.size())));
    }
    const QByteArray bytes = data.mid(1, len);
    const QString s = QString::fromUtf8(bytes);
    // Basic UTF-8 validity check: invalid sequences typically round-trip differently (replacement chars).
    if (s.toUtf8() != bytes) {
        return Result<QString>::err(GarminError(GarminError::Code::InvalidMessage,
                                                     "Invalid UTF-8 in string"));
    }
    consumedBytes = 1 + len;
    return Result<QString>::isOk(s);
}

QSet<quint16> GarminGfdiMessage::parseCapabilities(const QByteArray& bytes)
{
    QSet<quint16> caps;
    quint16 current = 0;
    for (auto ch : bytes) {
        const quint8 byte = quint8(ch);
        for (int i=0;i<8;i++) {
            if (byte & (1u << i)) caps.insert(current);
            current++;
        }
    }
    return caps;
}

void GarminGfdiMessage::onMessage(const QByteArray& data) {
    parse(data);
}


void GarminGfdiMessage::setCommunicator(CommunicatorV2* comm) {
    mCommunicator = comm;
}

// -------------------- Parser --------------------
// Parses an incoming GFDI messages and calls
// corresponging handlers
//-------------------------------------------------

void GarminGfdiMessage::parse(const QByteArray& data) {
    // Parse a GFDI message from raw bytes
    // MessageParser::parse in Rust

    qDebug() << Q_FUNC_INFO << "Garmin: parsing GFDI Message " << data;

    if (data.size() < 6) {
        return;// Result<GfdiMessage>::err(GarminError::invalidMessage(QStringLiteral("Message too short")));
    }

    int offset = 0;
    offset += 1; // packet size field, skip over - TODO: was 2, guess it's not packet size but handle number

    // Read message ID (2 bytes, little-endian)
    quint16 rawId = le16(data.constData() + offset);
    offset += 2;

    qDebug() << Q_FUNC_INFO << "Garmin: raw msgid is " << rawId;


    // Check for sequence number (bit 15 set)
    // If bit 15 is set, the message ID is encoded with a sequence number
    // Format: [bit 15: 1] [bits 14-8: sequence] [bits 7-0: message_id - 5000]
    // We need to decode it: actual_id = (raw_id & 0xFF) + 5000
    quint16 msgId = rawId;
    if ((msgId & 0x8000) != 0) {
        msgId = (msgId & 0xFF) + 5000;
    }

    qDebug() << Q_FUNC_INFO << "Garmin:  msgid is " << msgId;


    const auto mid = messageIdFromU16(msgId);
    if (!mid.has_value()) {
        qDebug() << Q_FUNC_INFO << "Garmin: Unknown GFDI message: " << msgId;
        return;
    }

    qDebug() << Q_FUNC_INFO << "Garmin:  GFDI message: " << messageIdToString(msgId).value();

    switch (*mid) {
    case MessageId::DeviceInformation:
        parseDeviceInformation(data.mid(offset));
        return;
    case MessageId::Configuration:
        return parseConfiguration(data.mid(offset));
    case MessageId::CurrentTimeRequest:
         parseCurrentTimeRequest(data.mid(offset));
         return;
    case MessageId::NotificationControl:
        parseNotificationControl(data.mid(offset));
        return;
    case MessageId::NotificationSubscription:
        parseNotificationSubscription(data.mid(offset));
        return;
    case MessageId::Synchronization:
        parseSynchronization(data.mid(offset));
        return;
    case MessageId::WeatherRequest:
        parseWeatherRequest(data.mid(offset));
        return;
    case MessageId::Response: {
        // Check if this is a filter status response - special-case
        if (offset + 2 < data.size()) {
            const quint16 orig = le16(data.constData() + offset);
            if (orig == 5007) {
                parseFilterStatus(data.mid(offset));
                return;
            }
        }
        // Other Responses will be handled as Unknown Messages in the default section
    }
    default:
        parseUnknownMessage(msgId, data.mid(offset));
    }
}

// -------------------- Parser: per-message parsers --------------------
void GarminGfdiMessage::parseCurrentTimeRequest(const QByteArray& data)
{
    auto timeResponse = GfdiMessageGenerator::currentTimeResponse(data);
    if (timeResponse.ok)
        if (mCommunicator) mCommunicator->sendMessage("Time Response", timeResponse.value);
}

void GarminGfdiMessage::parseDeviceInformation(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO << "Garmin: parsing device information" << data;
    if (data.size() < 10) {
        qDebug() << Q_FUNC_INFO <<  "Garmin: DeviceInformation too short";
        return;
        //return Result<GfdiMessage>::err(
        //    GarminError(GarminError::Code::InvalidMessage, "DeviceInformation too short"));
    }
    int off = 0;

    DeviceInformationMessage msg;
    msg.protocolVersion = u16le(data, off); off += 2;
    msg.productNumber   = u16le(data, off); off += 2;
    msg.unitNumber      = u32le(data, off); off += 4;
    msg.softwareVersion = u16le(data, off); off += 2;
    msg.maxPacketSize   = u16le(data, off); off += 2;

    int consumed = 0;

    // Read length-prefixed strings (1 byte length, then N bytes of UTF-8 string)
    auto s1 = readLengthPrefixedString(data.mid(off), consumed);
    msg.bluetoothFriendlyName = s1.value;
    qDebug() << Q_FUNC_INFO << "Garmin: Friendly name is " <<s1.value;

    off += consumed;

    auto s2 = readLengthPrefixedString(data.mid(off), consumed);
    msg.deviceName = s2.value;
    qDebug() << Q_FUNC_INFO << "Garmin: name is " <<s2.value;
    off += consumed;

    auto s3 = readLengthPrefixedString(data.mid(off), consumed);
    msg.deviceModel = s3.value;

    // Now update the device via the communicator
    if (mCommunicator) mCommunicator->onDeviceInformationReceived(msg);
}

void GarminGfdiMessage::parseConfiguration(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO << "Garmin: parsing configuration";
    if (data.isEmpty()) {
        return ;//Result<GfdiMessage>::err(GarminError(GarminError::Code::InvalidMessage, "Configuration message empty"));
    }
    const int numBytes = quint8(data[0]);
    if (data.size() < 1 + numBytes) {
        return; // Result<GfdiMessage>::err(GarminError(GarminError::Code::InvalidMessage, "Configuration data truncated"));
    }
    ConfigurationMessage msg;
    msg.capabilities = parseCapabilities(data.mid(1, numBytes));
    if (mCommunicator) mCommunicator->onConfigurationReceived(msg);
}

void GarminGfdiMessage::parseNotificationControl(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO << "Garmin: parsing notification control";
    if (data.size() < 7) {
        return; //Result<GfdiMessage>::err(GarminError(GarminError::Code::InvalidMessage, "NotificationControl message too short"));
    }

    NotificationControlMessage msg;
    msg.command = quint8(data[0]);
    msg.notificationId = i32le(data, 1);

    // PERFORM_NOTIFICATION_ACTION (128)
    if (msg.command == 128) {
        if (data.size() < 6) {
            return;
        }
        msg.actionId = quint8(data[5]);

        // null-terminated action string (if present)
        if (data.size() > 6) {
            const QByteArray stringData = data.mid(6);
            const int nullPos = stringData.indexOf(char(0));
            if (nullPos > 0) {
                const QByteArray raw = stringData.left(nullPos);
                const QString s = QString::fromUtf8(raw);
                if (!s.isEmpty() && s.toUtf8() == raw) {
                    bool printable = true;
                    for (QChar c : s) {
                        if (c.isPrint()) continue;
                        if (c == QChar('\n') || c == QChar('\r')) continue;
                        printable = false;
                        break;
                    }
                    if (printable) msg.actionString = s;
                }
            }
        }
        // attributes empty
        if (mCommunicator)mCommunicator->onNotificationControlReceived(msg);
        return;
    }

    // Requested attributes parsing
    int off = 5;
    while (off < data.size()) {
        if (off + 3 > data.size()) break;
        const quint8 attrId = quint8(data[off]);

        if (attrId == 0 || attrId == 4 || attrId == 5) {
            msg.attributes.append({attrId, 128});
            off += 1;
            continue;
        }

        if (attrId == 127) {
            msg.attributes.append({attrId, quint16(quint8(data[off + 1]))});
            off += 1; // attr_id
            off += 1; // short
            off += 2; // byte;
            continue;
        }

        const quint16 maxLen = u16le(data, off + 1);
        msg.attributes.append({attrId, maxLen});
        off += 3;
    }
    if (mCommunicator) mCommunicator->onNotificationControlReceived(msg);
}

void GarminGfdiMessage::parseNotificationSubscription(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO << "Garmin: parsing notification subscription";
    if (data.size() < 2) {
        return;// Result<GfdiMessage>::err(GarminError(GarminError::Code::InvalidMessage, "NotificationSubscription message too short"));
    }
    NotificationSubscriptionMessage msg;
    msg.enable = (quint8(data[0]) == 1);
    msg.unk = quint8(data[1]);
    if (mCommunicator) mCommunicator->onNotificationSubscriptionReceived(msg);
}

void GarminGfdiMessage::parseSynchronization(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO << "Garmin: parsing synchronization";
    if (data.size() < 2) {
        return; // Result<GfdiMessage>::err(GarminError(GarminError::Code::InvalidMessage, "Synchronization message too short"));
    }
    const quint8 syncType = quint8(data[0]);
    const int size = quint8(data[1]);

    if (data.size() < 2 + size) {
        return;// Result<GfdiMessage>::err(GarminError(GarminError::Code::InvalidMessage, "Synchronization message truncated"));
    }

    quint64 bitmask = 0;
    if (size == 8) {
        bitmask = u64le(data, 2);
    } else if (size == 4) {
        bitmask = quint64(u32le(data, 2));
    } else {
        return;// Result<GfdiMessage>::err(GarminError(GarminError::Code::InvalidMessage, QString("Unexpected synchronization bitmask size: %1").arg(size)));
    }

    SynchronizationMessage msg;
    msg.synchronizationType = syncType;
    msg.fileTypeBitmask = bitmask;
    if (mCommunicator) mCommunicator->onSynchronizationReceived(msg);
}

void GarminGfdiMessage::parseWeatherRequest(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO << "Garmin: parsing weather request";

    if (data.size() < 10) {
        return; // Result<GfdiMessage>::err(GarminError(GarminError::Code::InvalidMessage, "Weather request message too short"));
    }
    WeatherRequestMessage msg;
    msg.format = quint8(data[0]);
    msg.latitude = i32le(data, 1);
    msg.longitude = i32le(data, 5);
    msg.hoursOfForecast = quint8(data[9]);
    if (mCommunicator) mCommunicator->onWeatherRequestReceived(msg);
}

void GarminGfdiMessage::parseFilterStatus(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO << "Garmin: parsing filter status";

    if (data.size() < 3) {
        return; // Result<GfdiMessage>::err(GarminError(GarminError::Code::InvalidMessage, "Filter status message too short"));
    }

    const quint8 statusByte = quint8(data[2]);
    Status st;
    switch (statusByte) {
    case 0: st = Status::Ack; break;
    case 1: st = Status::Nack; break;
    case 2: st = Status::Unsupported; break;
    default:
        return;// Result<GfdiMessage>::err(GarminError(GarminError::Code::InvalidMessage,QString("Unknown status: %1").arg(statusByte)));
    }

    FilterStatusMessage msg;
    msg.status = st;
    msg.filterType = (data.size() > 3) ? quint8(data[3]) : 0;
    if (mCommunicator) mCommunicator->onFilterStatusReceived(msg);
}


void GarminGfdiMessage::parseUnknownMessage(const quint16 msgId, const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO << "Garmin: parsing unknown message ";;

    UnknownMessage msg;
    msg.messageId = msgId;
    msg.data=data;

    if (mCommunicator) mCommunicator->onUnknownMessageReceived(msg);
}



// MessageGenerator is used to generate messages for sending to the device

// -------------------- Generator helpers --------------------


void GfdiMessageGenerator::overwriteU16le(QByteArray& out, int off, quint16 v) {
    out[off] = char(v & 0xFF);
    out[off+1] = char((v >> 8) & 0xFF);
}

// CRC-16 checksum (Modbus/IBM), matches Rust compute_checksum()
quint16 GfdiMessageGenerator::computeChecksum(const QByteArray& data) {
    quint16 crc = 0;
    for (auto ch : data) {
        crc ^= quint8(ch);
        for (int i=0;i<8;i++) {
            if (crc & 1) crc = quint16((crc >> 1) ^ 0xA001);
            else crc = quint16(crc >> 1);
        }
    }
    return crc;
}



QByteArray GfdiMessageGenerator::truncateUtf8Bytes(const QString& s, int maxBytes) {
    QByteArray utf8 = s.toUtf8();
    if (utf8.size() <= maxBytes) return utf8;

    int boundary = maxBytes;
    // Back up while in a UTF-8 continuation byte (10xxxxxx)
    while (boundary > 0 && (quint8(utf8[boundary]) & 0xC0u) == 0x80u) boundary--;
    return utf8.left(boundary);
}



// -------------------- Generator: public API --------------------

Result<QByteArray> GfdiMessageGenerator::deviceInformationResponse(const DeviceInformationMessage& incoming)
{
    QByteArray r;
    r.append(char(0)); r.append(char(0));           // size placeholder - will be filled at the end
    writeU16le(r, 5000);                             // RESPONSE
    writeU16le(r, 5024);                             // original DEVICE_INFORMATION
    r.append(char(quint8(Status::Ack)));            // status

    writeU16le(r, 150);                              // protocol version 1.50
    writeU16le(r, 0xFFFF);                           // product number (-1 = 0xFFFF for phone)
    writeU32le(r, 0xFFFFFFFFu);                      // our unit number
    writeU16le(r, 7791);                             // software version  (7791 = version 77.91, matching Gadgetbridge)
    writeU16le(r, 0xFFFF);                           // our max packet size( -1 = 0xFFFF means no limit)
    // Bluetooth name (null-terminated)
    r.append("Jolla-Amazfish"); r.append(char(0));
    // Device manufacturer (null-terminated)
    r.append("Jolla");      r.append(char(0));
    // Device model (null-terminated)
    r.append("SailfishOS");             r.append(char(0));

     // Protocol flags (1 for v1.x, 0 for v2.x)
    const quint8 protocolFlags = (incoming.protocolVersion / 100 == 1) ? 1 : 0;
    r.append(char(protocolFlags));

    // Fill in packet size (total length + 2 for checksum)
    const quint16 packetSize = quint16(r.size() + 2);   // +2 for checksum
    overwriteU16le(r, 0, packetSize);

    // Add checksum
    const quint16 crc = computeChecksum(r);
    writeU16le(r, crc);
    return Result<QByteArray>::isOk(r);
}

Result<QByteArray> GfdiMessageGenerator::configurationResponse()
{
    QByteArray r;

    // Packet size placeholder
    r.append(char(0)); r.append(char(0));
    // Message ID: CONFIGURATION (5050)
    writeU16le(r, 5050); // CONFIGURATION

    // Generate our capabilities
    const QByteArray caps = generateCapabilities().value;
    // Number of capability bytes
    r.append(char(quint8(caps.size())));
    // Capability bytes
    r.append(caps);

    // Fill in packet size
    const quint16 packetSize = quint16(r.size() + 2);
    overwriteU16le(r, 0, packetSize);

    // Add checksum
    const quint16 crc = computeChecksum(r);
    writeU16le(r, crc);
    return Result<QByteArray>::isOk(r);
}

Result<QByteArray> GfdiMessageGenerator::ackResponse(quint16 messageId)
{
    QByteArray r;
    // Packet size
    writeU16le(r, 9);        // fixed packet size
    // Message ID: RESPONSE (5000)
    writeU16le(r, 5000);
    // Original message ID
    writeU16le(r, messageId);
    // Status: ACK
    r.append(char(quint8(Status::Ack)));

    // Add checksum
    const quint16 crc = computeChecksum(r);
    writeU16le(r, crc);
    return Result<QByteArray>::isOk(r);
}

Result<QByteArray> GfdiMessageGenerator::generateCapabilities() {
    QByteArray caps(15, char(0));

    // Set ALL capabilities (0-119) except the unknown/unsupported ones
    // This matches Gadgetbridge Java's OUR_CAPABILITIES behavior
    const int unsupported[] = {
        104,105,106,107,108,109,110,111,
        114,115,116,117,118,119
    };

    for (int cap = 0; cap < 120; ++cap) {
        bool skip = false;

        for (int u : unsupported) if (u == cap) { skip = true; break; }
        if (skip) continue;

        const int byteIdx = cap / 8;
        const int bitIdx = cap % 8;
        if (byteIdx < caps.size()) {
            caps[byteIdx] = char(quint8(caps[byteIdx]) | (1u << bitIdx));
        }
    }
    return Result<QByteArray>::isOk(caps);
}

Result<QByteArray> GfdiMessageGenerator::currentTimeResponse(const QByteArray& data)
{
    // Generate a CurrentTimeRequest response with current time
    // Unix seconds -> Garmin epoch (Dec 31 1989) offset 631065600
    quint32 unixNow = quint32(QDateTime::currentMSecsSinceEpoch()/1000);
    quint32 garminTime = unixNow - 631065600u;
    quint16 refid=u16le(data,0);

    //TODO: Fix this.
    int nextTransitionEndsGarminTs =0;
    int nextTransitionStartsGarminTs = 0;
    int timeZoneOffset = 0;// = TimeZone.getDefault().getOffset(now.toEpochMilli()) / 1000;

    QByteArray r;
    // Packet size placeholder
    r.append(char(0)); r.append(char(0));
    // Message ID: RESPONSE (5000)
    writeU16le(r, 5000);
    // Original message ID: CURRENT_TIME_REQUEST (5052)
    writeU16le(r, 5052);
    // Status: ACK
    r.append(char(quint8(Status::Ack)));
    // Now referenceid(32bit)
    writeU16le(r,refid);
    //Now Garmin Time
    writeU32le(r, garminTime);;
    //now timezoneoffset
    writeU32le(r,timeZoneOffset);
    //nexttransitionendsgarmints
    writeU32le(r,nextTransitionEndsGarminTs);
    //nexttransitionstartsgarmints
    writeU32le(r,nextTransitionStartsGarminTs);
    // Fill in packet size
    const quint16 packetSize = quint16(r.size() + 2);
    overwriteU16le(r, 0, packetSize);

    // Add checksum
    const quint16 crc = computeChecksum(r);
    writeU16le(r, crc);

    return Result<QByteArray>::isOk(r);
}

Result<QByteArray> GfdiMessageGenerator::filterMessage(quint8 filterType)
{
    QByteArray m;
    // Packet size placeholder (will be filled at end)
    m.append(char(0)); m.append(char(0));
    // Message ID: FILTER (5007)
    writeU16le(m, 5007);
    // Filter type
    m.append(char(filterType));


    // Fill in packet size TODO: Shouldn't this be +2?
    const quint16 packetSize = quint16(m.size() - 2);
    overwriteU16le(m, 0, packetSize);

    // Add checksum
    const quint16 crc = computeChecksum(m);
    writeU16le(m, crc);
    return Result<QByteArray>::isOk(m);
}

Result<QByteArray> GfdiMessageGenerator::synchronizationAck()
{
    QByteArray r;
    // Packet size placeholder
    r.append(char(0)); r.append(char(0));
    // Message ID: RESPONSE (5000)
    writeU16le(r, 5000);
    // Original message ID: SYNCHRONIZATION (5037)
    writeU16le(r, 5037);
    // Status: ACK
    r.append(char(quint8(Status::Ack)));

    // Fill in packet size TODO: Shouldn't this be +2?
    const quint16 packetSize = quint16(r.size() - 2);
    overwriteU16le(r, 0, packetSize);

    // Add checksum
    const quint16 crc = computeChecksum(r);
    writeU16le(r, crc);
    return Result<QByteArray>::isOk(r);
}

Result<QByteArray> GfdiMessageGenerator::weatherResponse(const WeatherRequestMessage&)
{
    // Generate a weather response ACK
    //
    // This sends a simple ACK to weather requests. The actual weather data
    // is sent separately via FIT messages (FitDefinition and FitData).
    //
    // # Arguments
    // * `WeatherRequestMessage` - The incoming WeatherRequestMessage

    QByteArray r;
    // Packet size placeholder
    r.append(char(0)); r.append(char(0));
    // Message ID: RESPONSE (5000)
    writeU16le(r, 5000);
    // Original message ID: WEATHER_REQUEST (5014)
    writeU16le(r, 5014);
    // Status: ACK
    r.append(char(quint8(Status::Ack)));

    // Fill in packet size TODO: Shouldn't this be +2?
    const quint16 packetSize = quint16(r.size() - 2);
    overwriteU16le(r, 0, packetSize);

    // Add checksum
    const quint16 crc = computeChecksum(r);
    writeU16le(r, crc);
    return Result<QByteArray>::isOk(r);
}

Result<QByteArray> GfdiMessageGenerator::fitDefinitionMessage(const QByteArray& fitDefinitionData)
{
    // Generate a FIT Definition message (5011)
    //
    // This wraps FIT definition data in a GFDI message envelope for BLE transmission.
    //
    // # Arguments
    // * `fit_definition_data` - The encoded FIT definition message bytes
    //
    QByteArray m;
    // Packet size placeholder
    m.append(char(0)); m.append(char(0));
    // Message ID: FIT_DEFINITION (5011)
    writeU16le(m, 5011);
    // FIT definition payload
    m.append(fitDefinitionData);

    // Fill in packet size
    const quint16 packetSize = quint16(m.size() - 2);
    overwriteU16le(m, 0, packetSize);

    // Add checksum
    const quint16 crc = computeChecksum(m);
    writeU16le(m, crc);
    return Result<QByteArray>::isOk(m);
}

Result<QByteArray> GfdiMessageGenerator::fitDataMessage(const QByteArray& fitData)
{
    // Generate a FIT Data message (5012)
    //
    // This wraps FIT data in a GFDI message envelope for BLE transmission.
    //
    // # Arguments
    // * `fitData` - The encoded FIT data message bytes
    QByteArray m;
    // Packet size placeholder
    m.append(char(0)); m.append(char(0));
    // Message ID: FIT_DATA (5012)
    writeU16le(m, 5012);
    // FIT data payload
    m.append(fitData);

    // Fill in packet size
    const quint16 packetSize = quint16(m.size() - 2);
    overwriteU16le(m, 0, packetSize);

    // Add checksum
    const quint16 crc = computeChecksum(m);
    writeU16le(m, crc);
    return Result<QByteArray>::isOk(m);
}

Result<QByteArray> GfdiMessageGenerator::notificationSubscriptionResponse(const NotificationSubscriptionMessage& incoming, bool enabled)
{
    // Generate a NotificationSubscription response
    //
    // This responds to the watch's notification subscription request (message ID 5036)
    // with an ACK and notification status information.
    //
    // # Arguments
    // * `incoming` - The incoming NotificationSubscriptionMessage
    // * `enabled` - Whether notifications are enabled (typically true)
    QByteArray r;
    // Packet size placeholder
    r.append(char(0)); r.append(char(0));
    // Message ID: RESPONSE (5000)
    writeU16le(r, 5000);
    // Original message ID: NOTIFICATION_SUBSCRIPTION (5036)
    writeU16le(r, 5036);
    // Status: ACK
    r.append(char(quint8(Status::Ack)));
    // Notification Status (0 = ENABLED, 1 = DISABLED)
    r.append(char(enabled ? 0 : 1));
    // Enable flag (matches incoming request)
    r.append(char(incoming.enable ? 1 : 0));
    // Unknown byte (copy from incoming)
    r.append(char(incoming.unk));

    // Fill in packet size
    const quint16 packetSize = quint16(r.size() + 2);
    overwriteU16le(r, 0, packetSize);

    // Add checksum
    const quint16 crc = computeChecksum(r);
    writeU16le(r, crc);
    return Result<QByteArray>::isOk(r);
}

Result<QByteArray> GfdiMessageGenerator::supportedFileTypesRequest()
{
    // Generate a SupportedFileTypesRequest message
    //
    // This proactively asks the watch what file types it supports.
    // Sent after Configuration exchange during initialization.
    //
    // Message format: Just the message ID, no payload
    QByteArray m;
    // Packet size placeholder
    m.append(char(0)); m.append(char(0));
    // Message ID: SUPPORTED_FILE_TYPES_REQUEST (5031)
    writeU16le(m, 5031);

    // Fill in packet size
    const quint16 packetSize = quint16(m.size() + 2);
    overwriteU16le(m, 0, packetSize);

     // Add checksum
    const quint16 crc = computeChecksum(m);
    writeU16le(m, crc);
    return Result<QByteArray>::isOk(m);
}

Result<QByteArray> GfdiMessageGenerator::setDeviceSettings(bool autoUpload, bool weatherConditions, bool weatherAlerts)
{
    // Generate a SetDeviceSettings message
    //
    // Sends device settings to the watch (auto upload, weather, etc.)
    // Sent after Configuration exchange during initialization.
    //
    // # Arguments
    // * `autoUpload` - Enable auto upload of activities
    // * `weatherConditions` - Enable weather conditions
    // * `weatherAlerts` - Enable weather alerts
    QByteArray m;

    // Packet size placeholder
    m.append(char(0)); m.append(char(0));
    // Message ID: DEVICE_SETTINGS (5026)
    writeU16le(m, 5026);
    // Number of settings (always 3 for now)
    m.append(char(3));

    // Each setting consists of an ordinal/index, data length and status byte
    // Setting 1: AUTO_UPLOAD_ENABLED (index 6)
    m.append(char(6)); m.append(char(1)); m.append(char(autoUpload ? 1 : 0)); // GarminDeviceSetting.AUTO_UPLOAD_ENABLED ordinal
    // Setting 2: WEATHER_CONDITIONS_ENABLED (index 7)
    m.append(char(7)); m.append(char(1)); m.append(char(weatherConditions ? 1 : 0));
    // Setting 3: WEATHER_ALERTS_ENABLED (index 8)
    m.append(char(8)); m.append(char(1)); m.append(char(weatherAlerts ? 1 : 0));

    // Fill in packet size
    const quint16 packetSize = quint16(m.size() + 2);
    overwriteU16le(m, 0, packetSize);

    // Add checksum
    const quint16 crc = computeChecksum(m);
    writeU16le(m, crc);
    return Result<QByteArray>::isOk(m);
}

Result<QByteArray> GfdiMessageGenerator::systemEvent(quint8 eventType, quint8 value)
{
    // Generate a SystemEvent message
    //
    // Sends system events to the watch (PAIR_START, SYNC_READY, etc.)
    //
    // # Arguments
    // * `eventType` - The event type ordinal (see GarminSystemEventType in Java)
    // * `value` - The event value (typically 0)
    //
    // Event types:
    // - 0: SYNC_COMPLETE
    // - 1: SYNC_FAIL
    // - 2: FACTORY_RESET
    // - 3: PAIR_START
    // - 4: PAIR_COMPLETE
    // - 5: PAIR_FAIL
    // - 6: HOST_DID_ENTER_FOREGROUND
    // - 7: HOST_DID_ENTER_BACKGROUND
    // - 8: SYNC_READY
    // - 9: NEW_DOWNLOAD_AVAILABLE
    // - 10: DEVICE_SOFTWARE_UPDATE
    // - 11: DEVICE_DISCONNECT
    // - 12: TUTORIAL_COMPLETE
    // - 13: SETUP_WIZARD_START
    // - 14: SETUP_WIZARD_COMPLETE
    // - 15: SETUP_WIZARD_SKIPPED
    // - 16: TIME_UPDATED
    QByteArray m;
    // Packet size placeholder
    m.append(char(0)); m.append(char(0));
    // Message ID: SYSTEM_EVENT (5030)
    writeU16le(m, 5030);
    // Event type
    m.append(char(eventType));
    // Value
    m.append(char(value));

    // Fill in packet size
    const quint16 packetSize = quint16(m.size() + 2);
    overwriteU16le(m, 0, packetSize);

    // Add checksum
    const quint16 crc = computeChecksum(m);
    writeU16le(m, crc);
    return Result<QByteArray>::isOk(m);
}

Result<QByteArray> GfdiMessageGenerator::protobufBatteryStatusRequest(quint16 requestId)
{

    // Generate a ProtobufRequest for battery status updates
    //
    // This sends a protobuf request asking the watch to provide battery status updates.
    // Sent during initialization (completeInitialization in Java).
    //
    // The protobuf structure is:
    // ```proto
    // Smart {
    //   device_status_service = 8 {
    //     remote_device_battery_status_request = 2 {
    //       // empty message
    //     }
    //   }
    // }
    // ```
    //
    // # Arguments
    // * `requestId` - The protobuf request ID (incrementing counter)

    // Build the protobuf payload manually
    // Smart.device_status_service (field 8, type length-delimited)
    // DeviceStatusService.remote_device_battery_status_request (field 2, type length-delimited)

    // Inner message: RemoteDeviceBatteryStatusRequest (empty)
    QByteArray inner; // empty

    // DeviceStatusService with field 2 (remote_device_battery_status_request)
    QByteArray deviceStatusService;
    deviceStatusService.append(char((2 << 3) | 2)); // Field 2, wire type 2 (length-delimited)
    deviceStatusService.append(char(inner.size()));// Length of inner message (0)
    deviceStatusService.append(inner);

    // Smart with field 8 (device_status_service)
    QByteArray smartProto;
    smartProto.append(char((8 << 3) | 2));// Field 8, wire type 2 (length-delimited)
    smartProto.append(char(deviceStatusService.size()));// Length
    smartProto.append(deviceStatusService);

    // Now build the ProtobufRequest message
    QByteArray m;
    // Packet size placeholder
    m.append(char(0)); m.append(char(0));
    // Message ID: PROTOBUF_REQUEST (5043)
    writeU16le(m, 5043);
    // Request ID
    writeU16le(m, requestId);
    // Data offset (0 for non-chunked)
    writeU32le(m, 0);
    // Total protobuf length
    writeU32le(m, quint32(smartProto.size()));
    // Protobuf data length (same as total for non-chunked)
    writeU32le(m, quint32(smartProto.size()));
    // Protobuf payload
    m.append(smartProto);

    // Fill in packet size
    const quint16 packetSize = quint16(m.size() + 2);
    overwriteU16le(m, 0, packetSize);

    // Add checksum
    const quint16 crc = computeChecksum(m);
    writeU16le(m, crc);
    return Result<QByteArray>::isOk(m);
}

// Convenience wrapper
Result<QByteArray> GfdiMessageGenerator::notificationData(

    quint32 notificationId,
    const QVector<QPair<quint8, quint16>>& requestedAttributes,
    const QString& title, const QString& body, const QString& sender,
    const QString& timestamp, const QString& appId)
{
    // Generate a NotificationData message with notification content
    //
    // This sends the actual notification content (sender, body, title, etc.)
    // in response to the watch's NotificationControl request.
    //
    // # Arguments
    // * `notificationId` - The notification ID from NotificationControl
    // * `requestedAttributes` - List of (attribute_id, max_length) from NotificationControl
    // * `title` - Notification title
    // * `body` - Notification body/message
    // * `sender` - Notification sender/source
    // * `timestamp` - Timestamp in format "yyyyMMddTHHmmss"
    // * `appId` - Application identifier
    return notificationDataWithActions(notificationId, requestedAttributes, title, body, sender, timestamp, appId, true);
}

Result<QByteArray> GfdiMessageGenerator::notificationDataWithActions(
    quint32 notificationId,
    const QVector<QPair<quint8, quint16>>& requestedAttributes,
    const QString& title, const QString& body, const QString& sender,
    const QString& timestamp, const QString& appId,
    bool hasDismissAction)
{

    // Generate NotificationData message with action support
    // * `notificationId` - Unique notification identifier
    // * `requestedAttributes` - List of (attribute_id, max_length) tuples
    // * `title` - Notification title
    // * `body` - Notification body text
    // * `sender` - Notification sender
    // * `timestamp` - Timestamp in format "yyyyMMddTHHmmss"
    // * `appId` - Application identifier
    // * `hasDismissAction` - Whether to include a dismiss action
    QByteArray payload;
    // Command byte: GET_NOTIFICATION_ATTRIBUTES = 0
    payload.append(char(0));
    // Notification ID
    writeU32le(payload, notificationId);

    // Process all requested attributes except MESSAGE_SIZE first (matching Java implementation)
    std::optional<QPair<quint8, quint16>> msgSizeEntry;

    for (const auto& p : requestedAttributes) {
        // Save MESSAGE_SIZE for last
        const quint8 attrId = p.first;
        const quint16 maxLen = p.second;

        if (attrId == 4) { msgSizeEntry = p; continue; }

        if (attrId == 127) {
            // ACTIONS
            if (hasDismissAction) {
                // Encode actions list with DISMISS_NOTIFICATION action (code 98)
                // Format: [count(1), action_code(1), icon_position(1), title_len(1), title...]
                // DISMISS_NOTIFICATION = 98, LEFT position = 0x04 (bit 2), title = "Dismiss"
                // Icon positions are bit vectors: BOTTOM=0x01, RIGHT=0x02, LEFT=0x04
                const QString dismissTitle = "Dismiss";
                QByteArray actions;
                actions.append(char(1));
                actions.append(char(98));
                actions.append(char(0x04));
                actions.append(char(dismissTitle.toUtf8().size()));
                actions.append(dismissTitle.toUtf8());

                // Write attribute with binary data
                payload.append(char(attrId));
                writeU16le(payload, quint16(actions.size()));
                payload.append(actions);
            } else {
                // No actions - send empty action list
                payload.append(char(attrId));
                writeU16le(payload, 4);
                payload.append(char(0)); payload.append(char(0)); payload.append(char(0)); payload.append(char(0));
            }
            continue;// Skip unknown attributes
        }


        QString value;
        switch (attrId) {
        case 0: value = appId; break;
        case 1: value = title; break;
        case 2: value = ""; break;
        case 3: value = body; break;
        case 5: value = timestamp; break;
        case 9: value = sender; break;
        default: continue;
        }

        // Truncate if max_len is specified and not 0xFFFF (no limit)
        QByteArray bytes = value.toUtf8();
        if (maxLen > 0 && maxLen < 0xFFFF) bytes = truncateUtf8Bytes(value, int(maxLen));

        // Write attribute: [id][length][value]
        payload.append(char(attrId));
        writeU16le(payload, quint16(bytes.size()));
        payload.append(bytes);
    }

    // Now process MESSAGE_SIZE last (Java does this to match protocol expectations)
    if (msgSizeEntry.has_value()) {
        const quint8 attrId = msgSizeEntry->first;
        const quint16 maxLen = msgSizeEntry->second;

        // MESSAGE_SIZE: Length of the body as a string
        // Java reference: Integer.toString(body.length())
        QString value = QString::number(body.toUtf8().size());
        QByteArray bytes = value.toUtf8();
        if (maxLen > 0 && maxLen < 0xFFFF) bytes = truncateUtf8Bytes(value, int(maxLen));

         // Write attribute: [id][length][value]
        payload.append(char(attrId));
        writeU16le(payload, quint16(bytes.size()));
        payload.append(bytes);
    }

    // Calculate CRC of the payload data
    const quint16 dataCrc = computeCrc16(payload);

    // Build the NotificationData message
    QByteArray m;
    // Packet size placeholder
    m.append(char(0)); m.append(char(0));
    // Message ID: NOTIFICATION_DATA (5035)
    writeU16le(m, 5035);
    // Message size (total payload size)
    writeU16le(m, quint16(payload.size()));

    // Data CRC
    writeU16le(m, dataCrc);
    // Data offset (0 for non-chunked messages)
    writeU16le(m, 0);
    // Payload
    m.append(payload);

    // Fill in packet size
    const quint16 packetSize = quint16(m.size() + 2);
    overwriteU16le(m, 0, packetSize);

    // Add envelope checksum
    const quint16 crc = computeChecksum(m);
    writeU16le(m, crc);
    return Result<QByteArray>::isOk(m);
}
