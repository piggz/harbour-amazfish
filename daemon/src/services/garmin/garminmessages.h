#ifndef GARMINMESSAGES__H
#define GARMINMESSAGES__H
#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QVector>
#include <QPair>
#include <QSet>
#include <QDateTime>
#include <QtGlobal>
#include <optional>

#include "garmintypes.h"
#include <variant>



static inline quint16 le16(const char* p) {
    return quint16(quint8(p[0]) | (quint16(quint8(p[1]))<<8));
}
static inline quint32 le32(const char* p) {
    return quint32(quint8(p[0]) | (quint32(quint8(p[1]))<<8) | (quint32(quint8(p[2]))<<16) | (quint32(quint8(p[3]))<<24));
}
static inline quint64 le64(const char* p) {
    quint64 v=0;
    for (int i=0;i<8;++i) v |= (quint64(quint8(p[i])) << (8*i));
    return v;
}

// -------------------- Enums --------------------

enum class MessageId : quint16 {
    Response = 5000,
    DownloadRequest = 5002,
    UploadRequest = 5003,
    FileTransferData = 5004,
    CreateFile = 5005,
    Filter = 5007,
    SetFileFlag = 5008,
    FitDefinition = 5011,
    FitData = 5012,
    WeatherRequest = 5014,
    DeviceInformation = 5024,
    DeviceSettings = 5026,
    SystemEvent = 5030,
    SupportedFileTypesRequest = 5031,
    NotificationUpdate = 5033,
    NotificationControl = 5034,
    NotificationData = 5035,
    NotificationSubscription = 5036,
    Synchronization = 5037,
    FindMyPhoneRequest = 5039,
    FindMyPhoneCancel = 5040,
    MusicControl = 5041,
    MusicControlCapabilities = 5042,
    ProtobufRequest = 5043,
    ProtobufResponse = 5044,
    MusicControlEntityUpdate = 5049,
    Configuration = 5050,
    CurrentTimeRequest = 5052,
    AuthNegotiation = 5101,
};

inline std::optional<MessageId> messageIdFromU16(quint16 id) {
    switch (id) {
    case 5000: return MessageId::Response;
    case 5002: return MessageId::DownloadRequest;
    case 5003: return MessageId::UploadRequest;
    case 5004: return MessageId::FileTransferData;
    case 5005: return MessageId::CreateFile;
    case 5007: return MessageId::Filter;
    case 5008: return MessageId::SetFileFlag;
    case 5011: return MessageId::FitDefinition;
    case 5012: return MessageId::FitData;
    case 5014: return MessageId::WeatherRequest;
    case 5024: return MessageId::DeviceInformation;
    case 5026: return MessageId::DeviceSettings;
    case 5030: return MessageId::SystemEvent;
    case 5031: return MessageId::SupportedFileTypesRequest;
    case 5033: return MessageId::NotificationUpdate;
    case 5034: return MessageId::NotificationControl;
    case 5035: return MessageId::NotificationData;
    case 5036: return MessageId::NotificationSubscription;
    case 5037: return MessageId::Synchronization;
    case 5039: return MessageId::FindMyPhoneRequest;
    case 5040: return MessageId::FindMyPhoneCancel;
    case 5041: return MessageId::MusicControl;
    case 5042: return MessageId::MusicControlCapabilities;
    case 5043: return MessageId::ProtobufRequest;
    case 5044: return MessageId::ProtobufResponse;
    case 5049: return MessageId::MusicControlEntityUpdate;
    case 5050: return MessageId::Configuration;
    case 5052: return MessageId::CurrentTimeRequest;
    case 5101: return MessageId::AuthNegotiation;
    default: return std::nullopt;
    }
}

