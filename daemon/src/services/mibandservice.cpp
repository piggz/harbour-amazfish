#include "mibandservice.h"
#include <QDateTime>
#include <QTimeZone>

#include "typeconversion.h"
#include "huamidevice.h"
#include "huamiweathercondition.h"
#include "amazfishconfig.h"

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
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_2021_CHUNKED_CHAR_WRITE = "00000016-0000-3512-2118-0009af100700";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_2021_CHUNKED_CHAR_READ = "00000017-0000-3512-2118-0009af100700";

constexpr uint8_t MiBandService::DATEFORMAT_TIME[];
constexpr uint8_t MiBandService::DATEFORMAT_DATETIME[];
constexpr uint8_t MiBandService::DATEFORMAT_TIME_12_HOURS[];
constexpr uint8_t MiBandService::DATEFORMAT_TIME_24_HOURS[];
constexpr uint8_t MiBandService::COMMAND_ENABLE_DISPLAY_ON_LIFT_WRIST[];
constexpr uint8_t MiBandService::COMMAND_DISABLE_DISPLAY_ON_LIFT_WRIST[];
constexpr uint8_t MiBandService::COMMAND_SCHEDULE_DISPLAY_ON_LIFT_WRIST[];
constexpr uint8_t MiBandService::COMMAND_ENABLE_GOAL_NOTIFICATION[];
constexpr uint8_t MiBandService::COMMAND_DISABLE_GOAL_NOTIFICATION[];
constexpr uint8_t MiBandService::COMMAND_ENABLE_ROTATE_WRIST_TO_SWITCH_INFO[];
constexpr uint8_t MiBandService::COMMAND_DISABLE_ROTATE_WRIST_TO_SWITCH_INFO[];
constexpr uint8_t MiBandService::COMMAND_ENABLE_DISPLAY_CALLER[];
constexpr uint8_t MiBandService::COMMAND_DISABLE_DISPLAY_CALLER[];
constexpr uint8_t MiBandService::COMMAND_DISTANCE_UNIT_METRIC[];
constexpr uint8_t MiBandService::COMMAND_DISTANCE_UNIT_IMPERIAL[];
constexpr uint8_t MiBandService::COMMAND_SET_FITNESS_GOAL_START[];
constexpr uint8_t MiBandService::COMMAND_SET_FITNESS_GOAL_END[];
constexpr uint8_t MiBandService::COMMAND_CHANGE_SCREENS[];
constexpr uint8_t MiBandService::COMMAND_ENABLE_DISCONNECT_NOTIFICATION[];
constexpr uint8_t MiBandService::COMMAND_DISABLE_DISCONNECT_NOTIFICATION[];

constexpr uint8_t MiBandService::DISPLAY_XXX[];
constexpr uint8_t MiBandService::DISPLAY_YYY[];
constexpr uint8_t MiBandService::WEAR_LOCATION_LEFT_WRIST[];
constexpr uint8_t MiBandService::WEAR_LOCATION_RIGHT_WRIST[];
constexpr uint8_t MiBandService::RESPONSE_ACTIVITY_DATA_START_DATE_SUCCESS[];
constexpr uint8_t MiBandService::RESPONSE_FINISH_SUCCESS[];
constexpr uint8_t MiBandService::RESPONSE_FINISH_FAIL[];

