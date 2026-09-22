#include "garmingfdimessage.h"
#include <QChar>

#include "communicator_v2.h"
#include "garmindeviceinformationmessage.h"
#include "garmincurrenttimemessage.h"
#include "garminconfigurationmessage.h"
#include "garminnotificationsubscriptionmessage.h"
#include "garminnotificationcontrolmessage.h"
#include "garminsynchronizationmessage.h"
#include "garminweathermessage.h"
#include "garminfilterstatusmessage.h"
#include "garmingfdistatusmessage.h"
#include "garminprotobufmessage.h"
#include "garminauthnegotiationmessage.h"



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

    qDebug() << Q_FUNC_INFO << "Garmin: parsing GFDI Message " << data.toHex();

    int offset = 0;

    // Read message ID (2 bytes, little-endian)
    quint16 rawId = le16(data.constData() + offset);
    offset += 2;

    //qDebug() << Q_FUNC_INFO << "Garmin: raw msgid is " << rawId;


    // Check for sequence number (bit 15 set)
    // If bit 15 is set, the message ID is encoded with a sequence number
    // Format: [bit 15: 1] [bits 14-8: sequence] [bits 7-0: message_id - 5000]
    // We need to decode it: actual_id = (raw_id & 0xFF) + 5000
    quint16 msgId = rawId;
    if ((msgId & 0x8000) != 0) {
        msgId = (msgId & 0xFF) + 5000;
    }

    qDebug() << Q_FUNC_INFO << "Garmin: recieved GFDI message " << messageIdToString(msgId)->data();


    const auto mid = messageIdFromU16(msgId);
    if (!mid.has_value()) {
        qDebug() << Q_FUNC_INFO << "Garmin: Unknown GFDI message: " << msgId;
        // We have to ack the mesage as unsupported
        parseUnknownMessage(msgId, data.mid(offset));
        return;
    }

    // Check if message is Response
    if (*mid == MessageId::Response)
    {
        // Check if this is a filter status response - special-case

        if (offset + 2 < data.size()) {
            const quint16 orig = le16(data.constData() + offset);
            if (orig == 5007) {
                parseFilterStatus(data.mid(offset));
                return;
            }
        }

        parseResponse(data.mid(offset));
        return;
    }

    // Ack message
    /*
    QByteArray ackData = GfdiMessageGenerator::ackResponse(msgId);
    mCommunicator->sendMessage("ACK",ackData);
    */

    // Now handle packet
    qDebug() << Q_FUNC_INFO << "Garmin:  Received GFDI message: " << messageIdToString(msgId).value();
    switch (*mid) {
    case MessageId::DeviceInformation:
        parseDeviceInformation(data.mid(offset));
        return;
    case MessageId::Configuration:
        parseConfiguration(data.mid(offset),msgId);
        return;
    case MessageId::CurrentTimeRequest:
         parseCurrentTimeRequest(data.mid(offset));
         return;
    case MessageId::NotificationControl:
        parseNotificationControl(data.mid(offset));
        return;
    case MessageId::NotificationSubscription:
        parseNotificationSubscription(data.mid(offset));
        return;
        /*
    case MessageId::Synchronization:
        parseSynchronization(data.mid(offset));
        return;
    case MessageId::WeatherRequest:
        parseWeatherRequest(data.mid(offset));
        return;

    case  MessageId::ProtobufRequest:
        parseProtobufRequest(data.mid(offset));
        return;
    */
    case  MessageId::ProtobufResponse:
        parseProtobufResponse(data.mid(offset));
        return;
    case MessageId::AuthNegotiation:
        parseAuthNegotiation(data.mid(offset));
        return;

    default:
        // Send simple Ack
        QByteArray data = GfdiMessageGenerator::ackResponse(msgId);
        mCommunicator->sendMessage("ACK",data);

    }
}

// -------------------- Parser: per-message parsers --------------------
void GarminGfdiMessage::parseCurrentTimeRequest(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO;
    GarminCurrentTimeMessage* mesg = new GarminCurrentTimeMessage(mCommunicator);
    mesg->parse(data);
}