inline std::optional<QString> messageIdToString(quint16 id) {
    switch (id) {
    case 5000: return "Response";
    case 5002: return "DownloadRequest";
    case 5003: return "UploadRequest";
    case 5004: return "FileTransferData";
    case 5005: return "CreateFile";
    case 5007: return "Filter";
    case 5008: return "SetFileFlag";
    case 5011: return "FitDefinition";
    case 5012: return "FitData";
    case 5014: return "WeatherRequest";
    case 5024: return "DeviceInformation";
    case 5026: return "DeviceSettings";
    case 5030: return "SystemEvent";
    case 5031: return "SupportedFileTypesRequest";
    case 5033: return "NotificationUpdate";
    case 5034: return "NotificationControl";
    case 5035: return "NotificationData";
    case 5036: return "NotificationSubscription";
    case 5037: return "Synchronization";
    case 5039: return "FindMyPhoneRequest";
    case 5040: return "FindMyPhoneCancel";
    case 5041: return "MusicControl";
    case 5042: return "MusicControlCapabilities";
    case 5043: return "ProtobufRequest";
    case 5044: return "ProtobufResponse";
    case 5049: return "MusicControlEntityUpdate";
    case 5050: return "Configuration";
    case 5052: return "CurrentTimeRequest";
    case 5101: return "AuthNegotiation";
    default: return "Unknown";
    //default: return std::nullopt;
    }
}


/// Status codes for response messages
///
enum class Status : quint8 {
    Ack = 0,
    Nack = 1,
    Unsupported = 2,
    DecodeError = 3,
    CrcError = 4,
    LengthError = 5,
};

inline std::optional<Status> statusFromU8(quint8 code) {
    switch (code) {
    case 0: return Status::Ack;
    case 1: return Status::Nack;
    case 2: return Status::Unsupported;
    case 3: return Status::DecodeError;
    case 4: return Status::CrcError;
    case 5: return Status::LengthError;
    default: return std::nullopt;
    }
}

inline QString statusName(Status s) {
    switch (s) {
    case Status::Ack: return "ACK";
    case Status::Nack: return "NACK";
    case Status::Unsupported: return "UNSUPPORTED";
    case Status::DecodeError: return "DECODE_ERROR";
    case Status::CrcError: return "CRC_ERROR";
    case Status::LengthError: return "LENGTH_ERROR";
    }
    return "UNKNOWN";
}

// -------------------- Parsed message structs --------------------
// Device Information Message (incoming from watch)


struct DeviceInformationMessage {
    quint16 protocolVersion{};
    quint16 productNumber{};
    quint32 unitNumber{};
    quint16 softwareVersion{};
    quint16 maxPacketSize{};
    QString bluetoothFriendlyName;
    QString deviceName;
    QString deviceModel;
};

// Configuration Message (incoming from watch)

struct ConfigurationMessage {
    QSet<quint16> capabilities;
};

// Notification Control Message (incoming from watch)

struct NotificationControlMessage {
    qint32 notificationId{};
    quint8 command{};
    QVector<QPair<quint8, quint16>> attributes; // (attribute_id, max_length)
    std::optional<quint8> actionId;
    std::optional<QString> actionString;
};

// Notification Subscription Message (incoming from watch)

struct NotificationSubscriptionMessage {
    bool enable{};
    quint8 unk{};
};

// Synchronization Message (incoming from watch)

struct SynchronizationMessage {
    quint8 synchronizationType{};
    quint64 fileTypeBitmask{};

    bool shouldProceed() const {
        constexpr quint64 WORKOUTS = 1ull << 3;
        constexpr quint64 ACTIVITIES = 1ull << 5;
        constexpr quint64 ACTIVITY_SUMMARY = 1ull << 21;
        constexpr quint64 SLEEP = 1ull << 26;
        return (fileTypeBitmask & (WORKOUTS | ACTIVITIES | ACTIVITY_SUMMARY | SLEEP)) != 0;
    }
};

// Filter Status Message (response to Filter message)

struct FilterStatusMessage {
    Status status{};
    quint8 filterType{};
};

// Weather Request Message (5014)

struct WeatherRequestMessage {
    quint8 format{};
    qint32 latitude{};
    qint32 longitude{};
    quint8 hoursOfForecast{};
};

struct UnknownMessage {
    quint16 messageId {0};
    QByteArray data;
};


using GfdiMessage = std::variant<
    DeviceInformationMessage,
    ConfigurationMessage,
    std::monostate, // CurrentTimeRequest
    NotificationControlMessage,
    NotificationSubscriptionMessage,
    SynchronizationMessage,
    FilterStatusMessage,
    WeatherRequestMessage,
    UnknownMessage
>;


// -------------------- Parser (QObject + signals) --------------------

