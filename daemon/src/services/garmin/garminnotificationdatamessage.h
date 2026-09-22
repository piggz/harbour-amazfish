#ifndef GARMINNOTIFICATIONDATAMESSAGE_H
#define GARMINNOTIFICATIONDATAMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"
#include "garmintypes.h"

#include <QObject>

class GarminNotificationDataMessage : public QObject
{
    Q_OBJECT
public:
    GarminNotificationDataMessage(CommunicatorV2 *parent) {
        mCommunicator = parent;
    };

    QByteArray getNotificationDataMessage(const NotificationControlMessage& msg, const NotificationSpec& spec);
private:
    CommunicatorV2 *mCommunicator = nullptr;

signals:

};

#endif // GARMINNOTIFICATIONDATAMESSAGE_H
