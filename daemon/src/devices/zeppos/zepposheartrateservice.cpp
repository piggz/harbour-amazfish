#include "zepposheartrateservice.h"
#include "hrmservice.h"

ZeppOsHeartRateService::ZeppOsHeartRateService(ZeppOSDevice *device) : AbstractZeppOsService(device, false)
{
    m_endpoint = 0x001d;
}

void ZeppOsHeartRateService::handlePayload(const QByteArray &payload)
{
    switch (payload[0]) {
    case CMD_REALTIME_ACK:
        // what does the status mean? Seems to be 0 on success
        qDebug() << "Band acknowledged heart rate command, status = " << payload[1];
        return;
    case CMD_SLEEP:
        switch (payload[1]) {
        case SLEEP_EVENT_FALL_ASLEEP:
            qDebug() << "Fell asleep";
            //evaluateGBDeviceEvent(new GBDeviceEventSleepStateDetection(SleepState.ASLEEP));
            break;
        case SLEEP_EVENT_WAKE_UP:
            qDebug() << "Woke up";
            //evaluateGBDeviceEvent(new GBDeviceEventSleepStateDetection(SleepState.AWAKE));
            break;
        default:
            qDebug() << "Unexpected sleep byte " << payload[1];
            break;
        }
        return;
    }

    qDebug() << "Unexpected heart rate byte " << payload[0];
}

QString ZeppOsHeartRateService::name() const
{
    return "HeartRate";
}

void ZeppOsHeartRateService::enableRealtimeHeartRateMeasurement(bool enable, bool oneshot)
{
    qDebug() << Q_FUNC_INFO << enable;

    if (enable == realtimeStarted) {
        // same state, ignore
        return;
    }

    realtimeStarted = enable;
    realtimeOneShot = oneshot;

    QBLEService *hr = m_device->service(HRMService::UUID_SERVICE_HRM);
    if (hr) {
        hr->enableNotification(HRMService::UUID_CHARACTERISTIC_HRM_MEASUREMENT);
    }

    QByteArray cmd;
    cmd += CMD_REALTIME_SET;
    cmd += (enable ? REALTIME_MODE_START : REALTIME_MODE_STOP);

    write(cmd);

    //realtimeHandler.removeCallbacksAndMessages(null);
    //if (enable) {
    //    scheduleContinue();
    //}

}

void ZeppOsHeartRateService::handleHeartRate(const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO;

    if (realtimeOneShot) {
        realtimeStarted = false;
    }

    if (!realtimeOneShot && !realtimeStarted) {
        // Failsafe in case it gets out of sync, stop it
        enableRealtimeHeartRateMeasurement(false, true);
        return;
    }

    if (value.size() == 2 && value[0] == 0) {
        m_heartRate = (value[1] & 0xff);
        qDebug() << "Real-time hr: " << m_heartRate;

        m_device->informationChanged(AbstractDevice::INFO_HEARTRATE, QString::number(m_heartRate));

        //if (realtimeSamplesAggregator != null) {
        //    realtimeSamplesAggregator.broadcastHeartRate(hrValue);
        //}
        //if (sleepAsAndroidSender != null) {
        //    sleepAsAndroidSender.onHrChanged(hrValue, 0);
        //}
    }
}
