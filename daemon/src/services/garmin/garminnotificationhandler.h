//#ifndef _GARMINNOTIFICATIONHANDLER_H
//#define _GARMINNOTIFICATIONHANDLER_H

#pragma once

#include <QtCore/QString>
#include <optional>
#include <QDateTime>
#include <QElapsedTimer>
#include <QObject>

#include "communicator_v2.h"
#include"garminnotificationupdatemessage.h"
#include "garmintypes.h"

class CommunicatorV2;

// -------------------------
// CallCommand
// -------------------------
class Action  {
public:
    static constexpr int TYPE_UNDEFINED = -1;
    static constexpr int TYPE_WEARABLE_SIMPLE = 0;
    static constexpr int TYPE_WEARABLE_REPLY = 1;
    static constexpr int TYPE_SYNTECTIC_REPLY_PHONENR = 2;
    static constexpr int TYPE_SYNTECTIC_DISMISS = 3;
    static constexpr int TYPE_SYNTECTIC_DISMISS_ALL = 4;
    static constexpr int TYPE_SYNTECTIC_MUTE = 5;
    static constexpr int TYPE_SYNTECTIC_OPEN = 6;
    static constexpr int TYPE_CUSTOM_SIMPLE = 7;
    static constexpr int TYPE_CUSTOM_REPLY = 8;

    int type = TYPE_UNDEFINED;
    long handle = 0;
    QString title;

    bool isReply() const {
        return type == TYPE_WEARABLE_REPLY || type == TYPE_SYNTECTIC_REPLY_PHONENR || type == TYPE_CUSTOM_REPLY;
    }
};



// ---------------------------------------------------------------------
// NotificationSpec
// ---------------------------------------------------------------------
class NotificationSpec
{
public:
    qint32 id;
    NotificationType notificationType;

    std::optional<QString> title;
    std::optional<QString> body;
    std::optional<QString> sender;
    std::optional<QString> phoneNumber;
    std::optional<QString> sourceName;

    quint64 when;                 // UNIX timestamp (seconds)
    bool hasActions;
    bool hasPicture;

    QElapsedTimer timestamp;
    bool retrieved;

    QVector<QSharedPointer<Action>> attachedActions;

    static NotificationSpec create(qint32 id, NotificationType type)
    {
        NotificationSpec s;
        s.id = id;
        s.notificationType = type;

        // SystemTime::now() -> UNIX timestamp
        s.when = static_cast<quint64>(
            QDateTime::currentMSecsSinceEpoch()*1000
        );

        s.hasActions = true;      // default enabled
        s.hasPicture = false;
        s.retrieved = false;

        s.timestamp.start();
        return s;
    }

    // Builder methods (immutability preserved)
    NotificationSpec withTitle(const QString& v) const
    {
        NotificationSpec copy = *this;
        copy.title = v;
        return copy;
    }

    NotificationSpec withBody(const QString& v) const
    {
        NotificationSpec copy = *this;
        copy.body = v;
        return copy;
    }

    NotificationSpec withSender(const QString& v) const
    {
        NotificationSpec copy = *this;
        copy.sender = v;
        return copy;
    }

    NotificationSpec withPhoneNumber(const QString& v) const
    {
        NotificationSpec copy = *this;
        copy.phoneNumber = v;
        return copy;
    }

    NotificationSpec withSourceName(const QString& v) const
    {
        NotificationSpec copy = *this;
        copy.sourceName = v;
        return copy;
    }

    NotificationSpec withActions(bool v) const
    {
        NotificationSpec copy = *this;
        copy.hasActions = v;
        return copy;
    }

    NotificationSpec withPicture(bool v) const
    {
        NotificationSpec copy = *this;
        copy.hasPicture = v;
        return copy;
    }

};

