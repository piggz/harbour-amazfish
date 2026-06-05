#include "garminnotification.h"
#include "garmintypes.h"

#include <optional>


const uint MAX_PROTOBUF_CHUNK_SIZE = 3072;

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

    auto msg = NotificationUpdateMessageBuilder::create(
        NotificationUpdateType::Remove,
        type,
        count,
        id).build();

    QByteArray gfdi = wrapInGfdiEnvelope(5033, msg);

    // async send simulated
    if (m_communicator) return m_communicator->sendMessage("notification_remove", gfdi);
    else return false;
}

void GarminNotificationHandler::onNotification(NotificationSpec notification)
{
    qDebug() << Q_FUNC_INFO << "Garmin: sending notification";
    bool isUpdate = addNotificationToQueue(notification);
    NotificationUpdateType updateType = isUpdate ? NotificationUpdateType::Modify : NotificationUpdateType::Add;
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

    //bool hasPicture = notification.hasPicture;
    int count = getNotificationCount(notification.notificationType);
    qDebug() << Q_FUNC_INFO << "Garmin: Found " << count << " notifications of this type";

    QByteArray updateMsg = NotificationUpdateMessageBuilder::create(
                updateType,
                notification.notificationType,
                count,
                notification.id)
                .withActions(notification.hasActions)
                .build();

    QByteArray gfdi = wrapInGfdiEnvelope(5033, updateMsg);
    if (m_communicator) {
        //m_communicator->sendMessage("notification", gfdi);
        bool result = m_communicator->sendMessage("notification", gfdi);

        if (result) {
            qDebug() << Q_FUNC_INFO << "Garmin: Notification sent successfully";
        } else {
            qDebug() << Q_FUNC_INFO <<"Garmin: Notification could not be sent";
        }
    }
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

QByteArray GarminNotificationHandler::wrapInGfdiEnvelope(quint16 messageId, const QByteArray& payload)
{
    QByteArray msg;

    quint16 size = static_cast<quint16>(2 + 2 + payload.size() + 2);

    writeU16le(msg,size);

    writeU16le(msg,messageId);


    msg.append(payload);

    quint16 crc = computeCrc16(msg);
    writeU16le(msg,crc);

    //msg.append(char(crc & 0xFF));
    //msg.append(char((crc >> 8) & 0xFF));

    return msg;
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

