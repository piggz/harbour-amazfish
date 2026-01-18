#ifndef ABSTRACTDEVICE_H
#define ABSTRACTDEVICE_H

#include "amazfish.h"
#include "abstractactivitydetailparser.h"
#include "abstractactivitysummaryparser.h"
#include "qble/qbledevice.h"
#include "weather/currentweather.h"
#include "abstractfirmwareinfo.h"

#include <KDb3/KDbDriver>
#include <KDb3/KDbConnection>
#include <KDb3/KDbConnectionData>
#include <KDb3/KDbTransactionGuard>

class AbstractDevice : public QBLEDevice
{
    Q_OBJECT

public:

    enum Event {
        EVENT_MUSIC_STOP,
        EVENT_MUSIC_PLAY,
        EVENT_MUSIC_PAUSE,
        EVENT_MUSIC_NEXT,
        EVENT_MUSIC_PREV,
        EVENT_MUSIC_VOLUP,
        EVENT_MUSIC_VOLDOWN,
        EVENT_APP_MUSIC,
        EVENT_DECLINE_CALL,
        EVENT_ANSWER_CALL,
        EVENT_IGNORE_CALL,
        EVENT_FIND_PHONE,
        EVENT_CANCEL_FIND_PHONE
    };
    Q_ENUM(Event)

    explicit AbstractDevice(const QString &pairedName, QObject *parent = nullptr);
    
    virtual void pair() override;

    virtual void connectToDevice() override;
    virtual void disconnectFromDevice() override;
    virtual QString connectionState() const;

    bool supportsFeature(Amazfish::Feature f) const;
    virtual Amazfish::Features supportedFeatures() const = 0;

    virtual Amazfish::DataTypes supportedDataTypes() const = 0;
    bool supportsDataType(Amazfish::DataType t) const;

    virtual void setDatabase(KDbConnection *conn);
    virtual KDbConnection* database();

    virtual QString deviceType() const = 0;
    QString deviceName() const;
    virtual void abortOperations();

    //Firmware handling
    virtual AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes, const QString &fileName) = 0; //Caller owns the pointer and should delete it

    virtual void prepareFirmwareDownload(const AbstractFirmwareInfo* info);
    virtual void startDownload();

    virtual void downloadSportsData();
    virtual void downloadActivityData();
    virtual void fetchLogs();
    virtual void sendWeather(CurrentWeather *weather);
    virtual void refreshInformation();
    virtual QString information(Amazfish::Info i) const;
    virtual void applyDeviceSetting(Amazfish::Settings s);
    virtual void rebootWatch();
    virtual void sendAlert(const Amazfish::WatchNotification &notification) = 0;
    virtual void incomingCall(const QString &caller) = 0;
    virtual void incomingCallEnded() = 0;
    virtual void sendEventReminder(int id, const QDateTime &dt, const QString &event);
    virtual void enableFeature(Amazfish::Feature feature);
    virtual void setMusicStatus(bool playing, const QString &title, const QString &artist, const QString &album, int duration = 0, int position = 0);
    virtual void navigationRunning(bool running);
    virtual void navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress);
    virtual void requestScreenshot();
    virtual QStringList supportedDisplayItems() const;
    virtual void immediateAlert(int level);
    virtual void requestManualHeartrate() const;
    virtual AbstractActivitySummaryParser* activitySummaryParser() const;
    virtual AbstractActivityDetailParser* activityDetailParser() const;

    Q_SIGNAL void operationRunningChanged();

    virtual bool operationRunning();

    //signals    
    Q_SIGNAL void message(const QString &text);
    Q_SIGNAL void downloadProgress(int percent);
    Q_SIGNAL void buttonPressed(int presses);
    Q_SIGNAL void connectionStateChanged();
    Q_SIGNAL void informationChanged(Amazfish::Info key, const QString& val);
    Q_SIGNAL void deviceEvent(Event event);

protected:
    bool m_needsAuth = false;
    bool m_pairing = false;
    bool m_ready = false;
    bool m_hasInitialised = false;
    bool m_autoreconnect = true;
    QString m_connectionState;
    QTimer *m_reconnectTimer;

    void setConnectionState(const QString &state);
    KDbConnection *m_conn = nullptr;

    Q_SLOT void deviceError(const QString &message);

private:
    void reconnectionTimer();
    void devicePairFinished(const QString& status);
    QString m_pairedName;

};

#endif
