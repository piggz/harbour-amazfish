#ifndef GARMINNOTIFICATIONSUBSCRIPTIONMESSAGE_H
#define GARMINNOTIFICATIONSUBSCRIPTIONMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"

#include <QObject>

// Notification Subscription Message (incoming from watch)


class GarminNotificationSubscriptionMessage : public GarminGfdiMessage
{
    Q_OBJECT
public:
    explicit GarminNotificationSubscriptionMessage(CommunicatorV2* parent=nullptr, bool enable = true )  {
        mCommunicator = parent;
        mWatchProposal=enable;
    }

    void parse(const QByteArray& data);
    MessageId getMessageType() {return MessageId::NotificationSubscription; };
private:
    QByteArray generateStatusMessage();
    bool mWatchProposal;
};

#endif // GARMINNOTIFICATIONSUBSCRIPTIONMESSAGE_H
