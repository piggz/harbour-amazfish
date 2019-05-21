#include "voicecallhandler.h"

#include <QDebug>
#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QVariantMap>

/*!
  \class VoiceCallHandler
  \brief This is the D-Bus proxy for communicating with the voice call manager
    from a declarative context, this interface specifically interfaces with
    the managers' voice call handler instances.
*/
class VoiceCallHandlerPrivate
{
    Q_DECLARE_PUBLIC(VoiceCallHandler)

public:
    VoiceCallHandlerPrivate(VoiceCallHandler *q, const QString &pHandlerId)
        : q_ptr(q), handlerId(pHandlerId), interface(NULL)
        , duration(0), status(0), emergency(false), incoming(false)
        , multiparty(false) , forwarded(false), remoteHeld(false)
    { /* ... */ }

    VoiceCallHandler *q_ptr;

    QString handlerId;

    QDBusInterface *interface;

    int duration;
    int status;
    QString statusText;
    QString lineId;
    QString providerId;
    QDateTime startedAt;
    bool emergency;
    bool incoming;
    bool multiparty;
    bool forwarded;
    bool remoteHeld;
};

/*!
  Constructs a new proxy interface for the provided voice call handlerId.
*/
VoiceCallHandler::VoiceCallHandler(const QString &handlerId, QObject *parent)
    : QObject(parent), l(metaObject()->className()), d_ptr(new VoiceCallHandlerPrivate(this, handlerId))
{
    Q_D(VoiceCallHandler);
    qCDebug(l) << QString("Creating D-Bus interface to: ") + handlerId;
    d->interface = new QDBusInterface("org.nemomobile.voicecall",
                                      "/calls/" + handlerId,
                                      "org.nemomobile.voicecall.VoiceCall",
                                      QDBusConnection::sessionBus(),
                                      this);
    this->initialize(true);
}

VoiceCallHandler::~VoiceCallHandler()
{
    Q_D(VoiceCallHandler);
    delete d;
}

void VoiceCallHandler::initialize(bool notifyError)
{
    Q_D(VoiceCallHandler);

    if (d->interface->isValid()) {
        if (getProperties()) {
            emit durationChanged();
            emit statusChanged();
            emit lineIdChanged();
            emit startedAtChanged();
            emit multipartyChanged();
            emit emergencyChanged();
            emit forwardedChanged();

            connect(d->interface, SIGNAL(error(QString)), SIGNAL(error(QString)));
            connect(d->interface, SIGNAL(statusChanged(int, QString)), SLOT(onStatusChanged(int, QString)));
            connect(d->interface, SIGNAL(lineIdChanged(QString)), SLOT(onLineIdChanged(QString)));
            connect(d->interface, SIGNAL(durationChanged(int)), SLOT(onDurationChanged(int)));
            connect(d->interface, SIGNAL(startedAtChanged(QDateTime)), SLOT(onStartedAtChanged(QDateTime)));
            connect(d->interface, SIGNAL(emergencyChanged(bool)), SLOT(onEmergencyChanged(bool)));
            connect(d->interface, SIGNAL(multipartyChanged(bool)), SLOT(onMultipartyChanged(bool)));
            connect(d->interface, SIGNAL(forwardedChanged(bool)), SLOT(onForwardedChanged(bool)));
            connect(d->interface, SIGNAL(remoteHeldChanged(bool)), SLOT(onRemoteHeldChanged(bool)));
        }
        else {
            if (notifyError) emit this->error("Failed to get VoiceCall properties from VCM D-Bus service.");
        }
    }
    else {
        qCCritical(l) << d->interface->lastError().name() << d->interface->lastError().message();
    }
}

bool VoiceCallHandler::getProperties()
{
    Q_D(VoiceCallHandler);

    QDBusInterface props(d->interface->service(), d->interface->path(),
                         "org.freedesktop.DBus.Properties", d->interface->connection());

    QDBusReply<QVariantMap> reply = props.call("GetAll", d->interface->interface());
    if (reply.isValid()) {
        QVariantMap props = reply.value();
        qCDebug(l) << props;
        d->providerId = props["providerId"].toString();
        d->duration = props["duration"].toInt();
        d->status = props["status"].toInt();
        d->statusText = props["statusText"].toString();
        d->lineId = props["lineId"].toString();
        d->startedAt = QDateTime::fromMSecsSinceEpoch(props["startedAt"].toULongLong());
        d->multiparty = props["isMultiparty"].toBool();
        d->emergency = props["isEmergency"].toBool();
        d->forwarded = props["isForwarded"].toBool();
        d->incoming = props["isIncoming"].toBool();
        d->remoteHeld = props["isIncoming"].toBool();
        return true;
    }
    else {
        qCCritical(l) << "Failed to get VoiceCall properties from VCM D-Bus service.";
        return false;
    }
}

void VoiceCallHandler::onDurationChanged(int duration)
{
    Q_D(VoiceCallHandler);
    //qCDebug(l) <<"onDurationChanged"<<duration;
    d->duration = duration;
    emit durationChanged();
}

void VoiceCallHandler::onStatusChanged(int status, QString statusText)
{
    Q_D(VoiceCallHandler);
    qCDebug(l) <<"onStatusChanged" << status << statusText;
    d->status = status;
    d->statusText = statusText;
    if (status) getProperties(); // make sure all properties are present
    emit statusChanged();
}

void VoiceCallHandler::onLineIdChanged(QString lineId)
{
    Q_D(VoiceCallHandler);
    qCDebug(l) << "onLineIdChanged" << lineId;
    d->lineId = lineId;
    emit lineIdChanged();
}

