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
    
    virtual void abortOperations() override;

    virtual void refreshInformation() override;
    virtual QString information(Info i) const override;
    virtual void rebootWatch() override;

    virtual void applyDeviceSetting(Settings s) override;

    virtual void sendAlert(const QString &sender, const QString &subject, const QString &message) override;
    virtual void incomingCall(const QString &caller) override;

    QString softwareRevision();
    
    virtual void prepareFirmwareDownload(const AbstractFirmwareInfo *info) override;
    virtual void startDownload() override;

    Q_SLOT void authenticated(bool ready);

    void sendWeatherHuami(CurrentWeather *weather, bool sendConditionString);

    virtual int activitySampleSize();
    virtual void navigationRunning(bool running) override;
    virtual void navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress) override;

protected:
    Q_SLOT void handleButtonPressed();
    Q_SLOT void buttonPressTimeout();
    Q_SLOT void stepsChanged();
    Q_SLOT void batteryInfoChanged();
    Q_SLOT void serviceEvent(char event);

    int m_ActivitySampleSize = 4;

private:
    QString m_softwareRevision;
    int m_buttonPresses = 0;
    QTimer *m_keyPressTimer = nullptr;
};

#endif // BIPDEVICE_H