MiBandService::MiBandService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_MIBAND, path, parent)
{
    qDebug() << "MiBandService::MiBandService";

    connect(this, &QBLEService::characteristicChanged, this, &MiBandService::characteristicChanged);
    connect(this, &QBLEService::characteristicRead, this, &MiBandService::characteristicRead);

    m_operationTimeout = new QTimer();
    connect(m_operationTimeout, &QTimer::timeout, this, &MiBandService::operationTimeout);

    displayItemsIdMap["status"] = 0x01;
    displayItemsIdMap["hr"] = 0x02;
    displayItemsIdMap["workout"] = 0x03;
    displayItemsIdMap["weather"] = 0x04;
    displayItemsIdMap["notifications"] = 0x06;
    displayItemsIdMap["more"] = 0x07;
    displayItemsIdMap["dnd"] = 0x08;
    displayItemsIdMap["alarm"] = 0x09;
    displayItemsIdMap["takephoto"] = 0x0a;
    displayItemsIdMap["music"] = 0x0b;
    displayItemsIdMap["stopwatch"] = 0x0c;
    displayItemsIdMap["timer"] = 0x0d;
    displayItemsIdMap["findphone"] = 0x0e;
    displayItemsIdMap["mutephone"] = 0x0f;
    displayItemsIdMap["nfc"] = 0x10;
    displayItemsIdMap["alipay"] = 0x11;
    displayItemsIdMap["watchface"] = 0x12;
    displayItemsIdMap["settings"] = 0x13;
    displayItemsIdMap["activity"] = 0x14;
    displayItemsIdMap["eventreminder"] = 0x15;
    displayItemsIdMap["compass"] = 0x16;
    displayItemsIdMap["pai"] = 0x19;
    displayItemsIdMap["worldclock"] = 0x1a;
    displayItemsIdMap["timer_stopwatch"] = 0x1b;
    displayItemsIdMap["stress"] = 0x1c;
    displayItemsIdMap["period"] = 0x1d;
    displayItemsIdMap["goal"] = 0x21;
    displayItemsIdMap["sleep"] = 0x23;
    displayItemsIdMap["spo2"] = 0x24;
    displayItemsIdMap["events"] = 0x26;
    displayItemsIdMap["widgets"] = 0x28;
    displayItemsIdMap["breathing"] = 0x33;
    displayItemsIdMap["steps"] = 0x34;
    displayItemsIdMap["distance"] = 0x35;
    displayItemsIdMap["calories"] = 0x36;
    displayItemsIdMap["pomodoro"] = 0x38;
    displayItemsIdMap["alexa"] = 0x39;
    displayItemsIdMap["battery"] = 0x3a;
    displayItemsIdMap["temperature"] = 0x40;
    displayItemsIdMap["barometer"] = 0x41;
    displayItemsIdMap["flashlight"] = 0x43;
}

void MiBandService::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << "MiBand Changed:" << characteristic << value.toHex();

    if (characteristic == UUID_CHARACTERISTIC_MIBAND_DEVICE_EVENT) {
        if (value[0] == EVENT_DECLINE_CALL) {
            emit serviceEvent(EVENT_DECLINE_CALL);
        } else if (value[0] == EVENT_IGNORE_CALL) {
            emit serviceEvent(EVENT_IGNORE_CALL);
        } else if (value[0] == EVENT_BUTTON) {
            emit buttonPressed();
        } else if (value[0] == EVENT_MUSIC) {
            emit serviceEvent(value[1]);
        } else {
            qDebug() << "device event " << value[0];
            if (value[0] == MTU_REQUEST) {
                qDebug() << "mtu event";
            }
        }
    } else if (characteristic == UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO) {
        qDebug() << "...Got battery info";
        m_batteryInfo.setData(value);
        emit informationChanged(AbstractDevice::INFO_BATTERY, QString::number(m_batteryInfo.currentChargeLevelPercent()));

    } else if (characteristic == UUID_CHARACTERISTIC_MIBAND_REALTIME_STEPS) {
        qDebug() << "...Got realtime steps:" << value.length();
        if (value.length() == 13) {
            m_steps = TypeConversion::toUint16(value[1], value[2]);
            emit informationChanged(AbstractDevice::INFO_STEPS, QString::number(m_steps));
        }
    } else if (characteristic == UUID_CHARACTERISTIC_MIBAND_CONFIGURATION) {
        if (value[0] == RESPONSE && value[1] == COMMAND_REQUEST_GPS_VERSION && value[2] == SUCCESS) {
            m_gpsVersion = value.mid(3);
            qDebug() << "Got gps version = " << m_gpsVersion;
            emit informationChanged(AbstractDevice::INFO_GPSVER, m_gpsVersion);
        } else if (value[0] == RESPONSE && value[1] == COMMAND_REQUEST_ALARMS && value[2] == SUCCESS) {
            qDebug() << "Got alarm info = " << value.mid(3);
            decodeAlarms(value.mid(3));
        } else {
            qDebug() << "Unknown configuration info: " << value;
        }
    } else if (characteristic == UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA) {
        //qDebug() << "...got data";
        m_operationTimeout->start(10000);
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
        m_operationTimeout->start(10000);
        if (m_operationRunning == 1 && m_logFetchOperation) {
            if (m_logFetchOperation->handleMetaData(value)) {
                delete m_logFetchOperation;
                m_logFetchOperation = nullptr;
                m_operationRunning = 0;
                emit operationRunningChanged();
                m_operationTimeout->stop();
            }
        } else if (m_operationRunning == 2 && m_activityFetchOperation) {
            if (m_activityFetchOperation->handleMetaData(value)) {
                delete m_activityFetchOperation;
                m_activityFetchOperation = nullptr;
                m_operationRunning = 0;
                emit operationRunningChanged();
                m_operationTimeout->stop();
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
                m_operationTimeout->stop();
                if (createDetail) {
                    m_sportsDetailOperation = new SportsDetailOperation(this, m_conn, summary);
                    m_operationRunning = 4;
                    m_sportsDetailOperation->start();
                    emit operationRunningChanged();
                    m_operationTimeout->start(10000);
                }
            }
        }  else if (m_operationRunning == 4 && m_sportsDetailOperation) {
            if (m_sportsDetailOperation->handleMetaData(value)) {
                delete m_sportsDetailOperation;
                m_sportsDetailOperation = nullptr;
                m_operationRunning = 0;
                emit operationRunningChanged();
                m_operationTimeout->stop();
            }
        }
    }
}

