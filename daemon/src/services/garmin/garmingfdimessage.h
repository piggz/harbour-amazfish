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





class GarminGfdiMessage : public ServiceCallback {
    Q_OBJECT
public:
    static QSharedPointer<GarminGfdiMessage> create(CommunicatorV2* parent=nullptr) {
        return QSharedPointer<GarminGfdiMessage>(new GarminGfdiMessage(parent));
    }

    explicit GarminGfdiMessage(CommunicatorV2* parent=nullptr) : mCommunicator(parent)
    {
    }
    /*
    explicit GarminGfdiMessage(const QByteArray& data=QByteArray(), CommunicatorV2* parent=nullptr) : mMessageBytes(data), mCommunicator(parent)
    {

    }
    */
    void setCommunicator(CommunicatorV2* comm);
    QByteArray getMessageBytes() { return mMessageBytes; };
    void onMessage(const QByteArray& data) override;
    void parse(const QByteArray& data);
    QByteArray getAckByteStream();
    QByteArray getOutgoingMessage();
    MessageId getMessageType() {return mMessageType; };
    bool toSend() {return mSendOutgoing;};

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
    void parseProtobufRequest(const QByteArray& data);
    void parseProtobufResponse(const QByteArray& data);
    void parseAuthNegotiation(const QByteArray &data);
    void parseResponse(const QByteArray& data);
    void parseUnknownMessage(const quint16 , const QByteArray& data);


protected:
    CommunicatorV2* mCommunicator;
    QByteArray mMessageBytes;
    static Result<QString> readLengthPrefixedString(const QByteArray& data, int& consumedBytes);
    QSharedPointer<GarminGfdiMessage> mStatusMessage;
    MessageId mMessageType=MessageId::Generic;
    QByteArray generateOutgoing() { return QByteArray(); };
    bool mSendOutgoing=true;


};

// -------------------- Generator (static functions) --------------------

class GfdiMessageGenerator {
public:
    static Result<QByteArray> ackResponse(quint16 messageId);
    static Result<QByteArray> weatherResponse(const WeatherRequestMessage& request);
    static Result<QByteArray> fitDefinitionMessage(const QByteArray& fitDefinitionData);
    static Result<QByteArray> fitDataMessage(const QByteArray& fitData);
    static Result<QByteArray> supportedFileTypesRequest();
    static Result<QByteArray> deviceSettings();

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

    static QByteArray truncateUtf8Bytes(const QString& s, int maxBytes);

};



#endif //GARMINGDFDIMESSAGE_H
