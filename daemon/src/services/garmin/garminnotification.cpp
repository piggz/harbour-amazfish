#include "garminnotification.h"
#include "garmintypes.h"

#include <optional>


const uint MAX_PROTOBUF_CHUNK_SIZE = 3072;

void GarminNotificationHandler::cleanupOldNotifications()
{
    QMutexLocker lock(&m_mutex);

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

    {
        QMutexLocker lock(&m_mutex);
        if (m_storedNotifications.contains(id)) {
            type = m_storedNotifications[id].notificationType;
            m_storedNotifications.remove(id);
        }
    }

    auto msg = NotificationUpdateMessageBuilder::create(
        NotificationUpdateType::Remove,
        type,
        0,
        id).build();

    QByteArray gfdi = wrapInGfdiEnvelope(5033, msg);

    // async send simulated
    return m_communicator->sendMessage("notification_remove", gfdi);
}

void GarminNotificationHandler::onNotification(NotificationSpec notification)
{

    // Send generic notification (SMS, email, etc)
    qDebug() << Q_FUNC_INFO << notification.sourceName.value() << notification.title.value();
    if (!m_isConnected) {
        QMutexLocker lock(&m_mutex);
        m_missedNotifications.enqueue(notification);
        qDebug() << Q_FUNC_INFO << "Garmin: Watch not connected, can't send";
        return;
    }

    cleanupOldNotifications();


    QString typeStr;
    switch (notification.notificationType) {
    case NotificationType::GenericPhone:       typeStr = "Phone"; break;
    case NotificationType::GenericSms:         typeStr = "SMS"; break;
    case NotificationType::GenericEmail:       typeStr = "Email"; break;
    case NotificationType::GenericChat:        typeStr = "Chat"; break;
    case NotificationType::GenericSocial:      typeStr = "Social"; break;
    case NotificationType::GenericNavigation:  typeStr = "Navigation"; break;
    case NotificationType::GenericCalendar:    typeStr = "Calendar"; break;
    case NotificationType::GenericAlarmClock:  typeStr = "Alarm"; break;
    case NotificationType::Generic:            typeStr = "Generic"; break;
    default: typeStr = "Generic";
    }


    // ---------------------------------------------------------
    // Calculate unretrieved count + IDs
    // ---------------------------------------------------------
    quint8 count = 0;
    QVector<QPair<qint32, NotificationType>> unretrieved;

    {
        QMutexLocker lock(&m_mutex);

        for (auto it = m_storedNotifications.begin();
             it != m_storedNotifications.end(); ++it)
        {
            const auto& spec = it.value();

            if (spec.notificationType == notification.notificationType &&
                !spec.retrieved)
            {
                unretrieved.append({it.key(), spec.notificationType});
            }
        }

        quint8 unretrievedCount = static_cast<quint8>(unretrieved.size());

        // +1 for new notification, capped at 50
        count = qMin<quint8>(unretrievedCount + 1, 50);
    }

    // ---------------------------------------------------------
    // High count cleanup logic (>=5)
    // ---------------------------------------------------------
    if (count >= 5) {

        qWarning() << "🧹 Count too high (" << count
                   << "), removing OLD unretrieved notifications (>60s)";

        QElapsedTimer now;
        now.start();

        int removedCount = 0;
        int totalUnretrieved = unretrieved.size();

        for (const auto& pair : unretrieved) {
            qint32 id = pair.first;

            bool isOld = false;

            {
                QMutexLocker lock(&m_mutex);
                if (m_storedNotifications.contains(id)) {
                    const auto& spec = m_storedNotifications[id];
                    isOld = (spec.timestamp.elapsed() > 60000); // 60 sec
                }
            }

            if (isOld) {
                bool res = removeNotification(id);
                if (!res) {
                    qWarning() << "Failed to remove notification"
                               << id;
                } else {
                    removedCount++;
                }
            } else {
                qInfo() << "Keeping notification" << id
                        << "(too recent)";
            }
        }

        if (removedCount > 0) {
            qDebug()  << Q_FUNC_INFO << "Garmin: Removed" << removedCount
                    << "old notifications,"
                    << (totalUnretrieved - removedCount) << "kept";
        } else {
            qDebug() <<Q_FUNC_INFO << "Garmin: No old notifications to remove";
        }

        count = count-removedCount;
    }
    auto updateMsg =
        NotificationUpdateMessageBuilder::create(
            NotificationUpdateType::Add,
            notification.notificationType,
            count,
            notification.id)
            .withActions(notification.hasActions)
            .build();

    QByteArray gfdi = wrapInGfdiEnvelope(5033, updateMsg);

    bool result = m_communicator->sendMessage("notification", gfdi);

    if (result) {
        qDebug() << Q_FUNC_INFO << "Garmin: Notification sent successfully";
    } else {
        qDebug() << Q_FUNC_INFO <<"Garmin: Notification could not be sent";

        return; //result
    }

    // Store notification
    {
        QMutexLocker lock(&m_mutex);
        m_storedNotifications.insert(notification.id, notification);
    }
    return;
     qDebug() << Q_FUNC_INFO << "Garmin: Count is " << count;
}







void GarminNotificationHandler::replayMissedNotifications()
{
    QList<NotificationSpec> list;

    {
        QMutexLocker lock(&m_mutex);
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
    QMutexLocker lock(&m_mutex);
    m_isConnected = v;
}

void GarminNotificationHandler::onSetCallState(const CallSpec& call)
{
    const qint32 id = call.getId();

    if (call.command == CallCommand::Incoming) {

        {
            QMutexLocker lock(&m_mutex);
            m_activeNotifications.insert(id, call);
        }

        NotificationSpec notif =
            NotificationSpec::create(id, NotificationType::GenericPhone)
            .withTitle(call.name.value_or("Incoming Call"))
            .withBody(QString("From: %1").arg(call.number))
            .withSender(call.sourceName.value_or("Phone"));

        {
            QMutexLocker lock(&m_mutex);
            m_storedNotifications.insert(id, notif);
        }

        auto msg = NotificationUpdateMessageBuilder::create(
            NotificationUpdateType::Add,
            NotificationType::GenericPhone,
            1,
            id)
            .withActions(true)
            .build();

        QByteArray gfdi = wrapInGfdiEnvelope(5033, msg);
        m_communicator->sendMessage("notification", gfdi);
    }
    else if (call.command == CallCommand::End ||
             call.command == CallCommand::Reject)
    {
        QMutexLocker lock(&m_mutex);
        m_activeNotifications.remove(id);
        m_storedNotifications.remove(id);

        auto msg = NotificationUpdateMessageBuilder::create(
            NotificationUpdateType::Remove,
            NotificationType::GenericPhone,
            0,
            id).build();

        QByteArray gfdi = wrapInGfdiEnvelope(5033, msg);
        m_communicator->sendMessage("call_end_notification", gfdi);
    }
}

QByteArray GarminNotificationHandler::wrapInGfdiEnvelope(quint16 messageId, const QByteArray& payload)
{
    QByteArray msg;

    quint16 size = static_cast<quint16>(2 + 2 + payload.size() + 2);

    msg.append(char(size & 0xFF));
    msg.append(char((size >> 8) & 0xFF));

    msg.append(char(messageId & 0xFF));
    msg.append(char((messageId >> 8) & 0xFF));

    msg.append(payload);

    quint16 crc = computeCrc16(msg);

    msg.append(char(crc & 0xFF));
    msg.append(char((crc >> 8) & 0xFF));

    return msg;
}



