#include "hrmservice.h"

const char* HRMService::UUID_SERVICE_HRM = "0000180d-0000-1000-8000-00805f9b34fb";
const char* HRMService::UUID_CHARACTERISTIC_HRM_MEASUREMENT = "00002a37-0000-1000-8000-00805f9b34fb";
const char* HRMService::UUID_CHARACTERISTIC_HRM_CONTROL = "00002a39-0000-1000-8000-00805f9b34fb";


HRMService::HRMService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_HRM, path, parent)
{
    qDebug() << "HRMService::HRMService";

    connect(this, &QBLEService::characteristicChanged, this, &HRMService::characteristicChanged);
}


void HRMService::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << "HRM Changed:" << characteristic << value;

    if (characteristic == UUID_CHARACTERISTIC_HRM_MEASUREMENT) {
        qDebug() << "..got HR measurement";
        if (value.length() == 2 && value[0] == 0) {
            m_heartRate = (value[1] & 0xff);
            emit heartRateChanged();
        }
    }
}

int HRMService::heartRate() const
{
    return m_heartRate;
}

void HRMService::enableRealtimeHRMeasurement(bool enable)
{
    enableNotification(UUID_CHARACTERISTIC_HRM_MEASUREMENT);

    if (enable) {
        writeValue(UUID_CHARACTERISTIC_HRM_CONTROL, QByteArray(COMMAND_SET_HR_MANUAL_DISABLE, 3));
        writeValue(UUID_CHARACTERISTIC_HRM_CONTROL, QByteArray(COMMAND_SET_HR_CONTINUOUS_ENABLE, 3));
    } else {
        writeValue(UUID_CHARACTERISTIC_HRM_CONTROL, QByteArray(COMMAND_SET_HR_CONTINUOUS_DISABLE, 3));
    }
}


void HRMService::enableManualHRMeasurement(bool enable)
{
    enableNotification(UUID_CHARACTERISTIC_HRM_MEASUREMENT);

    if (enable) {
        writeValue(UUID_CHARACTERISTIC_HRM_CONTROL, QByteArray(COMMAND_SET_HR_CONTINUOUS_DISABLE, 3));
        writeValue(UUID_CHARACTERISTIC_HRM_CONTROL, QByteArray(COMMAND_SET_HR_MANUAL_ENABLE, 3));
    } else {
        writeValue(UUID_CHARACTERISTIC_HRM_CONTROL, QByteArray(COMMAND_SET_HR_MANUAL_DISABLE, 3));
    }
}

void HRMService::setAllDayHRM()
{
    int interval = m_settings.value("/uk/co/piggz/amazfish/profile/alldayhrm").toInt();

    qDebug() << "Setting HRM monitoring to" << interval;

    QByteArray cmd = QByteArray(1, COMMAND_SET_PERIODIC_HR_MEASUREMENT_INTERVAL);
    cmd += QByteArray(1, interval);

    enableNotification(UUID_CHARACTERISTIC_HRM_CONTROL);
    writeValue(UUID_CHARACTERISTIC_HRM_CONTROL, cmd);
    disableNotification(UUID_CHARACTERISTIC_HRM_CONTROL);
}
