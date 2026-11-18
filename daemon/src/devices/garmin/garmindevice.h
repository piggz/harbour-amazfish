#ifndef GARMIN_DEVICE__H
#define GARMIN_DEVICE__H

#include <QObject>
#include <QSharedPointer>
#include "abstractdevice.h"
#include "services/garmin/communicator_v2.h"
#include "services/garmin/garminnotificationhandler.h"
#include "services/garmin/protobufhandler.h"


class NotificationSpec;
class GarminNotificationHandler;
class ProtobufHandler;

class GarminDevice : public AbstractDevice
{
    Q_OBJECT
public:
    explicit GarminDevice(const QString &pairedName, QObject *parent = 0);
    QBLEService *drv_createService(const QString &uuid, const QString &path) override;

    Amazfish::Features supportedFeatures() const override;
    Amazfish::DataTypes supportedDataTypes() const override;
    QString deviceType() const override;
    void setPaired() {m_pairing = false;};
    bool isPairing() {return m_pairing;};
    void sendAlert(const Amazfish::WatchNotification &notification) override;
    void sendWeather(CurrentWeather* weather) override;
    void incomingCall(const QString &caller) override;
    void incomingCallEnded() override;
    void setMusicStatus(bool playing, const QString& title, const QString& artist, const QString& album, int duration = 0, int position = 0) override;

    AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes, const QString &path) override;
    Q_SLOT void authenticated(bool ready);

    QString information(Amazfish::Info i) const override;

public slots:

    void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);
    void onAnswerCallEvent();
    void onRejectCallEvent();
    void onFindPhoneRequested();
    void onFindPhoneCancelled();
    void onMusicControlCommand(quint8 command);

signals:
    void sendAlertToDevice(NotificationSpec &note);

private:
    int mSteps = 0;

    void initialise();
    void onWeatherRequestReceived();
    void sendWeatherToWatch();
    virtual void refreshInformation() override;

    QString mPreferredService;

    // Owned by DeviceInterface, so it outlives the device. The watch asks for
    // weather, it never receives a push, so the data is only kept to answer
    // that request with.
    CurrentWeather* mWeather = nullptr;
    // The watch asked before the phone had weather. sendWeather() answers then.
    bool mWeatherRequested = false;
    // Set once a connection asks the daemon for a weather refresh, so the watch
    // asking again every few seconds does not fetch again and again.
    bool mWeatherRefreshRequested = false;
    QSharedPointer<GarminNotificationHandler> mNotificationHandler;
    QSharedPointer<ProtobufHandler> mProtobufHandler;
    QSharedPointer<CommunicatorV2> mCommunicator;

};

#endif // GARMIN_DEVICE__H