void GarminGfdiMessage::parseDeviceInformation(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO;
    GarminDeviceInformationMessage* mesg = new GarminDeviceInformationMessage(mCommunicator);
    mesg->parse(data);
}

void GarminGfdiMessage::parseConfiguration(const QByteArray& data, quint16 msgId)
{
    qDebug() << Q_FUNC_INFO;
    //This type needs Ack first as response is new message (special case)
    QByteArray ackData = GfdiMessageGenerator::ackResponse(msgId);
    mCommunicator->sendMessage("ACK",ackData);
    GarminConfigurationMessage* mesg = new GarminConfigurationMessage(mCommunicator);
    mesg->parse(data);
}

void GarminGfdiMessage::parseNotificationControl(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO <<"Garmin: handling NOTIFICATION CONTROL";
    GarminNotificationControlMessage* mesg = new GarminNotificationControlMessage(mCommunicator);
    mesg->parse(data);
}

void GarminGfdiMessage::parseNotificationSubscription(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO;
    GarminNotificationSubscriptionMessage* msg = new GarminNotificationSubscriptionMessage(mCommunicator);
    msg->parse(data);
}

void GarminGfdiMessage::parseSynchronization(const QByteArray& data)
{
    GarminSynchronizationMessage* msg = new GarminSynchronizationMessage(mCommunicator);
    msg->parse(data);
}

void GarminGfdiMessage::parseWeatherRequest(const QByteArray& data)
{
    GarminWeatherMessage* msg = new GarminWeatherMessage(mCommunicator);
    msg->parse(data);
}

void GarminGfdiMessage::parseFilterStatus(const QByteArray& data)
{
    GarminFilterStatusMessage* msg = new GarminFilterStatusMessage(mCommunicator);
    msg->parse(data);
}

void GarminGfdiMessage::parseResponse(const QByteArray& data)
{
    GarminGfdiStatusMessage* msg = new GarminGfdiStatusMessage(mCommunicator);
    msg->parse(data);
}


void GarminGfdiMessage::parseProtobufResponse(const QByteArray& data)
{
    //GarminProtobufStatusMessage msg(data, mCommunicator);
    qDebug() << Q_FUNC_INFO << "Garmin: Received Protobuf response" << data.toHex();
    qDebug() << Q_FUNC_INFO;
    if (mCommunicator) mCommunicator->onProtobufMessageReceived(data);
    //msg->parse(data);
}

void GarminGfdiMessage::parseProtobufRequest(const QByteArray &data)
{
    //GarminProtobufMessage msg(data,mCommunicator);
    qDebug() << Q_FUNC_INFO << "Garmin: Received Protobuf request" << data.toHex();
    if (mCommunicator) mCommunicator->onProtobufMessageReceived(data);
    //msg->parse(data);

}

void GarminGfdiMessage::parseUnknownMessage(const quint16 msgId, const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO << "Garmin: parsing unknown message ";;
    // Build packet and send back to watch
    QByteArray payload;
    writeU16le(payload, msgId);
    payload.append(char(quint8(Status::Unsupported)));
    mCommunicator->sendMessage("UNSUPPORTED_MESSAGE",wrapInGfdiEnvelope(MessageId::Response,payload));
}

void GarminGfdiMessage::parseAuthNegotiation(const QByteArray &data) {
    qDebug() << Q_FUNC_INFO << "Garmin: Received Authentication request" << data.toHex();
    GarminAuthNegotiationMessage* msg = new GarminAuthNegotiationMessage(mCommunicator);
    msg->parse(data);
}

QByteArray GarminGfdiMessage::getOutgoingMessage(){
    QByteArray message;
    QByteArray toSend = generateOutgoing();
    if (toSend.isEmpty())
        return toSend;
    message=wrapInGfdiEnvelope(mMessageType,toSend);
    return message;
}


QByteArray GarminGfdiMessage::getAckByteStream() {
    if (mStatusMessage.isNull()) {
        return QByteArray();
    }
    return mStatusMessage->getOutgoingMessage();
}



