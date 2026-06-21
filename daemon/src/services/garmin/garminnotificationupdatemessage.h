#ifndef GARMINNOTIFICATIONUPDATEMESSAGE_H
#define GARMINNOTIFICATIONUPDATEMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"
#include "garmintypes.h"

#include <QObject>
class GarminNotificationUpdateMessage : public GarminGfdiMessage
{
public:
    GarminNotificationUpdateMessage(CommunicatorV2 *parent) {
        mCommunicator = parent;
    }
    void parse();
    QByteArray generateMessage();
    MessageId getMessageType() {return MessageId::NotificationUpdate; };


    NotificationUpdateType updateType = NotificationUpdateType::Add;
    NotificationType notificationType = NotificationType::Generic;
    bool hasActions = false;
    bool hasPicture = false;
    int notificationId=0;
    int count = 1;

};


#endif // GARMINNOTIFICATIONUPDATEMESSAGE_H