class GfdiMessageParser : public QObject {
    Q_OBJECT
public:
    explicit GfdiMessageParser(QObject* parent=nullptr) : QObject(parent) {}

public slots:
    // Convenience slot: parse and emit appropriate signal
    //void parseAndEmit(const QByteArray& data);

public:
    void parse(const QByteArray& data);

signals:
    void deviceInformationReceived(DeviceInformationMessage& msg);
    void configurationReceived(const ConfigurationMessage& msg);
    void currentTimeRequestReceived();
    void notificationControlReceived(const NotificationControlMessage& msg);
    void notificationSubscriptionReceived(const NotificationSubscriptionMessage& msg);
    void synchronizationReceived(const SynchronizationMessage& msg);
    void filterStatusReceived(const FilterStatusMessage& msg);
    void weatherRequestReceived(const WeatherRequestMessage& msg);
    void unknownMessageReceived(quint16 messageId, const QByteArray& payload);
    void parseError(const QString& error);

private:
    void parseDeviceInformation(const QByteArray& data);
    void parseConfiguration(const QByteArray& data);
    void parseNotificationControl(const QByteArray& data);
    void parseNotificationSubscription(const QByteArray& data);
    void parseSynchronization(const QByteArray& data);
    void parseWeatherRequest(const QByteArray& data);
    void parseFilterStatus(const QByteArray& data);

    static Result<QString> readLengthPrefixedString(const QByteArray& data, int& consumedBytes);
    static QSet<quint16> parseCapabilities(const QByteArray& bytes);

    // LE helpers
    static quint16 u16le(const QByteArray& b, int off);
    static quint32 u32le(const QByteArray& b, int off);
    static quint64 u64le(const QByteArray& b, int off);
    static qint32  i32le(const QByteArray& b, int off);
};

// -------------------- Generator (static functions) --------------------

class GfdiMessageGenerator {
public:
    static Result<QByteArray> deviceInformationResponse(const DeviceInformationMessage& incoming);
    static Result<QByteArray> configurationResponse();
    static Result<QByteArray> ackResponse(quint16 messageId);
    static Result<QByteArray> currentTimeResponse();
    static Result<QByteArray> filterMessage(quint8 filterType);
    static Result<QByteArray> synchronizationAck();
    static Result<QByteArray> weatherResponse(const WeatherRequestMessage& request);
    static Result<QByteArray> fitDefinitionMessage(const QByteArray& fitDefinitionData);
    static Result<QByteArray> fitDataMessage(const QByteArray& fitData);
    static Result<QByteArray> notificationSubscriptionResponse(const NotificationSubscriptionMessage& incoming, bool enabled);
    static Result<QByteArray> supportedFileTypesRequest();
    static Result<QByteArray> setDeviceSettings(bool autoUpload, bool weatherConditions, bool weatherAlerts);
    static Result<QByteArray> systemEvent(quint8 eventType, quint8 value);
    static Result<QByteArray> protobufBatteryStatusRequest(quint16 requestId);

    static Result<QByteArray> notificationData(quint32 notificationId,
                                                     const QVector<QPair<quint8, quint16>>& requestedAttributes,
                                                     const QString& title,
                                                     const QString& body,
                                                     const QString& sender,
                                                     const QString& timestamp,
                                                     const QString& appId);

    static Result<QByteArray> notificationDataWithActions(quint32 notificationId,
                                                                const QVector<QPair<quint8, quint16>>& requestedAttributes,
                                                                const QString& title,
                                                                const QString& body,
                                                                const QString& sender,
                                                                const QString& timestamp,
                                                                const QString& appId,
                                                                bool hasDismissAction);

private:
    static Result<QByteArray> generateCapabilities();
    static quint16 computeChecksum(const QByteArray& data);
    static quint16 computeCrc16(const QByteArray& data);
    static QByteArray truncateUtf8Bytes(const QString& s, int maxBytes);

    // LE push/overwrite helpers
    static void pushU16le(QByteArray& out, quint16 v);
    static void pushU32le(QByteArray& out, quint32 v);
    static void overwriteU16le(QByteArray& out, int off, quint16 v);
};

// Qt metatype declarations so these structs can be used in queued signals
Q_DECLARE_METATYPE(DeviceInformationMessage)
Q_DECLARE_METATYPE(ConfigurationMessage)
Q_DECLARE_METATYPE(NotificationControlMessage)
Q_DECLARE_METATYPE(NotificationSubscriptionMessage)
Q_DECLARE_METATYPE(SynchronizationMessage)
Q_DECLARE_METATYPE(FilterStatusMessage)
Q_DECLARE_METATYPE(WeatherRequestMessage)

#endif //GARMINMESSAGES__H
