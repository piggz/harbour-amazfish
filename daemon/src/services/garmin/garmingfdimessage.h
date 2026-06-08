#ifndef GARMINGDFDIMESSAGE_H
#define GARMINGDFDIMESSAGE_H
#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QVector>
#include <QPair>
#include <QSet>
#include <QDateTime>
#include <QtGlobal>
#include <QSharedPointer>
#include <optional>

#include "garmintypes.h"

// Forward declarations
class CommunicatorV2;




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


class GarminGfdiMessage : public ServiceCallback {
    Q_OBJECT
public:
    static QSharedPointer<GarminGfdiMessage> create(CommunicatorV2* parent=nullptr) {
        return QSharedPointer<GarminGfdiMessage>(new GarminGfdiMessage(parent));
    }
    explicit GarminGfdiMessage(CommunicatorV2* parent=nullptr) : mCommunicator(parent)
    {
    }
    void setCommunicator(CommunicatorV2* comm);
    void onMessage(const QByteArray& data) override;

    void parse(const QByteArray& data);

public slots:
    // Convenience slot: parse and emit appropriate signal
    //void parseAndEmit(const QByteArray& data);


private:
    void parseCurrentTimeRequest(const QByteArray& data);
    void parseDeviceInformation(const QByteArray& data);
    void parseConfiguration(const QByteArray& data);
    void parseNotificationControl(const QByteArray& data);
    void parseNotificationSubscription(const QByteArray& data);
    void parseSynchronization(const QByteArray& data);
    void parseWeatherRequest(const QByteArray& data);
    void parseFilterStatus(const QByteArray& data);
    void parseUnknownMessage(const quint16 , const QByteArray& data);


protected:
    CommunicatorV2* mCommunicator;
    static Result<QString> readLengthPrefixedString(const QByteArray& data, int& consumedBytes);


};

// -------------------- Generator (static functions) --------------------

class GfdiMessageGenerator {
public:
    static Result<QByteArray> ackResponse(quint16 messageId);
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
    static QByteArray truncateUtf8Bytes(const QString& s, int maxBytes);

    // LE push/overwrite helpers

    static void overwriteU16le(QByteArray& out, int off, quint16 v);
};



#endif //GARMINGDFDIMESSAGE_H