void MiBandService::operationTimeout()
{
    qDebug() << "Timeout while waiting for operation data";
    abortOperations();
}

void MiBandService::decodeAlarms(const QByteArray &data)
{
    //Smaple Data 03 17 80 00 02 05 20 01 12 03 04

    int numAlarms = data.at(5);
    auto config = AmazfishConfig::instance();

    for (int i = 0; i < numAlarms; i++) {
        char alarm_data = data.at(6 + i);
        int index = alarm_data & 0xf;
        bool enabled = (alarm_data & 0x10) == 0x10;
        qDebug() << "alarm " << index << " is enabled:" << enabled;
        config->setAlarmEnabled(i + 1, enabled);
    }
}

void MiBandService::characteristicRead(const QString &characteristic, const QByteArray &value)
{
    qDebug() << "Read:" << characteristic << value;

    if (characteristic == UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO) {
        qDebug() << "...Got battery info";
        m_batteryInfo.setData(value);
        emit informationChanged(AbstractDevice::INFO_BATTERY, QString::number(m_batteryInfo.currentChargeLevelPercent()));
    }
}

void MiBandService::requestGPSVersion()
{
    writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(1, COMMAND_REQUEST_GPS_VERSION));
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
    timeBytes += char(0); //could be adjust reason or DST offset. In both cases 0 is safe.

    // calculate timezone
    int offsetInSec = now.offsetFromUtc();
    timeBytes += char(offsetInSec/(15*60));

    qDebug() << "setting time to:" << now << ", tz: " << offsetInSec << timeBytes.toHex();
    writeValue(UUID_CHARACTERISTIC_MIBAND_CURRENT_TIME, timeBytes);
}

void MiBandService::setLanguage()
{
    auto format = AmazfishConfig::instance()->deviceLanguage();

    qDebug() << "Setting language to " << format;

    QByteArray lang;
    lang += QByteArray(1, ENDPOINT_DISPLAY);
    lang += QByteArray(1, COMMAND_SET_LANGUAGE);
    lang += char(0);
    //"zh_CN", "zh_TW", "en_US", "es_ES", "ru_RU", "de_DE", "it_IT", "fr_FR", "tr_TR"

    switch (format) {
    case AmazfishConfig::DeviceLanguageEsEs:
        lang += "es_ES";
        break;
    case AmazfishConfig::DeviceLanguageZhCn:
        lang += "zh_CN";
        break;
    case AmazfishConfig::DeviceLanguageZhTw:
        lang += "zh_TW";
        break;
    case AmazfishConfig::DeviceLanguageRuRu:
        lang += "ru_RU";
        break;
    case AmazfishConfig::DeviceLanguageDeDe:
        lang += "de_DE";
        break;
    case AmazfishConfig::DeviceLanguageItIt:
        lang += "it_IT";
        break;
    case AmazfishConfig::DeviceLanguageFrFr:
        lang += "fr_FR";
        break;
    case AmazfishConfig::DeviceLanguageTrTr:
        lang += "tr_TR";
        break;
    default:
        lang += "en_US";
    }

    writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, lang);
}

