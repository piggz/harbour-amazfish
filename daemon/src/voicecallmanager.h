#ifndef VOICECALLMANAGER_H
#define VOICECALLMANAGER_H

#include "voicecallhandler.h"

#include <QObject>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QLoggingCategory>
#include <QtContacts/QContactManager>
#include <QtContacts/QContactDetailFilter>

using namespace QtContacts;

class VoiceCallProviderData
{
public:
    VoiceCallProviderData() {/*..*/}
    VoiceCallProviderData(const QString &pId, const QString &pType, const QString &pLabel)
        : id(pId), type(pType), label(pLabel) {/*...*/}

    QString id;
    QString type;
    QString label;
};

typedef QHash<QString,VoiceCallProviderData> VoiceCallProviderHash;

typedef QList<VoiceCallHandler*> VoiceCallHandlerList;

class VoiceCallManager : public QObject
{
    Q_OBJECT
    QLoggingCategory l;

    Q_PROPERTY(QDBusInterface* interface READ interface)

    Q_PROPERTY(VoiceCallHandlerList voiceCalls READ voiceCalls NOTIFY voiceCallsChanged)
    Q_PROPERTY(VoiceCallProviderHash providers READ providers NOTIFY providersChanged)

    Q_PROPERTY(QString defaultProviderId READ defaultProviderId NOTIFY defaultProviderChanged)

    Q_PROPERTY(VoiceCallHandler* activeVoiceCall READ activeVoiceCall NOTIFY activeVoiceCallChanged)

    Q_PROPERTY(QString audioMode READ audioMode WRITE setAudioMode NOTIFY audioModeChanged)
    Q_PROPERTY(bool isAudioRouted READ isAudioRouted WRITE setAudioRouted NOTIFY audioRoutedChanged)
    Q_PROPERTY(bool isMicrophoneMuted READ isMicrophoneMuted WRITE setMuteMicrophone NOTIFY microphoneMutedChanged)
    Q_PROPERTY(bool isSpeakerMuted READ isSpeakerMuted WRITE setMuteSpeaker NOTIFY speakerMutedChanged)

public:
    explicit VoiceCallManager(QObject *parent = 0);
            ~VoiceCallManager();

    QDBusInterface* interface() const;

    VoiceCallHandlerList voiceCalls() const;
    VoiceCallProviderHash providers() const;

    QString defaultProviderId() const;

    VoiceCallHandler* activeVoiceCall() const;
    QString findPersonByNumber(QString number);
    void hangUp(uint cookie);

    QString audioMode() const;
    bool isAudioRouted() const;

    bool isMicrophoneMuted() const;
    bool isSpeakerMuted() const;

Q_SIGNALS:
    void error(const QString &message);

    void providersChanged();
    void voiceCallsChanged();

    void defaultProviderChanged();

    void activeVoiceCallChanged();

    void audioModeChanged();
    void audioRoutedChanged();
    void microphoneMutedChanged();
    void speakerMutedChanged();

public Q_SLOTS:
    void dial(const QString &providerId, const QString &msisdn);
    void hangupAll();

    void silenceRingtone();

    bool setAudioMode(const QString &mode);
    bool setAudioRouted(bool on);
    bool setMuteMicrophone(bool on = true);
    bool setMuteSpeaker(bool on = true);

protected Q_SLOTS:
    void initialize(bool notifyError = false);

    void onProvidersChanged();
    void onVoiceCallsChanged();
    void onActiveVoiceCallChanged();
    void onVoiceError(const QString &message);
    void onPendingCallFinished(QDBusPendingCallWatcher *watcher);
    void onPendingSilenceFinished(QDBusPendingCallWatcher *watcher);

private:
    class VoiceCallManagerPrivate *d_ptr;
    QContactManager *contacts;
    QContactDetailFilter numberFilter;

    Q_DISABLE_COPY(VoiceCallManager)
    Q_DECLARE_PRIVATE(VoiceCallManager)
};

#endif // VOICECALLMANAGER_H
