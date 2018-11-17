#include "mibandservice.h"
#include <QDateTime>
#include <QTimeZone>

#include "typeconversion.h"
#include "bipdevice.h"
#include "weather/huamiweathercondition.h"

const char* MiBandService::UUID_SERVICE_MIBAND = "0000fee0-0000-1000-8000-00805f9b34fb";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_NOTIFICATION = "00000002-0000-3512-2118-0009af100700";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_CONFIGURATION = "00000003-0000-3512-2118-0009af100700";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA = "00000004-0000-3512-2118-0009af100700";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA = "00000005-0000-3512-2118-0009af100700";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO = "00000006-0000-3512-2118-0009af100700";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_REALTIME_STEPS = "00000007-0000-3512-2118-0009af100700";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_USER_SETTINGS = "00000008-0000-3512-2118-0009af100700";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_DEVICE_EVENT = "00000010-0000-3512-2118-0009af100700";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_CURRENT_TIME = "00002a2b-0000-1000-8000-00805f9b34fb";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_WEATHER = "0000000e-0000-3512-2118-0009af100700";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER = "00000020-0000-3512-2118-0009af100700";


constexpr char MiBandService::DATEFORMAT_TIME[];
constexpr char MiBandService::DATEFORMAT_DATETIME[];
constexpr char MiBandService::DATEFORMAT_TIME_12_HOURS[];
constexpr char MiBandService::DATEFORMAT_TIME_24_HOURS[];
constexpr char MiBandService::COMMAND_ENABLE_DISPLAY_ON_LIFT_WRIST[];
constexpr char MiBandService::COMMAND_DISABLE_DISPLAY_ON_LIFT_WRIST[];
constexpr char MiBandService::COMMAND_SCHEDULE_DISPLAY_ON_LIFT_WRIST[];
constexpr char MiBandService::COMMAND_ENABLE_GOAL_NOTIFICATION[];
constexpr char MiBandService::COMMAND_DISABLE_GOAL_NOTIFICATION[];
constexpr char MiBandService::COMMAND_ENABLE_ROTATE_WRIST_TO_SWITCH_INFO[];
constexpr char MiBandService::COMMAND_DISABLE_ROTATE_WRIST_TO_SWITCH_INFO[];
constexpr char MiBandService::COMMAND_ENABLE_DISPLAY_CALLER[];
constexpr char MiBandService::COMMAND_DISABLE_DISPLAY_CALLER[];
constexpr char MiBandService::COMMAND_DISTANCE_UNIT_METRIC[];
constexpr char MiBandService::COMMAND_DISTANCE_UNIT_IMPERIAL[];
constexpr char MiBandService::COMMAND_SET_FITNESS_GOAL_START[];
constexpr char MiBandService::COMMAND_SET_FITNESS_GOAL_END[];
constexpr char MiBandService::COMMAND_CHANGE_SCREENS[];
constexpr char MiBandService::DISPLAY_XXX[];
constexpr char MiBandService::DISPLAY_YYY[];
constexpr char MiBandService::WEAR_LOCATION_LEFT_WRIST[];
constexpr char MiBandService::WEAR_LOCATION_RIGHT_WRIST[];
constexpr char MiBandService::RESPONSE_ACTIVITY_DATA_START_DATE_SUCCESS[];
constexpr char MiBandService::RESPONSE_FINISH_SUCCESS[];
constexpr char MiBandService::RESPONSE_FINISH_FAIL[];

MiBandService::MiBandService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_MIBAND, path, parent)
{
    qDebug() << "MiBandService::MiBandService";

    connect(this, &QBLEService::characteristicChanged, this, &MiBandService::characteristicChanged);
    connect(this, &QBLEService::characteristicRead, this, &MiBandService::characteristicRead);
}

