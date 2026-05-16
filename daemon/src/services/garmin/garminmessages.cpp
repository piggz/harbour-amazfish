#include "garminmessages.h"
#include <QChar>

// -------------------- Parser helpers --------------------

quint16 GfdiMessageParser::u16le(const QByteArray& b, int off) {
    return quint16(quint8(b[off])) | (quint16(quint8(b[off+1])) << 8);
}
quint32 GfdiMessageParser::u32le(const QByteArray& b, int off) {
    return quint32(quint8(b[off])) |
           (quint32(quint8(b[off+1])) << 8) |
           (quint32(quint8(b[off+2])) << 16) |
           (quint32(quint8(b[off+3])) << 24);
}
quint64 GfdiMessageParser::u64le(const QByteArray& b, int off) {
    quint64 v = 0;
    for (int i=0;i<8;i++) v |= (quint64(quint8(b[off+i])) << (8*i));
    return v;
}
qint32 GfdiMessageParser::i32le(const QByteArray& b, int off) {
    return qint32(u32le(b, off));
}

Result<QString> GfdiMessageParser::readLengthPrefixedString(const QByteArray& data, int& consumedBytes)
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

QSet<quint16> GfdiMessageParser::parseCapabilities(const QByteArray& bytes)
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

// -------------------- Parser: per-message parsers --------------------

Result<GfdiMessage> GfdiMessageParser::parseDeviceInformation(const QByteArray& data)
{
    if (data.size() < 10) {
        return Result<GfdiMessage>::err(
            GarminError(GarminError::Code::InvalidMessage, "DeviceInformation too short"));
    }
    int off = 0;
    DeviceInformationMessage msg;
    msg.protocolVersion = u16le(data, off); off += 2;
    msg.productNumber   = u16le(data, off); off += 2;
    msg.unitNumber      = u32le(data, off); off += 4;
    msg.softwareVersion = u16le(data, off); off += 2;
    msg.maxPacketSize   = u16le(data, off); off += 2;

    int consumed = 0;

    auto s1 = readLengthPrefixedString(data.mid(off), consumed);
    if (!s1.ok) return Result<GfdiMessage>::err(s1.error);
    msg.bluetoothFriendlyName = s1.value;
    off += consumed;

    auto s2 = readLengthPrefixedString(data.mid(off), consumed);
    if (!s2.ok) return Result<GfdiMessage>::err(s2.error);
    msg.deviceName = s2.value;
    off += consumed;

    auto s3 = readLengthPrefixedString(data.mid(off), consumed);
    if (!s3.ok) return Result<GfdiMessage>::err(s3.error);
    msg.deviceModel = s3.value;


    return Result<GfdiMessage>::isOk(msg);
}

Result<GfdiMessage> GfdiMessageParser::parseConfiguration(const QByteArray& data)
{
    if (data.isEmpty()) {
        return Result<GfdiMessage>::err(
            GarminError(GarminError::Code::InvalidMessage, "Configuration message empty"));
    }
    const int numBytes = quint8(data[0]);
    if (data.size() < 1 + numBytes) {
        return Result<GfdiMessage>::err(
            GarminError(GarminError::Code::InvalidMessage, "Configuration data truncated"));
    }
    ConfigurationMessage msg;
    msg.capabilities = parseCapabilities(data.mid(1, numBytes));
    return Result<GfdiMessage>::isOk(msg);
}

