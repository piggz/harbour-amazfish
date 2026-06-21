#include "garminnotificationhandler.h"
#include "garmintypes.h"
#include "garminnotificationupdatemessage.h"

#include <optional>


//const uint MAX_PROTOBUF_CHUNK_SIZE = 3072;

void GarminNotificationHandler::cleanupOldNotifications()
{
    //QMutexLocker lock(&m_mutex);

    if (m_lastCleanup.elapsed() < 120000) // 2 minutes
        return;

    m_lastCleanup.restart();

    QList<qint32> toRemove;

    const qint64 now = QDateTime::currentMSecsSinceEpoch()*1000;

    for (auto it = m_storedNotifications.begin(); it != m_storedNotifications.end(); ++it) {
        const auto& spec = it.value();

        if (!spec.retrieved &&
            (now - spec.when) > 600) { // 10 minutes
            toRemove.append(it.key());
        }
    }

    for (qint32 id : toRemove) {
        removeNotification(id);
    }

    if (m_storedNotifications.size() > 30) {
        QList<qint32> all;
        for (auto it = m_storedNotifications.begin(); it != m_storedNotifications.end(); ++it)
            all.append(it.key());

        for (qint32 id : all)
            removeNotification(id);
    }
}

bool GarminNotificationHandler::removeNotification(qint32 id)
{
    NotificationType type = NotificationType::Generic;
    int count = 0;

    {
        //QMutexLocker lock(&m_mutex);
        if (m_storedNotifications.contains(id)) {
            type = m_storedNotifications[id].notificationType;
            count = getNotificationCount(type);
            m_storedNotifications.remove(id);
        }
    }

    CommunicatorV2 *com = m_communicator.data();
    GarminNotificationUpdateMessage* msg = new GarminNotificationUpdateMessage(com);
    msg->notificationType=type;
    msg->updateType=NotificationUpdateType::Remove;
    msg->count=count;
    msg->notificationId=id;
    msg->parse();
    return true;
}

void GarminNotificationHandler::onNotification(NotificationSpec notification)
{
    qDebug() << Q_FUNC_INFO << "Garmin: sending notification";
    CommunicatorV2 *com = m_communicator.data();
    GarminNotificationUpdateMessage* updateMessage = new GarminNotificationUpdateMessage(com);
    bool isUpdate = addNotificationToQueue(notification);
    //TEST:
    isUpdate = false;
    qDebug() << Q_FUNC_INFO << "Garmin: notification isupdate=" <<isUpdate;
    updateMessage->updateType = isUpdate ? NotificationUpdateType::Modify : NotificationUpdateType::Add;
    if (m_storedNotifications.size() > 30)
        m_storedNotifications.erase(m_storedNotifications.end()); //remove the oldest notification TODO: should send a delete notification message to watch!

    bool  hasActions = (notification.hasActions);
    if (hasActions) {

        for (auto it = notification.attachedActions.begin(); it!= notification.attachedActions.end(); it++) {
             Action action = *(it->data());
             if (action.isReply()) {
                mNotificationReplyAction.insert(notification.id, action.handle);
            }
        }

    }
    updateMessage->hasActions=hasActions;
    updateMessage->notificationId=notification.id;
    updateMessage->notificationType=notification.notificationType;

    //bool hasPicture = notification.hasPicture;
    updateMessage->count = getNotificationCount(notification.notificationType);
    qDebug() << Q_FUNC_INFO << "Garmin: Found " << updateMessage->count << " notifications of this type";

    updateMessage->parse();

}




void GarminNotificationHandler::replayMissedNotifications()
{
    QList<NotificationSpec> list;

    {
        //QMutexLocker lock(&m_mutex);
        while (!m_missedNotifications.isEmpty())
            list.append(m_missedNotifications.dequeue());
    }

    for (const auto& n : list) {
        onNotification(n);
        QThread::msleep(100);
    }
}

void GarminNotificationHandler::setConnected(bool v)
{
    //QMutexLocker lock(&m_mutex);
    m_isConnected = v;
}

void GarminNotificationHandler::onSetCallState(const CallSpec& call)
{
    qDebug() << Q_FUNC_INFO;

    qint32 id;
    if (call.number.isEmpty()) id = qHash(call.number);
            else id = qHash("Amazfish Call");

    if (call.command == CallCommand::Incoming) {
        NotificationSpec notif =
            NotificationSpec::create(id, NotificationType::GenericPhone)
            .withTitle(call.name.value_or("Incoming Call"))
            .withBody(call.name.value_or("Unknown"))
            .withSender(call.sourceName.value_or("Phone"))
            .withBody(call.name.value_or("Unknown"))
            .withActions(true);

        // add an empty bogus action to toggle the hasActions boolean. The actions are hardcoded on the watch in case of incoming calls.
        notif.attachedActions.empty();
        notif.attachedActions.insert(0, QSharedPointer(new Action()));


        qDebug() << Q_FUNC_INFO << "Garmin: Sending incoming call notification";
        onNotification(notif);
    }
    else if (call.command == CallCommand::End ||
             call.command == CallCommand::Reject)
    {
        //QMutexLocker lock(&m_mutex);
        qDebug() << Q_FUNC_INFO << "Garmin: Sending call ended notification";

        bool result = removeNotification(id);

        if (result) {
            qDebug() << Q_FUNC_INFO << "Garmin: Notification sent successfully";
        } else {
            qDebug() << Q_FUNC_INFO <<"Garmin: Notification could not be sent";
            return; //result
        }
    }
}


bool GarminNotificationHandler::addNotificationToQueue(NotificationSpec note) {
    bool found = false;
    if (m_storedNotifications.contains(note.id)){
        found = true;
        m_storedNotifications.remove(note.id);
    }
    m_storedNotifications.begin();
    m_storedNotifications.insert(note.id,note);
    return found;
}

int GarminNotificationHandler::getNotificationCount(NotificationType type) {
    int count = 0;
    for (auto it =m_storedNotifications.begin(); it != m_storedNotifications.end();) {
        count += it.value().notificationType == type ? 1 : 0;
        ++it;
    }
    return count;
}