void MiBandService::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << "MiBand Changed:" << characteristic << value.toHex();

    if (value[0] == RESPONSE && value[1] == COMMAND_REQUEST_GPS_VERSION && value[2] == SUCCESS) {
        m_gpsVersion = value.mid(3);
        qDebug() << "Got gps version = " << m_gpsVersion;
        emit gpsVersionChanged();
    }

    if (characteristic == UUID_CHARACTERISTIC_MIBAND_DEVICE_EVENT) {
        if (value[0] == EVENT_DECLINE_CALL) {
            emit declineCall();
        } else if (value[0] == EVENT_IGNORE_CALL) {
            emit ignoreCall();
        } else if (value[0] == EVENT_BUTTON) {
            emit buttonPressed();
        }
    } else if (characteristic == UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO) {
        qDebug() << "...Got battery info";
        m_batteryInfo.setData(value);
        emit batteryInfoChanged();
    } else if (characteristic == UUID_CHARACTERISTIC_MIBAND_REALTIME_STEPS) {
        qDebug() << "...Got realtime steps:" << value.length();
        if (value.length() == 13) {
            m_steps = TypeConversion::toUint16(value[1], value[2]);
            emit stepsChanged();
        }
    } else if (characteristic == UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA) {
        //qDebug() << "...got data";
        if (m_operationRunning == 1 && m_logFetchOperation) {
            m_logFetchOperation->handleData(value);
        } else if (m_operationRunning == 2 && m_activityFetchOperation) {
            m_activityFetchOperation->handleData(value);
        } else if (m_operationRunning == 3 && m_sportsSummaryOperation) {
            m_sportsSummaryOperation->handleData(value);
        } else if (m_operationRunning == 4 && m_sportsDetailOperation) {
            m_sportsDetailOperation->handleData(value);
        }
    } else if (characteristic == UUID_CHARACTERISTIC_MIBAND_FETCH_DATA) {
        qDebug() << "...got metadata";
        if (m_operationRunning == 1 && m_logFetchOperation) {
            if (m_logFetchOperation->handleMetaData(value)) {
                delete m_logFetchOperation;
                m_logFetchOperation = nullptr;
                m_operationRunning = 0;
                emit operationRunningChanged();
            }
        } else if (m_operationRunning == 2 && m_activityFetchOperation) {
            if (m_activityFetchOperation->handleMetaData(value)) {
                delete m_activityFetchOperation;
                m_activityFetchOperation = nullptr;
                m_operationRunning = 0;
                emit operationRunningChanged();
            }
        } else if (m_operationRunning == 3 && m_sportsSummaryOperation) {
            if (m_sportsSummaryOperation->handleMetaData(value)) {
                //Now the summary is finished, need to get the detail
                bool createDetail = false;
                ActivitySummary summary;
                if (m_sportsSummaryOperation->success()) {
                    qDebug() << "Finished summary data, now getting track detail";
                    summary = m_sportsSummaryOperation->summary();
                    createDetail = true;
                }

                delete m_sportsSummaryOperation;
                m_sportsSummaryOperation = nullptr;
                m_operationRunning = 0;
                emit operationRunningChanged();
                if (createDetail) {
                    m_sportsDetailOperation = new SportsDetailOperation(this, m_conn, summary);
                    m_operationRunning = 4;
                    m_sportsDetailOperation->start();
                    emit operationRunningChanged();
                }
            }
        }  else if (m_operationRunning == 4 && m_sportsDetailOperation) {
            if (m_sportsDetailOperation->handleMetaData(value)) {
                delete m_sportsDetailOperation;
                m_sportsDetailOperation = nullptr;
                m_operationRunning = 0;
                emit operationRunningChanged();
            }
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
    writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(COMMAND_REQUEST_GPS_VERSION, 1));
}


QString MiBandService::gpsVersion()
{
    return m_gpsVersion;
}

void MiBandService::requestBatteryInfo()
{
    readValue(UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO);
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

    QTimeZone tz = QTimeZone::systemTimeZone();
    int utcOffset = tz.standardTimeOffset(now);

    qDebug() << tz << utcOffset << (utcOffset / (60 * 60)) * 4;

    timeBytes += char((utcOffset / (60 * 60)) * 4);

    qDebug() << "setting time to:" << now << timeBytes.toHex();
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
    case 4:
        lang += "ru_RU";
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

void MiBandService::setAlertFitnessGoal()
{
    bool alert = m_settings.value("/uk/co/piggz/amazfish/profile/alertfitnessgoal").toBool();

    if (alert) {
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(COMMAND_ENABLE_GOAL_NOTIFICATION,4));
    } else {
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(COMMAND_DISABLE_GOAL_NOTIFICATION,4));
    }
}


void MiBandService::setEnableDisplayOnLiftWrist()
{
    bool disp = m_settings.value("/uk/co/piggz/amazfish/profile/displayonliftwrist").toBool();

    if (disp) {
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(COMMAND_ENABLE_DISPLAY_ON_LIFT_WRIST,4));
    } else {
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(COMMAND_DISABLE_DISPLAY_ON_LIFT_WRIST,4));
    }
}