Result<GfdiMessage> GfdiMessageParser::parseNotificationControl(const QByteArray& data)
{
    if (data.size() < 7) {
        return Result<GfdiMessage>::err(
            GarminError(GarminError::Code::InvalidMessage, "NotificationControl message too short"));
    }

    NotificationControlMessage msg;
    msg.command = quint8(data[0]);
    msg.notificationId = i32le(data, 1);

    // PERFORM_NOTIFICATION_ACTION (128)
    if (msg.command == 128) {
        if (data.size() < 6) {
            return Result<GfdiMessage>::err(
                GarminError(GarminError::Code::InvalidMessage, "PERFORM_NOTIFICATION_ACTION message too short"));
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
        return Result<GfdiMessage>::isOk(msg);
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

    return Result<GfdiMessage>::isOk(msg);
}

Result<GfdiMessage> GfdiMessageParser::parseNotificationSubscription(const QByteArray& data)
{
    if (data.size() < 2) {
        return Result<GfdiMessage>::err(
            GarminError(GarminError::Code::InvalidMessage, "NotificationSubscription message too short"));
    }
    NotificationSubscriptionMessage msg;
    msg.enable = (quint8(data[0]) == 1);
    msg.unk = quint8(data[1]);
    return Result<GfdiMessage>::isOk(msg);
}

Result<GfdiMessage> GfdiMessageParser::parseSynchronization(const QByteArray& data)
{
    if (data.size() < 2) {
        return Result<GfdiMessage>::err(
            GarminError(GarminError::Code::InvalidMessage, "Synchronization message too short"));
    }
    const quint8 syncType = quint8(data[0]);
    const int size = quint8(data[1]);

    if (data.size() < 2 + size) {
        return Result<GfdiMessage>::err(
            GarminError(GarminError::Code::InvalidMessage, "Synchronization message truncated"));
    }

    quint64 bitmask = 0;
    if (size == 8) {
        bitmask = u64le(data, 2);
    } else if (size == 4) {
        bitmask = quint64(u32le(data, 2));
    } else {
        return Result<GfdiMessage>::err(
            GarminError(GarminError::Code::InvalidMessage,
                        QString("Unexpected synchronization bitmask size: %1").arg(size)));
    }

    SynchronizationMessage msg;
    msg.synchronizationType = syncType;
    msg.fileTypeBitmask = bitmask;
    return Result<GfdiMessage>::isOk(msg);
}

Result<GfdiMessage> GfdiMessageParser::parseWeatherRequest(const QByteArray& data)
{
    if (data.size() < 10) {
        return Result<GfdiMessage>::err(
            GarminError(GarminError::Code::InvalidMessage, "Weather request message too short"));
    }
    WeatherRequestMessage msg;
    msg.format = quint8(data[0]);
    msg.latitude = i32le(data, 1);
    msg.longitude = i32le(data, 5);
    msg.hoursOfForecast = quint8(data[9]);
    return Result<GfdiMessage>::isOk(msg);
}

Result<GfdiMessage> GfdiMessageParser::parseFilterStatus(const QByteArray& data)
{
    if (data.size() < 3) {
        return Result<GfdiMessage>::err(
            GarminError(GarminError::Code::InvalidMessage, "Filter status message too short"));
    }

    const quint8 statusByte = quint8(data[2]);
    Status st;
    switch (statusByte) {
    case 0: st = Status::Ack; break;
    case 1: st = Status::Nack; break;
    case 2: st = Status::Unsupported; break;
    default:
        return Result<GfdiMessage>::err(
            GarminError(GarminError::Code::InvalidMessage,
                        QString("Unknown status: %1").arg(statusByte)));
    }

    FilterStatusMessage msg;
    msg.status = st;
    msg.filterType = (data.size() > 3) ? quint8(data[3]) : 0;
    return Result<GfdiMessage>::isOk(msg);
}

// -------------------- Parser main --------------------


Result<GfdiMessage> GfdiMessageParser::parse(const QByteArray& data) {
    if (data.size() < 6) {
        return Result<GfdiMessage>::err(GarminError::invalidMessage(QStringLiteral("Message too short")));
    }

    int offset = 0;
    offset += 2; // packet size

    quint16 rawId = le16(data.constData() + offset);
    offset += 2;

    // decode possible sequence wrapper (bit 15)
    quint16 msgId = rawId;
    if ((msgId & 0x8000) != 0) {
        msgId = (msgId & 0xFF) + 5000;
    }

    const auto mid = messageIdFromU16(msgId);
    if (!mid.has_value()) {
        return Result<GfdiMessage>::isOk(UnknownMessage{msgId, data.mid(offset)});
    }

    switch (*mid) {
    case MessageId::DeviceInformation:
        return parseDeviceInformation(data.mid(offset));
    case MessageId::Configuration:
        return parseConfiguration(data.mid(offset));
    case MessageId::CurrentTimeRequest:
        return Result<GfdiMessage>::isOk(GfdiMessage{std::monostate{}});
    case MessageId::NotificationControl:
        return parseNotificationControl(data.mid(offset));
    case MessageId::NotificationSubscription:
        return parseNotificationSubscription(data.mid(offset));
    case MessageId::Synchronization:
        return parseSynchronization(data.mid(offset));
    case MessageId::WeatherRequest:
        return parseWeatherRequest(data.mid(offset));
    case MessageId::Response: {
        // Filter status special-case
        if (offset + 2 < data.size()) {
            const quint16 orig = le16(data.constData() + offset);
            if (orig == 5007) {
                return parseFilterStatus(data.mid(offset));
            }
        }
        return Result<GfdiMessage>::isOk(UnknownMessage{msgId, data.mid(offset)});
    }
    default:
        return Result<GfdiMessage>::isOk(UnknownMessage{msgId, data.mid(offset)});
    }
}

// -------------------- Generator helpers --------------------

void GfdiMessageGenerator::pushU16le(QByteArray& out, quint16 v) {
    out.append(char(v & 0xFF));
    out.append(char((v >> 8) & 0xFF));
}
void GfdiMessageGenerator::pushU32le(QByteArray& out, quint32 v) {
    out.append(char(v & 0xFF));
    out.append(char((v >> 8) & 0xFF));
    out.append(char((v >> 16) & 0xFF));
    out.append(char((v >> 24) & 0xFF));
}
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

QByteArray GfdiMessageGenerator::generateCapabilities() {
    QByteArray caps(15, char(0));

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
    return caps;
}

QByteArray GfdiMessageGenerator::truncateUtf8Bytes(const QString& s, int maxBytes) {
    QByteArray utf8 = s.toUtf8();
    if (utf8.size() <= maxBytes) return utf8;

    int boundary = maxBytes;
    // Back up while in a UTF-8 continuation byte (10xxxxxx)
    while (boundary > 0 && (quint8(utf8[boundary]) & 0xC0u) == 0x80u) boundary--;
    return utf8.left(boundary);
}

// Garmin CRC for NotificationData payload (matches Rust compute_crc16())
quint16 GfdiMessageGenerator::computeCrc16(const QByteArray& data) {
    static constexpr quint16 CONSTANTS[16] = {
        0x0000, 0xCC01, 0xD801, 0x1400,
        0xF001, 0x3C00, 0x2800, 0xE401,
        0xA001, 0x6C00, 0x7800, 0xB401,
        0x5000, 0x9C01, 0x8801, 0x4400
    };

    quint16 crc = 0;
    for (auto ch : data) {
        const quint16 b = quint8(ch);
        crc = quint16((((crc >> 4) & 0x0FFF) ^ CONSTANTS[crc & 0x0F]) ^ CONSTANTS[b & 0x0F]);
        crc = quint16((((crc >> 4) & 0x0FFF) ^ CONSTANTS[crc & 0x0F]) ^ CONSTANTS[(b >> 4) & 0x0F]);
    }
    return crc;
}

// -------------------- Generator: public API --------------------

Result<QByteArray> GfdiMessageGenerator::deviceInformationResponse(const DeviceInformationMessage& incoming)
{
    QByteArray r;
    r.append(char(0)); r.append(char(0));           // size placeholder
    pushU16le(r, 5000);                             // RESPONSE
    pushU16le(r, 5024);                             // original DEVICE_INFORMATION
    r.append(char(quint8(Status::Ack)));            // status

    pushU16le(r, 150);                              // protocol version 1.50
    pushU16le(r, 0xFFFF);                           // product number
    pushU32le(r, 0xFFFFFFFFu);                      // unit number
    pushU16le(r, 7791);                             // software version
    pushU16le(r, 0xFFFF);                           // max packet size

    r.append("Gadgetbridge-Rust"); r.append(char(0));
    r.append("Gadgetbridge");      r.append(char(0));
    r.append("Linux");             r.append(char(0));

    const quint8 protocolFlags = (incoming.protocolVersion / 100 == 1) ? 1 : 0;
    r.append(char(protocolFlags));

    const quint16 packetSize = quint16(r.size() + 2);   // +2 for checksum
    overwriteU16le(r, 0, packetSize);

    const quint16 crc = computeChecksum(r);
    pushU16le(r, crc);
    return Result<QByteArray>::isOk(r);
}

Result<QByteArray> GfdiMessageGenerator::configurationResponse()
{
    QByteArray r;
    r.append(char(0)); r.append(char(0));
    pushU16le(r, 5050); // CONFIGURATION

    const QByteArray caps = generateCapabilities();
    r.append(char(quint8(caps.size())));
    r.append(caps);

    const quint16 packetSize = quint16(r.size() + 2);
    overwriteU16le(r, 0, packetSize);

    const quint16 crc = computeChecksum(r);
    pushU16le(r, crc);
    return Result<QByteArray>::isOk(r);
}

Result<QByteArray> GfdiMessageGenerator::ackResponse(quint16 messageId)
{
    QByteArray r;
    pushU16le(r, 9);        // fixed packet size
    pushU16le(r, 5000);     // RESPONSE
    pushU16le(r, messageId);// original
    r.append(char(quint8(Status::Ack)));

    const quint16 crc = computeChecksum(r);
    pushU16le(r, crc);
    return Result<QByteArray>::isOk(r);
}

Result<QByteArray> GfdiMessageGenerator::currentTimeResponse()
{
    QByteArray r;
    r.append(char(0)); r.append(char(0));
    pushU16le(r, 5000); // RESPONSE
    pushU16le(r, 5052); // original CURRENT_TIME_REQUEST
    r.append(char(quint8(Status::Ack)));

    // Unix seconds -> Garmin epoch (Dec 31 1989) offset 631065600
    quint32 unixNow = quint32(QDateTime::currentMSecsSinceEpoch()/1000);
    quint32 garminTime = unixNow - 631065600u; // wraps like Rust wrapping_sub
    pushU32le(r, garminTime);

    const quint16 packetSize = quint16(r.size() + 2);
    overwriteU16le(r, 0, packetSize);

    const quint16 crc = computeChecksum(r);
    pushU16le(r, crc);
    return Result<QByteArray>::isOk(r);
}

Result<QByteArray> GfdiMessageGenerator::filterMessage(quint8 filterType)
{
    QByteArray m;
    m.append(char(0)); m.append(char(0));
    pushU16le(m, 5007); // FILTER
    m.append(char(filterType));

    // Note: Rust uses (len() - 2) here; we mirror it exactly.
    const quint16 packetSize = quint16(m.size() - 2);
    overwriteU16le(m, 0, packetSize);

    const quint16 crc = computeChecksum(m);
    pushU16le(m, crc);
    return Result<QByteArray>::isOk(m);
}

Result<QByteArray> GfdiMessageGenerator::synchronizationAck()
{
    QByteArray r;
    r.append(char(0)); r.append(char(0));
    pushU16le(r, 5000); // RESPONSE
    pushU16le(r, 5037); // original SYNCHRONIZATION
    r.append(char(quint8(Status::Ack)));

    // Mirror Rust: (len() - 2)
    const quint16 packetSize = quint16(r.size() - 2);
    overwriteU16le(r, 0, packetSize);

    const quint16 crc = computeChecksum(r);
    pushU16le(r, crc);
    return Result<QByteArray>::isOk(r);
}

Result<QByteArray> GfdiMessageGenerator::weatherResponse(const WeatherRequestMessage&)
{
    QByteArray r;
    r.append(char(0)); r.append(char(0));
    pushU16le(r, 5000);
    pushU16le(r, 5014);
    r.append(char(quint8(Status::Ack)));

    const quint16 packetSize = quint16(r.size() - 2);
    overwriteU16le(r, 0, packetSize);

    const quint16 crc = computeChecksum(r);
    pushU16le(r, crc);
    return Result<QByteArray>::isOk(r);
}

Result<QByteArray> GfdiMessageGenerator::fitDefinitionMessage(const QByteArray& fitDefinitionData)
{
    QByteArray m;
    m.append(char(0)); m.append(char(0));
    pushU16le(m, 5011);
    m.append(fitDefinitionData);

    const quint16 packetSize = quint16(m.size() - 2);
    overwriteU16le(m, 0, packetSize);

    const quint16 crc = computeChecksum(m);
    pushU16le(m, crc);
    return Result<QByteArray>::isOk(m);
}

Result<QByteArray> GfdiMessageGenerator::fitDataMessage(const QByteArray& fitData)
{
    QByteArray m;
    m.append(char(0)); m.append(char(0));
    pushU16le(m, 5012);
    m.append(fitData);

    const quint16 packetSize = quint16(m.size() - 2);
    overwriteU16le(m, 0, packetSize);

    const quint16 crc = computeChecksum(m);
    pushU16le(m, crc);
    return Result<QByteArray>::isOk(m);
}

Result<QByteArray> GfdiMessageGenerator::notificationSubscriptionResponse(const NotificationSubscriptionMessage& incoming, bool enabled)
{
    QByteArray r;
    r.append(char(0)); r.append(char(0));
    pushU16le(r, 5000);
    pushU16le(r, 5036);
    r.append(char(quint8(Status::Ack)));
    r.append(char(enabled ? 0 : 1));
    r.append(char(incoming.enable ? 1 : 0));
    r.append(char(incoming.unk));

    const quint16 packetSize = quint16(r.size() + 2);
    overwriteU16le(r, 0, packetSize);

    const quint16 crc = computeChecksum(r);
    pushU16le(r, crc);
    return Result<QByteArray>::isOk(r);
}

Result<QByteArray> GfdiMessageGenerator::supportedFileTypesRequest()
{
    QByteArray m;
    m.append(char(0)); m.append(char(0));
    pushU16le(m, 5031);

    const quint16 packetSize = quint16(m.size() + 2);
    overwriteU16le(m, 0, packetSize);

    const quint16 crc = computeChecksum(m);
    pushU16le(m, crc);
    return Result<QByteArray>::isOk(m);
}

Result<QByteArray> GfdiMessageGenerator::setDeviceSettings(bool autoUpload, bool weatherConditions, bool weatherAlerts)
{
    QByteArray m;

    m.append(char(0)); m.append(char(0));
    pushU16le(m, 5026);
    m.append(char(3));

    m.append(char(6)); m.append(char(1)); m.append(char(autoUpload ? 1 : 0));
    m.append(char(7)); m.append(char(1)); m.append(char(weatherConditions ? 1 : 0));
    m.append(char(8)); m.append(char(1)); m.append(char(weatherAlerts ? 1 : 0));

    const quint16 packetSize = quint16(m.size() + 2);
    overwriteU16le(m, 0, packetSize);

    const quint16 crc = computeChecksum(m);
    pushU16le(m, crc);
    return Result<QByteArray>::isOk(m);
}

Result<QByteArray> GfdiMessageGenerator::systemEvent(quint8 eventType, quint8 value)
{
    QByteArray m;
    m.append(char(0)); m.append(char(0));
    pushU16le(m, 5030);
    m.append(char(eventType));
    m.append(char(value));

    const quint16 packetSize = quint16(m.size() + 2);
    overwriteU16le(m, 0, packetSize);

    const quint16 crc = computeChecksum(m);
    pushU16le(m, crc);
    return Result<QByteArray>::isOk(m);
}

Result<QByteArray> GfdiMessageGenerator::protobufBatteryStatusRequest(quint16 requestId)
{
    // Build protobuf payload manually
    QByteArray inner; // empty

    QByteArray deviceStatusService;
    deviceStatusService.append(char((2 << 3) | 2));
    deviceStatusService.append(char(inner.size()));
    deviceStatusService.append(inner);

    QByteArray smartProto;
    smartProto.append(char((8 << 3) | 2));
    smartProto.append(char(deviceStatusService.size()));
    smartProto.append(deviceStatusService);

    QByteArray m;
    m.append(char(0)); m.append(char(0));
    pushU16le(m, 5043);
    pushU16le(m, requestId);
    pushU32le(m, 0); // data offset
    pushU32le(m, quint32(smartProto.size()));
    pushU32le(m, quint32(smartProto.size()));
    m.append(smartProto);

    const quint16 packetSize = quint16(m.size() + 2);
    overwriteU16le(m, 0, packetSize);

    const quint16 crc = computeChecksum(m);
    pushU16le(m, crc);
    return Result<QByteArray>::isOk(m);
}

// Convenience wrapper
Result<QByteArray> GfdiMessageGenerator::notificationData(
    quint32 notificationId,
    const QVector<QPair<quint8, quint16>>& requestedAttributes,
    const QString& title, const QString& body, const QString& sender,
    const QString& timestamp, const QString& appId)
{
    return notificationDataWithActions(notificationId, requestedAttributes, title, body, sender, timestamp, appId, true);
}

Result<QByteArray> GfdiMessageGenerator::notificationDataWithActions(
    quint32 notificationId,
    const QVector<QPair<quint8, quint16>>& requestedAttributes,
    const QString& title, const QString& body, const QString& sender,
    const QString& timestamp, const QString& appId,
    bool hasDismissAction)
{
    QByteArray payload;
    payload.append(char(0)); // GET_NOTIFICATION_ATTRIBUTES
    pushU32le(payload, notificationId);

    std::optional<QPair<quint8, quint16>> msgSizeEntry;

    for (const auto& p : requestedAttributes) {
        const quint8 attrId = p.first;
        const quint16 maxLen = p.second;

        if (attrId == 4) { msgSizeEntry = p; continue; }

        if (attrId == 127) {
            if (hasDismissAction) {
                const QString dismissTitle = "Dismiss";
                QByteArray actions;
                actions.append(char(1));
                actions.append(char(98));
                actions.append(char(0x04));
                actions.append(char(dismissTitle.toUtf8().size()));
                actions.append(dismissTitle.toUtf8());

                payload.append(char(attrId));
                pushU16le(payload, quint16(actions.size()));
                payload.append(actions);
            } else {
                payload.append(char(attrId));
                pushU16le(payload, 4);
                payload.append(char(0)); payload.append(char(0)); payload.append(char(0)); payload.append(char(0));
            }
            continue;
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

        QByteArray bytes = value.toUtf8();
        if (maxLen > 0 && maxLen < 0xFFFF) bytes = truncateUtf8Bytes(value, int(maxLen));

        payload.append(char(attrId));
        pushU16le(payload, quint16(bytes.size()));
        payload.append(bytes);
    }

    // MESSAGE_SIZE last
    if (msgSizeEntry.has_value()) {
        const quint8 attrId = msgSizeEntry->first;
        const quint16 maxLen = msgSizeEntry->second;

        QString value = QString::number(body.toUtf8().size()); // matches Rust body.len()
        QByteArray bytes = value.toUtf8();
        if (maxLen > 0 && maxLen < 0xFFFF) bytes = truncateUtf8Bytes(value, int(maxLen));

        payload.append(char(attrId));
        pushU16le(payload, quint16(bytes.size()));
        payload.append(bytes);
    }

    const quint16 dataCrc = computeCrc16(payload);

    QByteArray m;
    m.append(char(0)); m.append(char(0));
    pushU16le(m, 5035);
    pushU16le(m, quint16(payload.size()));
    pushU16le(m, dataCrc);
    pushU16le(m, 0); // offset
    m.append(payload);

    const quint16 packetSize = quint16(m.size() + 2);
    overwriteU16le(m, 0, packetSize);

    const quint16 crc = computeChecksum(m);
    pushU16le(m, crc);
    return Result<QByteArray>::isOk(m);
}