void MiBandService::setDateDisplay()
{
    auto format = AmazfishConfig::instance()->deviceDateFormat();

    qDebug() << "Setting date display to " << format;
    switch (format) {
    case AmazfishConfig::DeviceDateFormatTime:
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, UCHARARR_TO_BYTEARRAY(DATEFORMAT_TIME));
        break;
    case AmazfishConfig::DeviceDateFormatDateTime:
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, UCHARARR_TO_BYTEARRAY(DATEFORMAT_DATETIME));
        break;
    }

}
void MiBandService::setTimeFormat()
{
    auto format = AmazfishConfig::instance()->deviceTimeFormat();

    qDebug() << "Setting time format to " << format;
    switch (format) {
    case AmazfishConfig::DeviceTimeFormat24H:
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, UCHARARR_TO_BYTEARRAY(DATEFORMAT_TIME_24_HOURS));
        break;
    case AmazfishConfig::DeviceTimeFormat12H:
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, UCHARARR_TO_BYTEARRAY(DATEFORMAT_TIME_12_HOURS));
        break;
    }
}
void MiBandService::setUserInfo()
{
    QByteArray userInfo;
    auto config = AmazfishConfig::instance();
    auto profileName = config->profileName();
    uint id = qHash(profileName);
    auto gender = config->profileGender();
    auto height = config->profileHeight();
    auto weight = config->profileWeight();
    auto dob = config->profileDOB().date();

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
    auto format = AmazfishConfig::instance()->deviceDistanceUnit();

    qDebug() << "Setting distance unit to " << format;
    switch (format) {
    case AmazfishConfig::DeviceDistanceUnitMetric:
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, UCHARARR_TO_BYTEARRAY(COMMAND_DISTANCE_UNIT_METRIC));
        break;
    case AmazfishConfig::DeviceDistanceUnitImperial:
        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, UCHARARR_TO_BYTEARRAY(COMMAND_DISTANCE_UNIT_IMPERIAL));
        break;
    }

}

//Only use during device init
void MiBandService::setWearLocation()
{
    auto location = AmazfishConfig::instance()->profileWearLocation();

    qDebug() << "Setting wear location to " << location;
    switch (location) {
    case AmazfishConfig::WearLocationLeftWrist:
        writeValue(UUID_CHARACTERISTIC_MIBAND_USER_SETTINGS, UCHARARR_TO_BYTEARRAY(WEAR_LOCATION_LEFT_WRIST));
        break;
    case AmazfishConfig::WearLocationRightWrist:
        writeValue(UUID_CHARACTERISTIC_MIBAND_USER_SETTINGS, UCHARARR_TO_BYTEARRAY(WEAR_LOCATION_RIGHT_WRIST));
        break;
    }
}

void MiBandService::setFitnessGoal()
{
    auto goal = AmazfishConfig::instance()->profileFitnessGoal();

    QByteArray cmd = UCHARARR_TO_BYTEARRAY(COMMAND_SET_FITNESS_GOAL_START);
    cmd += TypeConversion::fromInt16(goal);
    cmd += UCHARARR_TO_BYTEARRAY(COMMAND_SET_FITNESS_GOAL_END);

    writeValue(UUID_CHARACTERISTIC_MIBAND_USER_SETTINGS, cmd);
}

void MiBandService::setAlertFitnessGoal()
{
    auto value = AmazfishConfig::instance()->profileAlertFitnessGoal()
            ? COMMAND_ENABLE_GOAL_NOTIFICATION
            : COMMAND_DISABLE_GOAL_NOTIFICATION;

    writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, UCHARARR_TO_BYTEARRAY(value));
}