void MiBandService::setDisplayItems()
{
    char items1 = 0x01; //Always display clock
    char items2 = 0x10;
    bool sw = false;
    bool sa = false;


    BipDevice *device = qobject_cast<BipDevice*>(parent());

    if (!device || device->softwareRevision() < "V0.1.1.14") { //Lexical string comparison should be fine here
        message(tr("Firmware is too old to set display items, V0.1.1.14 is required"));
        return;
    }

    if (m_settings.value("/uk/co/piggz/amazfish/device/displaystatus", QVariant(true)).toBool()) {
        items1 |= 0x02;
    }
    if (m_settings.value("/uk/co/piggz/amazfish/device/displayactivity", QVariant(true)).toBool()) {
        items1 |= 0x04;
    }
    if (m_settings.value("/uk/co/piggz/amazfish/device/displayweather", QVariant(true)).toBool()) {
        items1 |= 0x08;
    }
    if (m_settings.value("/uk/co/piggz/amazfish/device/displayalarm", QVariant(true)).toBool()) {
        items1 |= 0x10;
    }
    if (m_settings.value("/uk/co/piggz/amazfish/device/displaytimer", QVariant(true)).toBool()) {
        items1 |= 0x20;
    }
    if (m_settings.value("/uk/co/piggz/amazfish/device/displaycompass", QVariant(true)).toBool()) {
        items1 |= 0x40;
    }
    if (m_settings.value("/uk/co/piggz/amazfish/device/displaysettings", QVariant(true)).toBool()) {
        items1 |= 0x80;
    }
    if (m_settings.value("/uk/co/piggz/amazfish/device/displayalipay", QVariant(true)).toBool()) {
        items2 |= 0x01;
    }


    sw = m_settings.value("/uk/co/piggz/amazfish/device/displayweathershortcut", QVariant(true)).toBool();
    sa = m_settings.value("/uk/co/piggz/amazfish/device/displayalipayshortcut", QVariant(true)).toBool();

    QByteArray cmd = QByteArray(COMMAND_CHANGE_SCREENS, 12);
    cmd[1] = items1;
    cmd[2] = items2;

    qDebug() << "Setting display items to:" << cmd;

    writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, cmd);

    QByteArray shortcuts;
    shortcuts += 0x10;
    shortcuts += ((sa || sw) ? 0x80 : 0x00);
    shortcuts += (sw ? 0x02 : 0x01);
    shortcuts += ((sa && sw) ? 0x81 : 0x01);
    shortcuts += 0x01;

    qDebug() << "Setting shortcuts to:" << shortcuts;

    writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, shortcuts);
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

void MiBandService::setDisplayCaller()
{
    writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(COMMAND_ENABLE_DISPLAY_CALLER, 5));
}

void MiBandService::setInactivityWarnings()
{

}

int MiBandService::steps() const
{
    return m_steps;
}

void MiBandService::fetchLogs()
{
    if (!m_logFetchOperation && m_operationRunning == 0) {
        m_operationRunning = 1;
        m_logFetchOperation = new LogFetchOperation(this);
        m_logFetchOperation->start();
        emit operationRunningChanged();
    } else {
        emit message(tr("An operation is currently running, please try later"));
    }
}

void MiBandService::fetchActivityData()
{
    if (!m_activityFetchOperation && m_operationRunning == 0) {
        m_operationRunning = 2;
        m_activityFetchOperation = new ActivityFetchOperation(this, m_conn);
        m_activityFetchOperation->start();
        emit operationRunningChanged();
    } else {
        emit message(tr("An operation is currently running, please try later"));
    }
}

void MiBandService::fetchSportsSummaries()
{
    if (!m_sportsSummaryOperation && m_operationRunning == 0) {
        m_operationRunning = 3;
        m_sportsSummaryOperation = new SportsSummaryOperation(this, m_conn);
        m_sportsSummaryOperation->start();
        emit operationRunningChanged();
    } else {
        emit message(tr("An operation is currently running, please try later"));
    }
}

void MiBandService::setDatabase(KDbConnection *conn)
{
    m_conn = conn;
}

void MiBandService::setAlarms()
{
    for (int i =0; i < 5; ++i) {
        int base =0;
        int repeatMask=0;

        QString configBase = "/uk/co/piggz/amazfish/alarms/alarm" + QString::number(i+1) + "/";

        bool enabled = (m_settings.value(configBase + "enabled", QVariant(false)).toBool());
        if (enabled) {
            base = 128;
        }

        repeatMask = m_settings.value(configBase + "repeat", 0).toInt();
        if (repeatMask == 0) {
            repeatMask = 128;
        }

        QByteArray cmd;
        cmd += 0x02;
        cmd += (base + i);
        cmd += m_settings.value(configBase + "hour", 0).toInt();
        cmd += m_settings.value(configBase + "minute", 0).toInt();
        cmd += repeatMask;

        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, cmd);
    }
}

bool MiBandService::operationRunning()
{
    qDebug() << "is miband operation running:" << m_operationRunning;
    return m_operationRunning > 0;
}

void MiBandService::abortOperations()
{
    if (m_logFetchOperation) {
        delete m_logFetchOperation;
        m_logFetchOperation = nullptr;
    }
    if (m_activityFetchOperation) {
        delete m_activityFetchOperation;
        m_activityFetchOperation = nullptr;
    }
    if (m_sportsSummaryOperation) {
        delete m_sportsSummaryOperation;
        m_sportsSummaryOperation = nullptr;
    }
    if (m_sportsDetailOperation) {
        delete m_sportsDetailOperation;
        m_sportsDetailOperation = nullptr;
    }
    m_operationRunning = 0;
    emit operationRunningChanged();
}

