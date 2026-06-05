#ifndef GARMIN_DEVICE__H
#define GARMIN_DEVICE__H

#include <QObject>
#include <QSharedPointer>
#include "abstractdevice.h"
#include "services/garmin/communicator_v2.h"
#include "services/garmin/garminnotification.h"

class NotificationSpec;
class GarminNotificationHandler;

class GarminDevice : public AbstractDevice
{
    Q_OBJECT
public:
    explicit GarminDevice(const QString &pairedName, QObject *parent = 0);

    Amazfish::Features supportedFeatures() const override;
    Amazfish::DataTypes supportedDataTypes() const override;
    QString deviceType() const override;
    void sendAlert(const Amazfish::WatchNotification &notification) override;
    void incomingCall(const QString &caller) override;
    void incomingCallEnded() override;

    AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes, const QString &path) override;
    Q_SLOT void authenticated(bool ready);


    QString information(Amazfish::Info i) const override;


public slots:

    //void informationChanged(Amazfish::Info infoKey, const QString& infoValue);

//protected slots:
    void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);

signals:
    void sendAlertToDevice(NotificationSpec &note);

private:
    int mSteps = 0;


    void parseServices();
    void initialise();
    virtual void pair() override;
    QSharedPointer<GarminNotificationHandler> mNotificationHandler;


//    Q_SLOT void authenticated(bool ready);

   virtual void refreshInformation() override;

};

#endif // GARMIN_DEVICE__H