void MiBandService::setEnableDisplayOnLiftWrist()
{
    auto value = AmazfishConfig::instance()->profileDisplayOnLiftWrist()
            ? COMMAND_ENABLE_DISPLAY_ON_LIFT_WRIST
            : COMMAND_DISABLE_DISPLAY_ON_LIFT_WRIST;

    writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, UCHARARR_TO_BYTEARRAY(value));
}

void MiBandService::setDisplayItems()
{
    uint8_t items1 = 0x01; //Always display clock
    uint8_t items2 = 0x10;

    HuamiDevice *device = qobject_cast<HuamiDevice*>(parent());

    if (device && device->deviceType() == "amazfitbip" && device->softwareRevision() < "V0.1.1.14") { //Lexical string comparison should be fine here
        emit message(tr("Firmware is too old to set display items, V0.1.1.14 is required"));
        return;
    }

    auto items = AmazfishConfig::instance()->deviceDisplayItems().split(",");

    if (items.contains("status")) {
        items1 |= 0x02;
    }
    if (items.contains("activity")) {
        items1 |= 0x04;
    }
    if (items.contains("weather")) {
        items1 |= 0x08;
    }
    if (items.contains("alarm")) {
        items1 |= 0x10;
    }
    if (items.contains("timer")) {
        items1 |= 0x20;
    }
    if (items.contains("compass")) {
        items1 |= 0x40;
    }
    if (items.contains("settings")) {
        items1 |= 0x80;
    }
    if (items.contains("alipay")) {
        items2 |= 0x01;
    }

    QByteArray cmd = UCHARARR_TO_BYTEARRAY(COMMAND_CHANGE_SCREENS);
    cmd[1] = items1;
    cmd[2] = items2;

    qDebug() << "Setting display items to:" << cmd;
    writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, cmd);

    //Shortcuts
    auto config = AmazfishConfig::instance();
    auto sw = config->deviceDisplayWeatherShortcut();
    auto sa = config->deviceDisplayAliPayShortcut();

    QByteArray shortcuts;
    shortcuts += 0x10;
    shortcuts += ((sa || sw) ? 0x80 : 0x00);
    shortcuts += (sw ? 0x02 : 0x01);
    shortcuts += ((sa && sw) ? 0x81 : 0x01);
    shortcuts += 0x01;

    qDebug() << "Setting shortcuts to:" << shortcuts;

    writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, shortcuts);
}

void MiBandService::setDisplayItemsOld(QMap<QString, uint8_t> keyPosMap)
{
    HuamiDevice *device = qobject_cast<HuamiDevice*>(parent());

    if (device && device->deviceType() == "amazfitbip" && device->softwareRevision() < "V0.1.1.14") { //Lexical string comparison should be fine here
        emit message(tr("Firmware is too old to set display items, V0.1.1.14 is required"));
        return;
    }

    auto items = AmazfishConfig::instance()->deviceDisplayItems().split(",");

    QByteArray command(keyPosMap.size() + 4, 0x00);
    command[0] = ENDPOINT_DISPLAY_ITEMS;
    uint8_t index = 1;
    uint16_t enabled_mask = 0x01; //clock

    // it seem that we first have to put all ENABLED items into the array, oder does matter
    foreach (const QString &key, items) {
        uint8_t id = keyPosMap[key];
        if (id != 0) {
            qDebug() << "enabling:" << key;
            enabled_mask |= (1 << id);
            command[3 + id] = index++;
        }
    }

    qDebug() << command.toHex();

    // And then all DISABLED ones, order does not matter
    for (int i = 0; i < keyPosMap.size(); i++) {
        QString key = keyPosMap.key(i);
        int id = keyPosMap.value(key);
        if (!items.contains(key)) {
            command[3 + id] = index++;
        }
    }

    command[1] = (uint8_t) (enabled_mask & 0xff);
    command[2] = (uint8_t) ((enabled_mask >> 8 & 0xff));

    writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, command);
}