// MessageGenerator is used to generate messages for sending to the device

// -------------------- Generator helpers --------------------




QByteArray GfdiMessageGenerator::truncateUtf8Bytes(const QString& s, int maxBytes) {
    QByteArray utf8 = s.toUtf8();
    if (utf8.size() <= maxBytes) return utf8;

    int boundary = maxBytes;
    // Back up while in a UTF-8 continuation byte (10xxxxxx)
    while (boundary > 0 && (quint8(utf8[boundary]) & 0xC0u) == 0x80u) boundary--;
    return utf8.left(boundary);
}



// -------------------- Generator: public API --------------------



QByteArray GfdiMessageGenerator::ackResponse(quint16 messageId)
{
    QByteArray r;
    writeU16le(r, messageId);
    r.append(char(quint8(Status::Ack)));
    return wrapInGfdiEnvelope(MessageId::Response,r);
}

QByteArray GfdiMessageGenerator::weatherResponse(const WeatherRequestMessage&)
{
    // Generate a weather response ACK
    //
    // This sends a simple ACK to weather requests. The actual weather data
    // is sent separately via FIT messages (FitDefinition and FitData).
    //
    // # Arguments
    // * `WeatherRequestMessage` - The incoming WeatherRequestMessage

    QByteArray r;

    // Original message ID: WEATHER_REQUEST (5014)
    QByteArray data = GfdiMessageGenerator::ackResponse(5014);
    return data;
}

QByteArray GfdiMessageGenerator::fitDefinitionMessage(const QByteArray& fitDefinitionData)
{
    // Generate a FIT Definition message (5011)
    //
    // This wraps FIT definition data in a GFDI message envelope for BLE transmission.
    //
    // # Arguments
    // * `fit_definition_data` - The encoded FIT definition message bytes
    //
    QByteArray m;
    // FIT definition payload
    m.append(fitDefinitionData);
    return wrapInGfdiEnvelope(MessageId::FitDefinition, m);
}

QByteArray GfdiMessageGenerator::fitDataMessage(const QByteArray& fitData)
{
    // Generate a FIT Data message (5012)
    //
    // This wraps FIT data in a GFDI message envelope for BLE transmission.
    //
    // # Arguments
    // * `fitData` - The encoded FIT data message bytes
    QByteArray m;
    // Packet size placeholder
    // FIT data payload
    m.append(fitData);
    return wrapInGfdiEnvelope(MessageId::FitData,m);
}



QByteArray GfdiMessageGenerator::supportedFileTypesRequest()
{
    // Generate a SupportedFileTypesRequest message
    //
    // This proactively asks the watch what file types it supports.
    // Sent after Configuration exchange during initialization.
    //
    // Message format: Just the message ID, no payload
    QByteArray m;

    return wrapInGfdiEnvelope(MessageId::SupportedFileTypesRequest,m);
}

QByteArray GfdiMessageGenerator::deviceSettings()
{
    // Matches GarminSupport.sendDeviceSettings(): auto-upload + weather
    // conditions enabled, weather alerts disabled. Each setting is
    // [ordinal][valueLen][value...]; booleans are a single byte.
    QByteArray payload;
    payload.append(char(3)); // setting count
    payload.append(char(quint8(GfdiDeviceSetting::AutoUploadEnabled)));
    payload.append(char(1));
    payload.append(char(1));
    payload.append(char(quint8(GfdiDeviceSetting::WeatherConditionsEnabled)));
    payload.append(char(1));
    payload.append(char(1));
    payload.append(char(quint8(GfdiDeviceSetting::WeatherAlertsEnabled)));
    payload.append(char(1));
    payload.append(char(0));
    return wrapInGfdiEnvelope(MessageId::DeviceSettings, payload);
}

QByteArray GfdiMessageGenerator::systemEvent(quint8 eventType, quint8 value)
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

    m.append(char(eventType));
    m.append(char(value));

    return wrapInGfdiEnvelope(MessageId::SystemEvent, m);
}

