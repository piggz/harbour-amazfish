#include "garminnotificationhandler.h"
#include "garmintypes.h"
#include "garminnotificationupdatemessage.h"

#include <optional>


//const uint MAX_PROTOBUF_CHUNK_SIZE = 3072;


const quint8 NOTIF_CMD_GET_NOTIFICATION_ATTRIBUTES = 0;

const quint8 NOTIF_ATTR_APP_IDENTIFIER = 0;
const quint8 NOTIF_ATTR_TITLE = 1;
const quint8 NOTIF_ATTR_SUBTITLE = 2;
const quint8 NOTIF_ATTR_MESSAGE = 3;
const quint8 NOTIF_ATTR_MESSAGE_SIZE = 4;
const quint8 NOTIF_ATTR_DATE = 5;
const quint8 NOTIF_ATTR_ACTIONS = 127;
const quint8 NOTIF_ATTR_ATTACHMENTS = 128;

bool notificationAttributeHasLengthParam(quint8 code)
{
    return code == NOTIF_ATTR_TITLE || code == NOTIF_ATTR_SUBTITLE || code == NOTIF_ATTR_MESSAGE;
}

bool notificationAttributeHasAdditionalParams(quint8 code)
{
    return code == NOTIF_ATTR_ACTIONS;
}

// std::nullopt means "we don't supply this attribute" (the TLV is omitted
// entirely, matching how Gadgetbridge just skips ATTACHMENTS/etc. when the
// notification doesn't have one).
std::optional<QByteArray> notificationAttributeValue(quint8 code, int maxLength, const QString &appIdentifier, const QString &title, const QString &message)
{
    QString text;
    switch (code) {
    case NOTIF_ATTR_APP_IDENTIFIER:
        text = appIdentifier;
        break;
    case NOTIF_ATTR_TITLE:
        text = title;
        break;
    case NOTIF_ATTR_SUBTITLE:
        text = QString();
        break;
    case NOTIF_ATTR_MESSAGE:
        text = message;
        break;
    case NOTIF_ATTR_MESSAGE_SIZE:
        text = QString::number(message.length());
        break;
    case NOTIF_ATTR_DATE:
        text = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd'T'HHmmss"));
        break;
    case NOTIF_ATTR_ACTIONS:
        // No attached actions: 4 zero bytes is the "no actions" marker.
        return QByteArray(4, char(0));
    case NOTIF_ATTR_ATTACHMENTS:
        return std::nullopt; // we never advertise a picture, so never claim one
    default:
        // Never silently omit an attribute the watch explicitly asked for -
        // it treats an incomplete reply as unsatisfactory and re-requests
        // forever. Reply with an empty value instead when we have nothing
        // real to say, matching Gadgetbridge's own attribute encoder (its
        // switch has no case for every possible code either, but its
        // default still always encodes something).
        return QByteArray();
    }

    if (maxLength > 0)
        text = text.left(maxLength);
    return text.toUtf8();
}



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
    if (!com->handshakeComplete()) return;
    GarminNotificationUpdateMessage* updateMessage = new GarminNotificationUpdateMessage(com);
    bool isUpdate = addNotificationToQueue(notification);

    qDebug() << Q_FUNC_INFO << "Garmin: notification isupdate=" <<isUpdate;
    //Test
    isUpdate = false;
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

    //TESTING:
    updateMessage->hasActions=false;
    updateMessage->hasPicture=false;
    updateMessage->notificationType=NotificationType::GenericSms;
    updateMessage->count=1;
    //bool hasPicture = notification.hasPicture;
    updateMessage->count = getNotificationCount(notification.notificationType);
    qDebug() << Q_FUNC_INFO << "Garmin: Found " << updateMessage->count << " notifications of this type";

    updateMessage->parse();

}


void GarminNotificationHandler::onNotificationDataRequested(const NotificationControlMessage& msg)
{
    qDebug() << Q_FUNC_INFO << "Garmin: Notification Data requested, looking up data";
    QByteArray attributes;
    attributes.append(char(msg.command));
    writeU32le(attributes, msg.notificationId);

    QByteArray messageSizeAttribute;

    int pos = 0;
    while (pos < msg.data.size()) {
        const quint8 code = quint8(msg.data.at(pos++));
        int maxLength = 0;
        if (notificationAttributeHasLengthParam(code)) {
            if (pos + 2 > msg.data.size())
                break;
            maxLength = u16le(msg.data, pos);
            pos += 2;
        } else if (notificationAttributeHasAdditionalParams(code)) {
            if (pos + 3 > msg.data.size())
                break;
            maxLength = u16le(msg.data, pos);
            pos += 3; // 2-byte param + 1 unknown byte we don't use
        }

        QString source, title, content;
        if (m_storedNotifications.contains(msg.notificationId)) {
            source = m_storedNotifications[msg.notificationId].sourceName;
            title = m_storedNotifications[msg.notificationId].title;
            content = m_storedNotifications[msg.notificationId].body;
        }
        const std::optional<QByteArray> value = notificationAttributeValue(code, maxLength, source, title, content);
        if (!value)
            continue;

        QByteArray tlv;
        tlv.append(char(code));
        writeU16le(tlv, value->size());
        tlv.append(*value);

        if (code == NOTIF_ATTR_MESSAGE_SIZE)
            messageSizeAttribute = tlv;
        else
            attributes.append(tlv);
    }
    attributes.append(messageSizeAttribute);


    QByteArray dataPayload;
    writeU16le(dataPayload, attributes.size()); // messageSize
    writeU16le(dataPayload, computeCrc16(attributes)); // crc of this (only) chunk
    writeU16le(dataPayload, 0); // dataOffset
    dataPayload.append(attributes);
    CommunicatorV2 *com = m_communicator.data();
    if (com)  com->sendMessage("NOTIFICATIO_DATA",wrapInGfdiEnvelope(MessageId::NotificationData,dataPayload));
// Test code ends - need to implement the real data in Notifcationhandler.
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
    qDebug() << Q_FUNC_INFO << "Garmin: checking notification queue for id " << note.id;
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