void MiBandService::setDisplayItemsNew()
{
    QStringList enabledList;
    QStringList allList;
    QByteArray command;
    uint8_t menuType = 0xff; //0xfd for shortcuts

    enabledList = AmazfishConfig::instance()->deviceDisplayItems().split(",");
    qDebug() << "Enabled List:" << enabledList;

    command += QByteArray(1, (char)0x1e);
    // it seem that we first have to put all ENABLED items into the array, oder does matter
    int index = 0;

    command += QByteArray(1, (char)index++);
    command += QByteArray(1, (char)0x00);
    command += QByteArray(1, (char)menuType);
    command += QByteArray(1, (char)0x12);

    for (QString &key : enabledList) {
        uint8_t id = displayItemsIdMap[key];
        if (id != 0) {
            command += QByteArray(1, (char)index++);
            command += QByteArray(1, (char)0x00);
            command += QByteArray(1, (char)menuType);
            command += QByteArray(1, (char)id);
        }
    }
    // And then all DISABLED ones, order does not matter
    AbstractDevice *device = qobject_cast<AbstractDevice*>(parent());
    if (device) {
        allList = device->supportedDisplayItems();
        for (QString &key : allList) {
            uint8_t id = displayItemsIdMap[key];

            if (!enabledList.contains(key)) {
                command += QByteArray(1, (char)index++);
                command += QByteArray(1, (char)0x01);
                command += QByteArray(1, (char)menuType);
                command += QByteArray(1, (char)id);
            }
        }
    }

    writeChunked(MiBandService::UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 2, command);
}

void MiBandService::setDoNotDisturb()
{

}

void MiBandService::setRotateWristToSwitchInfo(bool enable)
{
    qDebug() << "Setting rotate write to " << enable;

    auto value = enable
            ? COMMAND_ENABLE_ROTATE_WRIST_TO_SWITCH_INFO
            : COMMAND_DISABLE_ROTATE_WRIST_TO_SWITCH_INFO;

    writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, UCHARARR_TO_BYTEARRAY(value));
}

void MiBandService::setDisplayCaller()
{
    writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, UCHARARR_TO_BYTEARRAY(COMMAND_ENABLE_DISPLAY_CALLER));
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
        m_operationTimeout->start(10000);
    } else {
        emit message(tr("An operation is currently running, please try later"));
    }
}

void MiBandService::fetchActivityData()
{
    if (!m_activityFetchOperation && m_operationRunning == 0) {
        m_operationRunning = 2;
        int sampleSize = 4;
        HuamiDevice *device = qobject_cast<HuamiDevice*>(parent());

        if (device) {
            sampleSize = device->activitySampleSize();
        }

        qDebug() << "Sample size is " << sampleSize;

        m_activityFetchOperation = new ActivityFetchOperation(this, m_conn, sampleSize);
        m_activityFetchOperation->start();
        emit operationRunningChanged();
        m_operationTimeout->start(10000);
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
        m_operationTimeout->start(10000);
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
    auto config = AmazfishConfig::instance();
    for (int i = 0, n = 1; i < 5; ++i, ++n) {
        int base       = config->alarmEnabled(n)    ? 128 : 0;
        int repeatMask = config->alarmRepeatMask(n) == 0 ? 128 : config->alarmRepeatMask(n);

        QByteArray cmd;
        cmd += 0x02;
        cmd += (base + i);
        cmd += config->alarmHour(n);
        cmd += config->alarmMinute(n);
        cmd += repeatMask;

        writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, cmd);
    }
}

void MiBandService::requestAlarms()
{
    writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, QByteArray(1,COMMAND_REQUEST_ALARMS));
}

void MiBandService::setDisconnectNotification()
{
    auto value = AmazfishConfig::instance()->deviceDisconnectNotification()
            ? COMMAND_ENABLE_DISCONNECT_NOTIFICATION
            : COMMAND_DISABLE_DISCONNECT_NOTIFICATION;

    writeValue(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION, UCHARARR_TO_BYTEARRAY(value));
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
    m_operationTimeout->stop();
}

