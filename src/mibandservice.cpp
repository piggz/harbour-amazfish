#include "mibandservice.h"
#include <QDateTime>
#include <QTimeZone>

#include "typeconversion.h"

const char* MiBandService::UUID_SERVICE_MIBAND = "{0000FEE0-0000-1000-8000-00805f9b34fb}";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_CONFIGURATION = "{00000003-0000-3512-2118-0009af100700}";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO = "{00000006-0000-3512-2118-0009af100700}";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_DEVICE_EVENT = "{00000010-0000-3512-2118-0009af100700}";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_NOTIFICATION = "{00000002-0000-3512-2118-0009af100700)";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_CURRENT_TIME = "{00002a2b-0000-1000-8000-00805f9b34fb)";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_USER_SETTINGS = "{00000008-0000-3512-2118-0009af100700)";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_REALTIME_STEPS = "{00000007-0000-3512-2118-0009af100700)";

MiBandService::MiBandService(QObject *parent) : QBLEService(UUID_SERVICE_MIBAND, parent)
{
    connect(this, &QBLEService::characteristicChanged, this, &MiBandService::characteristicChanged);
    connect(this, &QBLEService::characteristicRead, this, &MiBandService::characteristicRead);

    //    connect(this, &BipService::readyChanged, this, &MiBandService::serviceReady);
}

void MiBandService::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << "MiBand Changed:" << characteristic << value;

    if (value[0] == CHAR_RESPONSE && value[1] == COMMAND_REQUEST_GPS_VERSION && value[2] == CHAR_SUCCESS) {
        m_gpsVersion = value.mid(3);
        qDebug() << "Got gps version = " << m_gpsVersion;
        emit gpsVersionChanged();
    }

    if (characteristic == UUID_CHARACTERISTIC_MIBAND_DEVICE_EVENT) {
        if (value[0] == EVENT_DECLINE_CALL) {
            emit declineCall();
        } else if (value[0] == EVENT_IGNORE_CALL) {
            emit ignoreCall();
        }
    } else if (characteristic == UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO) {
        qDebug() << "...Got battery info";
        m_batteryInfo.setData(value);
        emit batteryInfoChanged();
    } else if (characteristic == UUID_CHARACTERISTIC_MIBAND_REALTIME_STEPS) {
        qDebug() << "...Got realtime steps";
        if (value.length() == 13) {
            m_steps = TypeConversion::toUint16(value[1], value[2]);
            emit stepsChanged();
        }
    }
}

void MiBandService::characteristicRead(const QString &characteristic, const QByteArray &value)
{
    qDebug() << "Read:" << characteristic << value;

    if (characteristic == UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO) {
        qDebug() << "...Got battery info";
        m_batteryInfo.setData(value);
        emit batteryInfoChanged();
    }
}

void MiBandService::requestGPSVersion()
{
    writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(&COMMAND_REQUEST_GPS_VERSION, 1));
}


QString MiBandService::gpsVersion()
{
    return m_gpsVersion;
}

void MiBandService::requestBatteryInfo()
{
    readCharacteristic(UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO);
}

int MiBandService::batteryInfo()
{
    return m_batteryInfo.currentChargeLevelPercent();
}

void MiBandService::setCurrentTime()
{
    QByteArray timeBytes;

    QDateTime now = QDateTime::currentDateTime();
    timeBytes += TypeConversion::fromInt16(now.date().year());
    timeBytes += TypeConversion::fromInt8(now.date().month());
    timeBytes += TypeConversion::fromInt8(now.date().day());
    timeBytes += TypeConversion::fromInt8(now.time().hour());
    timeBytes += TypeConversion::fromInt8(now.time().minute());
    timeBytes += TypeConversion::fromInt8(now.time().second());
    timeBytes += TypeConversion::fromInt8(now.date().dayOfWeek());
    timeBytes += char(0); //fractions of seconds
    timeBytes += char(0); //timezone marker?

    int utcOffset = QTimeZone::systemTimeZone().offsetFromUtc(now);

    qDebug() << utcOffset << (utcOffset / (60 * 60)) * 2;

    timeBytes += char((utcOffset / (60 * 60)) * 2);

    qDebug() << "setting time to:" << timeBytes.toHex();
    writeValue(UUID_CHARACTERISTIC_MIBAND_CURRENT_TIME, timeBytes);
}

void MiBandService::setLanguage()
{
    uint format = m_settings.value("/uk/co/piggz/amazfish/device/language").toUInt();

    qDebug() << "Setting language to " << format;

    QByteArray lang;
    lang += QByteArray(1, ENDPOINT_DISPLAY);
    lang += QByteArray(1, COMMAND_SET_LANGUAGE);
    lang += char(0);

    switch (format) {
    case 0:
        lang += "en_US";
        break;
    case 1:
        lang += "es_ES";
        break;
    case 2:
        lang += "zh_CN";
        break;
    case 3:
        lang += "zh_TW";
        break;
    default:
        lang += "en_US";
    }

    writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, lang);
}

