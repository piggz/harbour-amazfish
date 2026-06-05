//#ifndef _GARMIN_NOTIFICATION__H
//#define _GARMIN_NOTIFICATION__H

#pragma once

#include <QtCore/QString>
#include <optional>
#include <QDateTime>
#include <QElapsedTimer>
#include <QObject>

#include "communicator_v2.h"
#include "garmintypes.h"

class CommunicatorV2;

//
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

//
// -------------------------
// NotificationType
// -------------------------
enum class NotificationType : quint8 {
    GenericPhone        = 1,
    GenericSms          = 2,
    GenericEmail        = 3,
    GenericChat         = 4,
    GenericSocial       = 5,
    GenericNavigation   = 6,
    GenericCalendar     = 7,
    GenericAlarmClock   = 8,
    Generic             = 9
};



// Helper functions (Rust impl NotificationType)
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
// ---------------------------------------------------------------------
// NotificationUpdateType
// ---------------------------------------------------------------------
enum class NotificationUpdateType : quint8 {
    Add    = 0,
    Modify = 1,
    Remove = 2
};

//
// ---------------------------------------------------------------------
// NotificationUpdateMessageBuilder (data holder)
// ---------------------------------------------------------------------
class NotificationUpdateMessageBuilder
{
public:

    static NotificationUpdateMessageBuilder create(
        NotificationUpdateType updateType,
        NotificationType notificationType,
        quint8 count,
        qint32 notificationId)
    {
        NotificationUpdateMessageBuilder b;
        b.updateType = updateType;
        b.notificationType = notificationType;
        b.count = count;
        b.notificationId = notificationId;
        b.hasActions = false;
        b.hasPicture = false;
        return b;
    }

    // -----------------------------------------------------------------
    // Builder method (Rust: with_actions)
    // -----------------------------------------------------------------
    NotificationUpdateMessageBuilder withActions(bool value) const
    {
        NotificationUpdateMessageBuilder copy = *this;
        copy.hasActions = value;
        return copy;
    }

    // -----------------------------------------------------------------
    // build() -> QByteArray
    // -----------------------------------------------------------------
    QByteArray build() const
    {
        QByteArray message;
        // message.reserve(9); // exact Rust capacity

        // NOTE: Payload only, envelope added later (unchanged)

        // 1. Update type
        message.append(static_cast<char>(updateType));

        // 2. Category flags
        quint8 categoryFlags =
            NotificationTypeUtil::notificationFlags(notificationType, hasActions);
        message.append(static_cast<char>(categoryFlags));

        // 3. Category value
        quint8 categoryValue =
            NotificationTypeUtil::categoryValue(notificationType);
        message.append(static_cast<char>(categoryValue));

        // 4. Count
        message.append(static_cast<char>(count));

        // 5. Notification ID (LE i32)
        writeU32le(message,notificationId);


        // 6. Phone flags
        quint8 phoneFlags = 0;

        if (hasActions) {
            phoneFlags |= 0x02; // NEW_ACTIONS
        }
        if (hasPicture) {
            phoneFlags |= 0x04; // HAS_ATTACHMENTS
        }

        message.append(static_cast<char>(phoneFlags));

        return message;
    }

    NotificationUpdateType updateType;
    NotificationType notificationType;
    quint8 count = 0;
    qint32 notificationId = 0;
    bool hasActions = false;
    bool hasPicture = false;
};

//
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

//#endif //_GARMIN_NOTIFICATION__H