void MiBandService::sendWeather(const CurrentWeather *weather, bool supportsConditionString)
{
    HuamiDevice *device = qobject_cast<HuamiDevice*>(parent());

    if (!device || device->softwareRevision() > "V0.0.8.74") { //Lexical string comparison should be fine here
        supportsConditionString = true;
    }

    char condition = HuamiWeatherCondition::mapToAmazfitBipWeatherCode(weather->weatherCode());

    //int tz_offset_hours = SimpleTimeZone.getDefault().getOffset(weather->timestamp * 1000L) / (1000 * 60 * 60);
    int tz_offset_hours = 0; //!TODO work out what to do with this

    qDebug() << "Sending condition";
    QByteArray buf;
    QBuffer buffer(&buf);
    buffer.open(QIODevice::WriteOnly);

    char temp = weather->temperature() - 273.15;
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
    buffer.putChar((char) (weather->maxTemperature() - 273.15));
    buffer.putChar((char) (weather->minTemperature() - 273.15));
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
        buffer.putChar((char) (fc.maxTemperature() - 273.15));
        buffer.putChar((char) (fc.minTemperature() - 273.15));

        if (supportsConditionString) {
            buffer.write(fc.description().toLatin1());
            buffer.putChar((char)0x00);
        }
    }
    writeChunked(UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 1, buf);

    //Send forecast location
    QByteArray location;
    location += (char)(0x08);
    location += weather->city()->name().toLocal8Bit();
    location += (char)(0x00);
    writeChunked(UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 1, location);
}

void MiBandService::writeChunked(const QString &characteristic, int type, const QByteArray &value) {
    int MAX_CHUNKLENGTH = 17;
    int remaining = value.length();
    char count = 0;

    qDebug() << "Writing chunked: " << value;
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

void MiBandService::sendAlert(const QString &sender, const QString &subject, const QString &message)
{
    qDebug() << "Alert:" << sender << subject << message;

    if (message.isEmpty()) {
        return;
    }

    //Prefix
    int category = 0xfa; //Custom Huami icon
    int icon = AlertNotificationService::mapSenderToIcon(sender);

    if (sender == "Messages") { //SMS must use category, not icon
        category = AlertNotificationService::AlertCategory::SMS;
    }

    if (icon == AlertNotificationService::HuamiIcon::EMAIL) { //Email icon doesnt work, so use category
        category = AlertNotificationService::AlertCategory::Email;
    }

    QByteArray prefix = QByteArray(1, category) + QByteArray(4, 0x00) + QByteArray(1, 0x01); //1 alert
    prefix += QByteArray(1, AlertNotificationService::mapSenderToIcon(sender));

    //Message
    QByteArray msg = sender.left(32).toUtf8() + QByteArray(1, 0x00); //Null char indicates end of first line
    if (!subject.isEmpty()) {
        msg += subject.left(128).toUtf8() + QByteArray(2, 0x0a);
    }

    if (!message.isEmpty()) {
        msg += message.left(128).toUtf8();
    }

    //Suffix
    QByteArray suffix;
    suffix += (char)0x00;
    suffix += (char)' ';
    suffix += (char)0x00;

    writeChunked(UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 0, prefix + msg + suffix);
}

void MiBandService::setMusicStatus(bool playing, const QString &artist, const QString &album, const QString &track, int duration, int position)
{
    QByteArray cmd;

    char flags = 0x00;
    flags |= 0x01;

    if (artist.length() > 0) {
        flags |= 0x02;
    }
    if (album.length() > 0) {
        flags |= 0x04;
    }
    if (track.length() > 0) {
        flags |= 0x08;
    }
    if (duration != 0) {
        flags |= 0x10;
    }

    char state = playing ? 0x01 : 0x00; //Not playing
    cmd += flags;
    cmd += state;
    cmd += (char)0x00;

    //Position
    cmd += TypeConversion::fromInt16(position);

    if (artist.length() > 0) {
        cmd += artist.toLocal8Bit();
        cmd += char(0x00);
    }
    if (album.length() > 0) {
        cmd += album.toLocal8Bit();
        cmd += char(0x00);
    }
    if (track.length() > 0) {
        cmd += track.toLocal8Bit();
        cmd += char(0x00);
    }
    if (duration != 0) {
        cmd += TypeConversion::fromInt32(duration);
    }
    writeChunked(MiBandService::UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 3, cmd);
}
