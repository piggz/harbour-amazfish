#ifndef VOICECALLHANDLER_H
#define VOICECALLHANDLER_H

#include <QObject>
#include <QDateTime>
#include <QDBusPendingCallWatcher>
#include <QLoggingCategory>

class VoiceCallHandler : public QObject
{
    Q_OBJECT
    QLoggingCategory l;

    Q_ENUMS(VoiceCallStatus)

    Q_PROPERTY(QString handlerId READ handlerId CONSTANT)
    Q_PROPERTY(QString providerId READ providerId CONSTANT)
    Q_PROPERTY(int status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
    Q_PROPERTY(QString lineId READ lineId NOTIFY lineIdChanged)
    Q_PROPERTY(QDateTime startedAt READ startedAt NOTIFY startedAtChanged)
    Q_PROPERTY(int duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(bool isIncoming READ isIncoming CONSTANT)
    Q_PROPERTY(bool isEmergency READ isEmergency NOTIFY emergencyChanged)
    Q_PROPERTY(bool isMultiparty READ isMultiparty NOTIFY multipartyChanged)
    Q_PROPERTY(bool isForwarded READ isForwarded NOTIFY forwardedChanged)
    Q_PROPERTY(bool isRemoteHeld READ isRemoteHeld NOTIFY remoteHeldChanged)

public:
    enum VoiceCallStatus {
        STATUS_NULL,
        STATUS_ACTIVE,
        STATUS_HELD,
        STATUS_DIALING,
        STATUS_ALERTING,
        STATUS_INCOMING,
        STATUS_WAITING,
        STATUS_DISCONNECTED
    };
    enum HandlerProgressState {
        StateInit,
        StateRinging,
        StateAnswered,
        StateHangedUp,
        StateCleanedUp
    };

    explicit VoiceCallHandler(const QString &handlerId, QObject *parent = 0);
            ~VoiceCallHandler();

    QString handlerId() const;
    QString providerId() const;
    int status() const;
    QString statusText() const;
    QString lineId() const;
    QDateTime startedAt() const;
    int duration() const;
    bool isIncoming() const;
    bool isMultiparty() const;
    bool isEmergency() const;
    bool isForwarded() const;
    bool isRemoteHeld() const;

    HandlerProgressState getState() const { return m_state;}
    void setState(HandlerProgressState state) { m_state = state;}

Q_SIGNALS:
    void error(const QString &error);
    void statusChanged();
    void lineIdChanged();
    void durationChanged();
    void startedAtChanged();
    void emergencyChanged();
    void multipartyChanged();
    void forwardedChanged();
    void remoteHeldChanged();

public Q_SLOTS:
    void answer();
    void hangup();
    void hold(bool on);
    void deflect(const QString &target);
    void sendDtmf(const QString &tones);

protected Q_SLOTS:
    void initialize(bool notifyError = false);
    bool getProperties();

    void onPendingCallFinished(QDBusPendingCallWatcher *watcher);
    void onDurationChanged(int duration);
    void onStatusChanged(int status, QString statusText);
    void onLineIdChanged(QString lineId);
    void onStartedAtChanged(const QDateTime &startedAt);
    void onEmergencyChanged(bool isEmergency);
    void onMultipartyChanged(bool isMultiparty);
    void onForwardedChanged(bool isForwarded);
    void onRemoteHeldChanged(bool isRemoteHeld);

private:
    class VoiceCallHandlerPrivate *d_ptr;
    HandlerProgressState m_state = StateInit;

    Q_DISABLE_COPY(VoiceCallHandler)
    Q_DECLARE_PRIVATE(VoiceCallHandler)
};

#endif // VOICECALLHANDLER_H
