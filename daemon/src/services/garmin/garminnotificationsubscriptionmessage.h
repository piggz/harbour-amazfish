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
    explicit GarminNotificationSubscriptionMessage(CommunicatorV2* parent=nullptr)  {
        mCommunicator = parent;

    }
    void parse(const QByteArray& data);
    MessageId getMessageType() {return MessageId::NotificationSubscription; };
private:
    QByteArray generateStatusMessage(NotificationSubscriptionMessage& msg);
};

#endif // GARMINNOTIFICATIONSUBSCRIPTIONMESSAGE_H