void MiBandService::setDateDisplay()
{
    uint format = m_settings.value("/uk/co/piggz/amazfish/device/dateformat").toUInt();

    qDebug() << "Setting date display to " << format;
    switch (format) {
    case 0:
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(DATEFORMAT_TIME, 4));
        break;
    case 1:
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(DATEFORMAT_DATETIME, 4));
        break;
    }

}
void MiBandService::setTimeFormat()
{
    uint format = m_settings.value("/uk/co/piggz/amazfish/device/timeformat").toUInt();

    qDebug() << "Setting time format to " << format;
    switch (format) {
    case 0:
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(DATEFORMAT_TIME_24_HOURS, 4));
        break;
    case 1:
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(DATEFORMAT_TIME_12_HOURS, 4));
        break;
    }
}
void MiBandService::setUserInfo()
{
    QByteArray userInfo;
    QString profileName = m_settings.value("/uk/co/piggz/amazfish/profile/name").toString();
    uint id = qHash(profileName);
    uint gender = m_settings.value("/uk/co/piggz/amazfish/profile/gender").toUInt();
    uint height = m_settings.value("/uk/co/piggz/amazfish/profile/height").toUInt();
    uint weight = m_settings.value("/uk/co/piggz/amazfish/profile/weight").toUInt();
    QDate dob = m_settings.value("/uk/co/piggz/amazfish/profile/dob").toDateTime().date();

    qDebug() << "Setting profile" << profileName << id << gender << height << weight << dob;

    userInfo += QByteArray(1, COMMAND_SET_USERINFO);
    userInfo += char(0);
    userInfo += char(0);
    //DOB
    userInfo += char(dob.year() & 0xff);
    userInfo += char((dob.year() >> 8) & 0xff);
    userInfo += char(dob.month());
    userInfo += char(dob.day());

    //Gender
    userInfo += char(gender);

    //Height
    userInfo += char(height & 0xff);
    userInfo += char((height >> 8) & 0xff);

    //Weight
    userInfo += char(weight & 0xff);
    userInfo += char((weight >> 8) & 0xff);
    //User ID
    userInfo += char(id & 0xff);
    userInfo += char((id >> 8) & 0xff);
    userInfo += char((id >> 16) & 0xff);
    userInfo += char((id >> 24) & 0xff);

    writeValue(UUID_CHARACTERISTIC_MIBAND_USER_SETTINGS, userInfo);

}
void MiBandService::setDistanceUnit()
{
    uint format = m_settings.value("/uk/co/piggz/amazfish/device/distanceunit").toUInt();

    qDebug() << "Setting distance unit to " << format;
    switch (format) {
    case 0:
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(COMMAND_DISTANCE_UNIT_METRIC, 4));
        break;
    case 1:
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(COMMAND_DISTANCE_UNIT_IMPERIAL, 4));
        break;
    }

}

//Only use during device init
void MiBandService::setWearLocation()
{
    uint location = m_settings.value("/uk/co/piggz/amazfish/profile/wearlocation").toUInt();

    qDebug() << "Setting wear location to " << location;
    switch (location) {
    case 0:
        writeValue(UUID_CHARACTERISTIC_MIBAND_USER_SETTINGS, QByteArray(WEAR_LOCATION_LEFT_WRIST, 4));
        break;
    case 1:
        writeValue(UUID_CHARACTERISTIC_MIBAND_USER_SETTINGS, QByteArray(WEAR_LOCATION_LEFT_WRIST, 4));
        break;
    }
}

void MiBandService::setFitnessGoal()
{
    uint goal = m_settings.value("/uk/co/piggz/amazfish/profile/fitnessgoal").toUInt();

    QByteArray cmd = QByteArray(COMMAND_SET_FITNESS_GOAL_START, 3);
    cmd += TypeConversion::fromInt24(goal);
    cmd += QByteArray(COMMAND_SET_FITNESS_GOAL_END, 2);

    writeValue(UUID_CHARACTERISTIC_MIBAND_USER_SETTINGS, cmd);
}

void MiBandService::setDisplayItems()
{

}
void MiBandService::setDoNotDisturb()
{

}
void MiBandService::setRotateWristToSwitchInfo(bool enable)
{
    qDebug() << "Setting rotate write to " << enable;
    if (enable) {
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(COMMAND_ENABLE_ROTATE_WRIST_TO_SWITCH_INFO, 4));
    } else {
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(COMMAND_DISABLE_ROTATE_WRIST_TO_SWITCH_INFO, 4));
    }
}
void MiBandService::setActivateDisplayOnLiftWrist()
{

}
void MiBandService::setDisplayCaller()
{
    writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(COMMAND_ENABLE_DISPLAY_CALLER, 5));
}
void MiBandService::setGoalNotification(bool enable)
{
    qDebug() << "Setting goal notification to " << enable;
    if (enable) {
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(COMMAND_ENABLE_GOAL_NOTIFICATION, 4));
    } else {
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(COMMAND_DISABLE_GOAL_NOTIFICATION, 4));
    }
}
void MiBandService::setInactivityWarnings()
{

}
void MiBandService::setHeartrateSleepSupport(){

}

int MiBandService::steps() const
{
    return m_steps;
}