struct NotificationTypeUtil
{
    static quint8 categoryValue(NotificationType t)
    {
        switch (t) {
        case NotificationType::GenericPhone:       return 1;  // INCOMING_CALL
        case NotificationType::GenericEmail:       return 6;  // EMAIL
        case NotificationType::GenericSms:         return 12; // SMS
        case NotificationType::GenericChat:        return 12; // SMS
        case NotificationType::GenericNavigation:  return 10; // LOCATION
        case NotificationType::GenericSocial:      return 4;  // SOCIAL
        case NotificationType::GenericCalendar:    return 5;  // SCHEDULE
        case NotificationType::GenericAlarmClock:  return 0;  // OTHER
        case NotificationType::Generic:            return 0;  // OTHER
        }
        return 0;
    }

    static quint8 notificationFlags(NotificationType /*t*/, bool /*hasActions*/)
    {
        quint8 flags = 0;

        // Bit 1: FOREGROUND
        flags |= 0x02;

        // Bit 4: ACTION_DECLINE (always set)
        flags |= 0x10;

        return flags;
    }
};


//



//
// -------------------------
// CallCommand
// -------------------------
enum class CallCommand : quint8 {
    Undefined = 0,
    Accept    = 1,
    Incoming  = 2,
    Outgoing  = 3,
    Reject    = 4,
    Start     = 5,
    End       = 6
};

//
// -------------------------
// CallSpec
// -------------------------
class CallSpec
{
public:
    QString number;
    std::optional<QString> name;
    std::optional<QString> sourceName;
    std::optional<QString> sourceAppId;
    CallCommand command;

    // Constructor (Rust: new)
    static CallSpec create(const QString& number, CallCommand command)
    {
        CallSpec s;
        s.number = number;
        s.command = command;
        return s;
    }

    // Builder-style method (Rust: with_name)
    CallSpec withName(const QString& newName) const
    {
        CallSpec copy = *this;
        copy.name = newName;
        return copy;
    }

    // Equivalent to Rust get_id()
    qint32 getId() const
    {
        qint32 acc = 0;

        const QByteArray bytes = number.toUtf8();
        for (quint8 b : bytes) {
            acc = static_cast<qint32>(
                (acc * 31) + static_cast<qint32>(b)
            ); // wrapping behaviour preserved implicitly (32-bit overflow)
        }

        return acc;
    }
};



class GarminNotificationHandler : public QObject
{
    Q_OBJECT

public:
    static QSharedPointer<GarminNotificationHandler> create(const QSharedPointer<CommunicatorV2> communicator)
    {
        return QSharedPointer<GarminNotificationHandler>(new GarminNotificationHandler(communicator));
    }

    GarminNotificationHandler(QSharedPointer<CommunicatorV2> communicator)
        : m_communicator(communicator)
    {
        m_nextNotificationId = 100;
        m_lastCleanup.start();
        m_isConnected = true;
    }

    // -----------------------------------------------------------------
    // get_next_notification_id
    // -----------------------------------------------------------------
    qint32 getNextNotificationId()
    {
        QMutexLocker lock(&m_mutex);
        return m_nextNotificationId++;
    }

    void cleanupOldNotifications();
    bool removeNotification(qint32 id);
    void replayMissedNotifications();
    void setConnected(bool v);
    void onSetCallState(const CallSpec& call);
    QByteArray wrapInGfdiEnvelope(quint16 messageId, const QByteArray& payload);

private:

    bool addNotificationToQueue(NotificationSpec note);
    int getNotificationCount(NotificationType type);

   QSharedPointer<CommunicatorV2> m_communicator;

   QMutex m_mutex;

   //QHash<qint32, CallSpec> m_activeNotifications;
   //QHash<NotificationType, quint8> m_notificationCounts;
   QHash<int,long> mNotificationReplyAction;
   QHash<qint32, NotificationSpec> m_storedNotifications;

   std::optional<QPair<qint32, QByteArray>> m_lastControlRequest;

   QHash<qint32, quint8> m_messageSizeRequestCount;

   qint32 m_nextNotificationId;

   QElapsedTimer m_lastCleanup;

   //QSet<QPair<QString, quint64>> m_seenDbusMessages;


   QQueue<NotificationSpec> m_missedNotifications;

   bool m_isConnected;


public slots:
    void onNotification(NotificationSpec notification);

};

//#endif //_GARMINNOTIFICATIONHANDLER_H
