#ifndef GARMINNOTIFICATIONSUBSCRIPTIONMESSAGE_H
#define GARMINNOTIFICATIONSUBSCRIPTIONMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"

#include <QObject>

// Notification Subscription Message (incoming from watch)

struct NotificationSubscriptionMessage {
    bool enable{};
    quint8 unk{};
};

class GarminNotificationSubscriptionMessage : public GarminGfdiMessage
{
    Q_OBJECT
public:
    explicit GarminNotificationSubscriptionMessage(CommunicatorV2* parent=nullptr, bool enable = true, int unk=0)  {
        mCommunicator = parent;
        mMessage.unk=unk;
        mMessage.enable=enable;
    }

    void parse(const QByteArray& data);
    MessageId getMessageType() {return MessageId::NotificationSubscription; };
private:
    QByteArray generateStatusMessage();
    NotificationSubscriptionMessage mMessage;
};

class GarminNotificationSubscriptionStatusMessage : public GarminGfdiMessage
{
    Q_OBJECT
public:
    explicit GarminNotificationSubscriptionStatusMessage(CommunicatorV2* parent=nullptr, bool enable = true, int unk=0)  {
        mCommunicator = parent;
        mStatus=Status::Ack;
        mNotificationStatus = true; // TODO: check in Amazfish config if enabled
        mUnk=unk;
        mEnableRaw=enable;
    }

    QByteArray getOutgoingMessage();
    MessageId getMessageType() {return MessageId::NotificationSubscription; };
private:
    Status mStatus;
    bool mNotificationStatus;
    bool  mEnableRaw;
    int mUnk;
 };

#endif // GARMINNOTIFICATIONSUBSCRIPTIONMESSAGE_H
