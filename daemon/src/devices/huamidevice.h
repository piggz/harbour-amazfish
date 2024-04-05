#ifndef HUAMIDEVICE_H
#define HUAMIDEVICE_H

#include "abstractdevice.h"
#include "qble/qbleservice.h"

#include "deviceinfoservice.h"
#include "mibandservice.h"
#include "miband2service.h"
#include "alertnotificationservice.h"
#include "hrmservice.h"
#include "bipfirmwareservice.h"

#include <QTimer>

class HuamiDevice : public AbstractDevice
{
    Q_OBJECT
public:
    explicit HuamiDevice(const QString &pairedName, QObject *parent = nullptr);
    
    void abortOperations() override;

    void refreshInformation() override;
    QString information(Info i) const override;
    void rebootWatch() override;

    void applyDeviceSetting(Settings s) override;

    virtual void sendAlert(const QString &sender, const QString &subject, const QString &message) override;
    void incomingCall(const QString &caller) override;

    QString softwareRevision();
    
    void downloadSportsData() override;
    void downloadActivityData() override;
    void fetchLogs() override;

    void prepareFirmwareDownload(const AbstractFirmwareInfo *info) override;
    void startDownload() override;

    Q_SLOT void authenticated(bool ready);

    void sendWeatherHuami(CurrentWeather *weather, bool sendConditionString);

    int activitySampleSize();
    void navigationRunning(bool running) override;
    void navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress) override;

    void setDatabase(KDbConnection *conn) override;

protected:
    Q_SLOT void handleButtonPressed();
    Q_SLOT void buttonPressTimeout();
    Q_SLOT void stepsChanged();
    Q_SLOT void batteryInfoChanged();
    Q_SLOT void serviceEvent(char event);

    virtual void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);
    virtual void initialise() = 0;

    int m_ActivitySampleSize = 4;

private:
    QString m_softwareRevision;
    int m_buttonPresses = 0;
    QTimer *m_keyPressTimer = nullptr;
};

#endif // BIPDEVICE_H