void MiBandService::sendWeather(const CurrentWeather *weather)
{
    BipDevice *device = qobject_cast<BipDevice*>(parent());

    bool supportsConditionString = false;

    if (!device || device->softwareRevision() > "V0.0.8.74") { //Lexical string comparison should be fine here
        message(tr("Firmware supports weather condition string"));
        supportsConditionString = true;
    }

    char condition = HuamiWeatherCondition::mapToAmazfitBipWeatherCode(weather->weatherCode());

    //int tz_offset_hours = SimpleTimeZone.getDefault().getOffset(weather->timestamp * 1000L) / (1000 * 60 * 60);
    int tz_offset_hours = 0; //!TODO work out what to do with this

    qDebug() << "Sending condition";
    QByteArray buf;
    QBuffer buffer(&buf);
    buffer.open(QIODevice::WriteOnly);

    char temp = weather->temperature() - 273;
    qint32 dt = qToLittleEndian(weather->dateTime());

    qDebug() << dt << temp << condition;

    buffer.putChar(char(0x02));
    buffer.write((char*)&dt, sizeof(qint32));
    buffer.putChar(char(tz_offset_hours * 4));
    buffer.putChar(condition);
    buffer.putChar(temp);
    if (supportsConditionString) {
        buffer.write(weather->description().toLatin1());
        buffer.putChar((char)0x00);
    }

    writeChunked(UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 1, buf);
    buf.clear();
    buffer.close();

    qDebug() << "Sending aqi";
    buffer.open(QIODevice::WriteOnly);
    buffer.putChar(char(0x04));
    buffer.write((char*)&dt, sizeof(qint32));
    buffer.putChar(char(tz_offset_hours * 4));
    buffer.putChar(char(0x00)); //No AQI data, so just write 0
    buffer.putChar(char(0x00));
    if (supportsConditionString) {
        buffer.write(QString("(n/a)").toLatin1());
        buffer.putChar((char)0x00);
    }
    writeChunked(UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 1, buf);

    qDebug() << "Sending forecast";

    buf.clear();
    char NR_DAYS = (char) (qMin(weather->forecastCount(), 6) + 1);

    buffer.open(QIODevice::WriteOnly);

    dt = qToLittleEndian(weather->dateTime());

    qDebug() << dt << temp << condition;

    buffer.putChar(char(0x01));
    buffer.write((char*)&dt, sizeof(qint32));
    buffer.putChar(char(tz_offset_hours * 4));
    buffer.putChar(NR_DAYS);
    buffer.putChar(condition);
    buffer.putChar(condition);
    buffer.putChar((char) (weather->maxTemperature() - 273));
    buffer.putChar((char) (weather->minTemperature() - 273));
    if (supportsConditionString) {
        buffer.write(weather->description().toLatin1());
        buffer.putChar((char)0x00);
    }

    for (int f = 0; f < NR_DAYS - 1; f++) {
        CurrentWeather::Forecast fc = weather->forecast(f);
        qDebug() << "Forecast:" << f << fc.weatherCode() << fc.maxTemperature() << fc.minTemperature();

        condition = HuamiWeatherCondition::mapToAmazfitBipWeatherCode(fc.weatherCode());

        buffer.putChar(condition);
        buffer.putChar(condition);
        buffer.putChar((char) (fc.maxTemperature() - 273));
        buffer.putChar((char) (fc.minTemperature() - 273));

        if (supportsConditionString) {
            buffer.write(fc.description().toLatin1());
            buffer.putChar((char)0x00);
        }
    }
    writeChunked(UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 1, buf);
}

void MiBandService::writeChunked(const QString &characteristic, int type, const QByteArray &value) {
    int MAX_CHUNKLENGTH = 17;
    int remaining = value.length();
    char count = 0;
    while (remaining > 0) {
        int copybytes = qMin(remaining, MAX_CHUNKLENGTH);
        QByteArray chunk;

        char flags = 0;
        if (remaining <= MAX_CHUNKLENGTH) {
            flags |= 0x80; // last chunk
            if (count == 0) {
                flags |= 0x40; // weird but true
            }
        } else if (count > 0) {
            flags |= 0x40; // consecutive chunk
        }

        chunk += (char) 0x00;
        chunk += (char) (flags | type);
        chunk += (char) (count & 0xff);

        chunk += value.mid(count++ * MAX_CHUNKLENGTH, copybytes);

        writeValue(characteristic, chunk);
        remaining -= copybytes;
    }
}