void VoiceCallHandler::onStartedAtChanged(const QDateTime &startedAt)
{
    Q_D(VoiceCallHandler);
    qCDebug(l) << "onStartedAtChanged" << startedAt;
    d->startedAt = d->interface->property("startedAt").toDateTime();
    emit startedAtChanged();
}

void VoiceCallHandler::onEmergencyChanged(bool isEmergency)
{
    Q_D(VoiceCallHandler);
    qCDebug(l) << "onEmergencyChanged" << isEmergency;
    d->emergency = isEmergency;
    emit emergencyChanged();
}

void VoiceCallHandler::onMultipartyChanged(bool isMultiparty)
{
    Q_D(VoiceCallHandler);
    qCDebug(l) << "onMultipartyChanged" << isMultiparty;
    d->multiparty = isMultiparty;
    emit multipartyChanged();
}

void VoiceCallHandler::onForwardedChanged(bool isForwarded)
{
    Q_D(VoiceCallHandler);
    qCDebug(l) << "onForwardedChanged" << isForwarded;
    d->forwarded = isForwarded;
    emit forwardedChanged();
}

void VoiceCallHandler::onRemoteHeldChanged(bool isRemoteHeld)
{
    Q_D(VoiceCallHandler);
    qCDebug(l) << "onRemoteHeldChanged" << isRemoteHeld;
    d->forwarded = isRemoteHeld;
    emit remoteHeldChanged();
}

/*!
  Returns this voice calls' handler id.
 */
QString VoiceCallHandler::handlerId() const
{
    Q_D(const VoiceCallHandler);
    return d->handlerId;
}

/*!
  Returns this voice calls' provider id.
 */
QString VoiceCallHandler::providerId() const
{
    Q_D(const VoiceCallHandler);
    return d->providerId;
}

/*!
  Returns this voice calls' call status.
 */
int VoiceCallHandler::status() const
{
    Q_D(const VoiceCallHandler);
    return d->status;
}

/*!
  Returns this voice calls' call status as a symbolic string.
 */
QString VoiceCallHandler::statusText() const
{
    Q_D(const VoiceCallHandler);
    return d->statusText;
}

/*!
  Returns this voice calls' remote end-point line id.
 */
QString VoiceCallHandler::lineId() const
{
    Q_D(const VoiceCallHandler);
    return d->lineId;
}

/*!
  Returns this voice calls' started at property.
 */
QDateTime VoiceCallHandler::startedAt() const
{
    Q_D(const VoiceCallHandler);
    return d->startedAt;
}

/*!
  Returns this voice calls' duration property.
 */
int VoiceCallHandler::duration() const
{
    Q_D(const VoiceCallHandler);
    return d->duration;
}

/*!
  Returns this voice calls' incoming call flag property.
 */
bool VoiceCallHandler::isIncoming() const
{
    Q_D(const VoiceCallHandler);
    return d->incoming;
}

/*!
  Returns this voice calls' multiparty flag property.
 */
bool VoiceCallHandler::isMultiparty() const
{
    Q_D(const VoiceCallHandler);
    return d->multiparty;
}

/*!
  Returns this voice calls' forwarded flag property.
 */
bool VoiceCallHandler::isForwarded() const
{
    Q_D(const VoiceCallHandler);
    return d->forwarded;
}

/*!
  Returns this voice calls' emergency flag property.
 */
bool VoiceCallHandler::isEmergency() const
{
    Q_D(const VoiceCallHandler);
    return d->emergency;
}

/*!
  Returns this voice calls' remoteHeld flag property.
 */
bool VoiceCallHandler::isRemoteHeld() const
{
    Q_D(const VoiceCallHandler);
    return d->remoteHeld;
}

/*!
  Initiates answering this call, if the call is an incoming call.
 */
void VoiceCallHandler::answer()
{
    Q_D(VoiceCallHandler);
    QDBusPendingCall call = d->interface->asyncCall("answer");

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(onPendingCallFinished(QDBusPendingCallWatcher*)));
}

/*!
  Initiates droping the call, unless the call is disconnected.
 */
void VoiceCallHandler::hangup()
{
    Q_D(VoiceCallHandler);
    QDBusPendingCall call = d->interface->asyncCall("hangup");

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(onPendingCallFinished(QDBusPendingCallWatcher*)));
}

/*!
  Initiates holding the call, unless the call is disconnected.
 */
void VoiceCallHandler::hold(bool on)
{
    Q_D(VoiceCallHandler);
    QDBusPendingCall call = d->interface->asyncCall("hold", on);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(onPendingCallFinished(QDBusPendingCallWatcher*)));
}

/*!
  Initiates deflecting the call to the provided target phone number.
 */
void VoiceCallHandler::deflect(const QString &target)
{
    Q_D(VoiceCallHandler);
    QDBusPendingCall call = d->interface->asyncCall("deflect", target);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(onPendingCallFinished(QDBusPendingCallWatcher*)));
}

void VoiceCallHandler::sendDtmf(const QString &tones)
{
    Q_D(VoiceCallHandler);
    QDBusPendingCall call = d->interface->asyncCall("sendDtmf", tones);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(onPendingCallFinished(QDBusPendingCallWatcher*)));
}

void VoiceCallHandler::onPendingCallFinished(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<bool> reply = *watcher;

    if (reply.isError()) {
        qCCritical(l) << QString::fromLatin1("Received error reply for member: %1 (%2)").arg(reply.reply().member()).arg(reply.error().message());
        emit this->error(reply.error().message());
        watcher->deleteLater();
    } else {
        qCDebug(l) << QString::fromLatin1("Received successful reply for member: %1").arg(reply.reply().member());
    }
}
