#include "dk08nusservice.h"
#include "typeconversion.h"
#include "amazfishconfig.h"

/*
 * The Nordic UART Service is a simple GATT-based service with TX and RX characteristics.
 * Data received from the peer is passed to the application, and the data received
 * from the application of this service is sent to the peer as Handle Value
 * Notifications.
 */


const char* DK08NUSService::UUID_SERVICE_NUS  = "f000efe0-0451-4000-0000-00000000b000"; // BLE_UUID_NUS_SERVICE  Nordic UART Service
const char* DK08NUSService::UUID_CHARACTERISTIC_NUS_WRITE = "f000efe1-0451-4000-0000-00000000b000";
const char* DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY ="f000efe3-0451-4000-0000-00000000b000";

DK08NUSService::DK08NUSService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_NUS, path, parent)
{
    qDebug() << Q_FUNC_INFO;

    connect(this, &QBLEService::characteristicRead, this, &DK08NUSService::characteristicRead);
    enableNotification(UUID_CHARACTERISTIC_NUS_NOTIFY);
}

void DK08NUSService::refreshInformation() {
    getBattery();
    getHrInfo(InfoType::Current);
    getFirmware();
}

void DK08NUSService::characteristicRead(const QString &characteristic, const QByteArray &value)
{
    if (characteristic == UUID_CHARACTERISTIC_NUS_NOTIFY) {
        // bool valid = ((value.length() == 20) && ((value[0] & 0x01) == 0));
        if (value.length() != 20) {
            qWarning () << Q_FUNC_INFO << "Invalid length" << value.length();
            return;
        }

        qDebug() << Q_FUNC_INFO << value.toHex() << (NUSCallType)value[0] << "=================";

        switch(value[0]) {
        case NUSCallType::Time:
            // smartwatch confirms that time was set or send own time information for compensation of time drift?
            break;
        case NUSCallType::FirmwareVersion: // 0x0f
            parseFirmware(value.mid(1));
            break; // NUSCallType::FirmwareVersion
        case NUSCallType::Sport:
            parseStep(value.mid(1));
            break;
        case NUSCallType::ParseSport:
            parseSport(value.mid(1));
            break;
        case NUSCallType::HeartRate:
            parseHr(value.mid(1));
            break;
        case NUSCallType::SleepStats:
            parseSleepStats(value.mid(1));
            break;
        case NUSCallType::StepSection:
            parseStepSection(value.mid(1));
            break;
        case NUSCallType::SleepInfo:
            parseSleep(value.mid(1));
            break;
        case NUSCallType::User:
            parseUser(value.mid(1));
            break;
        default:
            qWarning() << "default handler for " << (NUSCallType)value[0] << QString("0x%1").arg((u_int8_t)value[0], 0, 16);
            break;
        }
    } else {
        qWarning() << Q_FUNC_INFO << "Unknown characteristics" << characteristic;
    }
}

void DK08NUSService::setTime() {
    QByteArray msg(20, 0);

    QDateTime now = QDateTime::currentDateTime();

    msg[0]  = NUSCallType::Command;
    msg[1]  = NUSCallType::Time;
    msg[2]  = twoDigitsToBCD(now.date().year() % 1000);
    msg[3]  = twoDigitsToBCD(now.date().month());
    msg[4]  = twoDigitsToBCD(now.date().day());
    msg[5]  = twoDigitsToBCD(now.time().hour());
    msg[6]  = twoDigitsToBCD(now.time().minute());
    msg[7]  = twoDigitsToBCD(now.time().second());
    msg[8]  = twoDigitsToBCD(now.date().dayOfWeek());
    msg[19] = getCrc(msg.left(19));

    writeAsync(UUID_CHARACTERISTIC_NUS_WRITE, msg);

    qDebug() << Q_FUNC_INFO << msg.toHex();
}

void DK08NUSService::getFirmware() {
    QByteArray msg(20, 0);
    msg[0] = NUSCallType::Command;
    msg[1] = NUSCallType::FirmwareVersion;
    msg[2] = 0x05;
    msg[19] = getCrc(msg.left(19));
    writeValue(UUID_CHARACTERISTIC_NUS_WRITE, msg);
    readAsync(UUID_CHARACTERISTIC_NUS_NOTIFY);
}

void DK08NUSService::parseFirmware(QByteArray value) {
    if (static_cast<quint8>(value[0]) == 0x05) {
        QString firmwareVersion = QString("%1.%2.%3")
        .arg(static_cast<unsigned char>(value[6]))
            .arg(static_cast<unsigned char>(value[7]))
            .arg(static_cast<unsigned char>(value[8]));
        emit informationChanged(AbstractDevice::INFO_FW_REVISION, firmwareVersion);

        if (value.length() > 11 && static_cast<quint8>(value[9]) == 0x55) {
            QString typeStr = QString("%1%2")
            .arg( bcdToTwoDigits(value[10]) )
                .arg( bcdToTwoDigits(value[11]) );
            emit informationChanged(AbstractDevice::INFO_MODEL, typeStr);
        }

    } else if (static_cast<quint8>(value[0]) == 0x06) {

        int battery = static_cast<unsigned char>(value[6]);
        int batteryState = static_cast<unsigned char>(value[7]);

        qDebug() << "Battery:" << battery;

        emit informationChanged(AbstractDevice::INFO_BATTERY, QString::number(batteryState));

    } else {
        qDebug() << "NUSCallType::FirmwareVersion" << NUSCallType::FirmwareVersion << value[0];
    }


}


void DK08NUSService::getBattery() {
    QByteArray msg(20, 0);
    msg[0] = NUSCallType::Command;
    msg[1] = NUSCallType::FirmwareVersion;
    msg[2] = 0x06;
    msg[19] = getCrc(msg.left(19));
    writeValue(UUID_CHARACTERISTIC_NUS_WRITE, msg);
    readAsync(UUID_CHARACTERISTIC_NUS_NOTIFY);
}

void DK08NUSService::getHrInfo(InfoType type) {
    qDebug() << Q_FUNC_INFO;
    QByteArray msg(20, 0);
    msg[0] = NUSCallType::Command;
    msg[1] = NUSCallType::HeartRate;
    msg[2] = type;
    msg[19] = getCrc(msg.left(19));
    writeValue(UUID_CHARACTERISTIC_NUS_WRITE, msg);
    readAsync(UUID_CHARACTERISTIC_NUS_NOTIFY);
}

void DK08NUSService::parseHr(QByteArray value) {
    int type = value[0]; // See InfoType
    int hr = static_cast<quint8>(value[11]);

    int hrLength = TypeConversion::toUint16(value[2], value[1]);
    int hrNum = TypeConversion::toUint16(value[4], value[3]);

    QDateTime hrDate = bcdToDateTime(value.mid(5, 5));

    qDebug() << "Type:" << (InfoType)type;
    qDebug() << "Heart Rate:" << hr;
    qDebug() << "HR Length:" << hrLength;
    qDebug() << "HR Num:" << hrNum;
    qDebug() << "HR Date:" << hrDate;

    if (type == InfoType::Current) {
        informationChanged(AbstractDevice::Info::INFO_HEARTRATE, QString::number(hr));
    } else if (type == InfoType::Notify) {
        qDebug() << "hr notify <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
    } else if (type == InfoType::History) {
        qDebug() << "hr history <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
    }
}

void DK08NUSService::getSportInfo(InfoType type) {
    QByteArray msg(20, 0);
    msg[0] = NUSCallType::Command;
    msg[1] = NUSCallType::Sport;
    const uint8_t mapping[] = { /* Current */ 0x07, /* History */ 0xC0, /* HistoryNum */0x80 };
    msg[2] = (type >= 0 && type < 3) ? mapping[type] : 0x00;
    msg[19] = getCrc(msg.left(19));
    writeValue(UUID_CHARACTERISTIC_NUS_WRITE, msg);
    readAsync(UUID_CHARACTERISTIC_NUS_NOTIFY);
}

void DK08NUSService::parseStep(QByteArray value) {
    quint8 en = static_cast<quint8>(value[0]); // Command identifier

    qDebug() << Q_FUNC_INFO << QString("0x%1").arg((u_int8_t)value[0], 0, 16);
    if (en == 0x80) {
        int hisLength = static_cast<quint8>(value[1]);
        qDebug() << "Step history length:" << hisLength;
    } else if (en == 0xC0) {
        int hisLength = static_cast<quint8>(value[1]);
        int hisCount = static_cast<quint8>(value[2]);

        QDate day = QDate(bcdToTwoDigits(value[3]) + 2000, bcdToTwoDigits(value[4]), bcdToTwoDigits(value[5]));
        int num = TypeConversion::toUint32(value[8], value[9], value[10], 0);

        qDebug() << "Step history data:";
        qDebug() << "  History length:" << hisLength;
        qDebug() << "  History count:" << hisCount;
        qDebug() << "  Date:" << day;
        qDebug() << "  Step count:" << num;
    } else {
        int count = TypeConversion::toUint32(value[3], value[4], value[5], 0);
        int mileage = TypeConversion::toUint32(value[6], value[7], value[8], 0);
        int calorie = TypeConversion::toUint32(value[9], value[10], value[11], 0);

        qDebug() << "Step data:";
        qDebug() << "  Step count:" << count;
        qDebug() << "  Step mileage:" << mileage;
        qDebug() << "  Step calorie:" << calorie;
    }

    if (en == 0x07) {
        qDebug() << "enable notify";
    }
}


void DK08NUSService::parseSport(QByteArray value) {
    qDebug() << Q_FUNC_INFO << value.toHex();

    int mode = (value[0] >> 4) & 0x0F; // Get sport mode (upper 4 bits of value[0])
    int en = value[0] & 0x0F;          // Get data model (lower 4 bits of value[0])
    int ss = value[1] & 0x03;          // Get walking status (bits 0-1 of value[1])

    // Extract package number info (3 bytes)
    int packageLength = ((value[2] & 0x0F) << 8) | (value[3] & 0xFF);
    int packageNum = ((value[4] & 0xFF) << 8) | (value[5] & 0xFF);

    // Extract sports data (7 bytes)
    int step = ((value[6] & 0x1F) << 12) | ((value[7] & 0xFF) << 4) | ((value[8] >> 4) & 0x0F);
    int ka = ((value[8] & 0x0F) << 13) | ((value[9] & 0xFF) << 5) | ((value[10] >> 3) & 0x1F);
    int mi = ((value[10] & 0x07) << 14) | ((value[11] & 0xFF) << 6) | ((value[12] >> 2) & 0x3F);

    // Extract timestamp (4 bytes)
    qint64 date = (static_cast<quint32>(value[13]) << 24) |
                  (static_cast<quint32>(value[14]) << 16) |
                  (static_cast<quint32>(value[15]) << 8) |
                  (static_cast<quint32>(value[16]));
    date *= 1000; // Convert to milliseconds
    // date -= QTimeZone::systemTimeZone().standardTimeOffset(); // Adjust for timezone

    // Extract time (2 bytes)
    int time = (static_cast<quint16>(value[17]) << 8) | static_cast<quint16>(value[18]);

    qDebug() << "Mode:" << mode << "En:" << en << "SS:" << ss;
    qDebug() << "Package Length:" << packageLength << "Package Num:" << packageNum;
    qDebug() << "Steps:" << step << "Calories:" << ka << "Distance:" << mi;
    qDebug() << "Date:" << date << "Time:" << time;

    if (en == InfoType::Notify || en == InfoType::History) { // Segmented historical data/reporting
        qDebug() << "Step Type:" << (ss + 2) << "Sport Mode:" << mode;
    } else if (en == InfoType::HistoryNum) { // Historical sport data count
        qDebug() << "Historical Data Count:" << packageLength;
    }


    if (en == InfoType::Notify) {
        qDebug() << "sport notify <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
    } else if (en == InfoType::History) {
        qDebug() << "sport history <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
    }

}


void DK08NUSService::getSleepStats() {
    QByteArray msg(20, 0);
    msg[0] = NUSCallType::Command;
    msg[1] = NUSCallType::SleepStats;
    msg[2] = 0x01;
    msg[19] = getCrc(msg.left(19));
    writeValue(UUID_CHARACTERISTIC_NUS_WRITE, msg);
    readAsync(UUID_CHARACTERISTIC_NUS_NOTIFY);

}

void DK08NUSService::parseSleepStats(QByteArray value) {

    qint64 dateStart = TypeConversion::toUint32(value[4], value[3], value[2], value[1]);
    qint64 dateEnd = TypeConversion::toUint32(value[8], value[7], value[6], value[5]);
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    QDateTime qDateStart = QDateTime::fromSecsSinceEpoch(dateStart);
    QDateTime qDateEnd = QDateTime::fromSecsSinceEpoch(dateEnd);
#else
    QDateTime qDateStart = QDateTime::fromTime_t(dateStart);
    QDateTime qDateEnd = QDateTime::fromTime_t(dateEnd);
#endif

    int sum = TypeConversion::toUint16(value[10], value[9]);
    int deep = TypeConversion::toUint16(value[12], value[11]);
    int light = TypeConversion::toUint16(value[14], value[13]);
    int rem = TypeConversion::toUint16(value[16], value[15]);

    qDebug() << "Sleep Stats:";
    qDebug() << "  Sum:" << sum;
    qDebug() << "  Deep sleep:" << deep;
    qDebug() << "  Light sleep:" << light;
    qDebug() << "  REM sleep:" << rem;
    qDebug() << "  Date Start:" <<  qDateStart;
    qDebug() << "  Date End:" << qDateEnd;


}

void DK08NUSService::getStepSection(InfoType type) {
    QByteArray msg(20, 0);
    msg[0] = NUSCallType::Command;
    msg[1] = NUSCallType::StepSection;

    const uint8_t mapping[] = { /* Current */ 0x00, /* History */ 0x04, /* HistoryNum */ 0x02 };
    msg[2] = (type >= 0 && type < 3) ? mapping[type] : 0x00;
    msg[19] = getCrc(msg.left(19));
    writeValue(UUID_CHARACTERISTIC_NUS_WRITE, msg);
    readAsync(UUID_CHARACTERISTIC_NUS_NOTIFY);
}

void DK08NUSService::parseStepSection(QByteArray value) {

    //       [0] [1] [2] [3]
    // bytes  12  34  56  78
    // en     12             -> 12
    // ah          4  5      -> 45
    // ch              6  78 -> 678
    quint8 en = static_cast<quint8>(value[0]);
    int ah = (value[1] << 4) | (value[2] >> 4);
    int ch = ((value[2] & 0x0f) << 8) | (value[3] & 0xff);

    qDebug() << "en" << en << "(en & 0x05)" << (en & 0x05) << QString::number(en, 2).rightJustified(8, '0');
    qint64 date = 0;
    QDateTime qDate;

    if ((en & 0x05) > 0) {  // "00000101" = contain 100 or 001 = Segment history data / Report
        uint step = TypeConversion::toUint32(value[6], value[5], value[4], 0);
        uint calorie = TypeConversion::toUint32(value[9], value[8], value[7], 0);
        uint mileage = TypeConversion::toUint32(value[12], value[11], value[10], 0);
        date = TypeConversion::toUint32(value[16], value[15], value[14], value[13]);
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
        qDate = QDateTime::fromSecsSinceEpoch(date);
#else
        qDate = QDateTime::fromTime_t(date);
#endif
        int time = static_cast<quint8>(value[17]);

        // Directly log the parsed values
        qDebug() << "Segmented Step Data:";
        qDebug() << "  Step count:" << step;
        qDebug() << "  Calorie:" << calorie;
        qDebug() << "  Mileage:" << mileage;
        qDebug() << "  History Length:" << ah;
        qDebug() << "  History Count:" << ch;
        qDebug() << "  Date:" << qDate;
        qDebug() << "  Step Time:" << time;
        qDebug() << "  Step Type:" << 1;
    } else if ((en & 0x02) > 0) {  // Segment history count
        qDebug() << "Segmented History Count:";
        qDebug() << "  History Length:" << ah;
    }

    if (en == 1) {
        // notify
        qDebug() << "Notify >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";

    } else if ((en & 0x02) > 0) {
        // history
        qDebug() << "history >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
        if (qDate.isValid()) {
            qDebug() << "next! ===========================";
            readAsync(UUID_CHARACTERISTIC_NUS_NOTIFY);
        } else {
            qWarning() << "Invalid not reading";
        }
    }

}

void DK08NUSService::getSleep(InfoType type) {
    QByteArray msg(20, 0);
    msg[0] = NUSCallType::Command;
    msg[1] = NUSCallType::SleepInfo;
    msg[2] = type;
    msg[19] = getCrc(msg.left(19));
    writeValue(UUID_CHARACTERISTIC_NUS_WRITE, msg);
    readAsync(UUID_CHARACTERISTIC_NUS_NOTIFY);
}

QDateTime DK08NUSService::bcdToDateTime(QByteArray bytes) {
    int year = bcdToTwoDigits(bytes[0]) + 2000;
    int month = bcdToTwoDigits(bytes[1]);
    int day = bcdToTwoDigits(bytes[2]);
    int hour = bcdToTwoDigits(bytes[3]);
    int minute = bcdToTwoDigits(bytes[4]);

    return QDateTime(QDate(year, month, day), QTime(hour, minute));
}

void DK08NUSService::parseSleep(QByteArray value) {
    int type = static_cast<quint8>(value[0]);
    int sleepLength = static_cast<quint8>(value[1]); // Sleep history total count
    int sleepNum = static_cast<quint8>(value[2]);    // Sleep packet number
    QDateTime startTime = bcdToDateTime(value.mid(3, 5));
    QDateTime endTime = bcdToDateTime(value.mid(8, 5));

    int time = TypeConversion::toUint16(value[14], value[13]);
    int sleepType = static_cast<quint8>(value[15]);

    int deep = 0, light = 0, awake = 0;

    switch (sleepType) {
    case SleepType::Deep:
        deep = time;
        break;
    case SleepType::Light:
        light = time;
        break;
    case SleepType::Awake:
        awake = time;
        break;
    }

    qDebug() << "starTime" << startTime;
    qDebug() << "endTime" << endTime;
    qDebug() << "sleep length" << sleepLength;
    qDebug() << "sleep num" << sleepNum;
    qDebug() << "type" << (SleepType)sleepType << (int)sleepType;
    qDebug() << "deep" << deep << "light" << light << "awake" << awake;

    if (type == InfoType::History) {
        qDebug() << "history >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
        // readAsync(UUID_CHARACTERISTIC_NUS_NOTIFY);
    } else if (type == InfoType::Notify) {
        qDebug() << "notify >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
    }

}

void DK08NUSService::parseUser(QByteArray value) {
    uint weight = static_cast<uint8_t>(value[0]);
    uint height = static_cast<uint8_t>(value[1]);
    qDebug() << Q_FUNC_INFO << "weight " << weight << "height " << height ;
}


void DK08NUSService::confirmSportHistoryData(int num) {
    qDebug() << Q_FUNC_INFO;
    QByteArray msg(20, 0);
    msg[0] = NUSCallType::Command;
    msg[1] = NUSCallType::Sport;
    msg[2] = 0xC0;
    msg[3] = (num >> 8) & 0xff;
    msg[4] = num & 0xff;
    msg[19] = getCrc(msg.left(19));
    writeValue(UUID_CHARACTERISTIC_NUS_WRITE, msg);
    readAsync(UUID_CHARACTERISTIC_NUS_NOTIFY);
}

void DK08NUSService::setInfoAlert(AlertType t) {
    qDebug() << Q_FUNC_INFO;
    QByteArray msg(20, 0);
    msg[0] = NUSCallType::Command;
    msg[1] = NUSCallType::InfoAlert;
    msg[2] = t;
    msg[19] = getCrc(msg.left(19));
    writeValue(UUID_CHARACTERISTIC_NUS_WRITE, msg);
    readAsync(UUID_CHARACTERISTIC_NUS_NOTIFY);
}


QByteArray DK08NUSService::toUtf16Hex(QString str) {
    QByteArray utf16BE;
    const ushort *utf16 = str.utf16();  // Get UTF-16 data

    for (int i = 0; i < str.length(); ++i) {
        ushort ch = utf16[i];
        utf16BE.append(static_cast<char>((ch >> 8) & 0xFF)); // High byte first
        utf16BE.append(static_cast<char>(ch & 0xFF));        // Low byte second
    }

    return utf16BE;
}

void DK08NUSService::setPhoneAlert(const QString &caller) {
    qDebug() << Q_FUNC_INFO;
    QByteArray msg(20, 0);
    msg[0] = NUSCallType::Command;
    msg[1] = NUSCallType::InfoAlert;
    msg[2] = AlertType::PhoneAlert;
    msg[3] = 0x01; // 1 = caller name, 2 = phone number
    QByteArray callerBytes = toUtf16Hex(caller).left(15);
    msg[4] = callerBytes.length();
    msg.replace(5, callerBytes.length(), callerBytes);
    msg[19] = getCrc(msg.left(19));
    writeValue(UUID_CHARACTERISTIC_NUS_WRITE, msg);
    readAsync(UUID_CHARACTERISTIC_NUS_NOTIFY);
}

void DK08NUSService::setSMSAlertName(int id, AlertType alertType, const QString &name) {
    qDebug() << Q_FUNC_INFO << id << name;

        QByteArray bytes_one = toUtf16Hex(name).left(12);

        QByteArray msg(20, 0);
        msg[0] = NUSCallType::Command;
        msg[1] = NUSCallType::InfoAlert;
        msg[2] = alertType;
        msg[3] = (id | 1 << 7 | 1 << 6);
        // 6th is true when sending name, false phone number
        // 7th is true when sending name, false when context
        msg[4] = bytes_one.length();
        msg[5] = 0x03; // 0x03 unicode, 0x00 Ascii BCD
        msg.replace(6, bytes_one.length(), bytes_one);
        msg[19] = getCrc(msg.left(19));
        writeValue(UUID_CHARACTERISTIC_NUS_WRITE, msg);
}

void DK08NUSService::setSMSAlertContext(int id, AlertType alertType, const QString &context) {
    qDebug() << Q_FUNC_INFO << id << context;

    QByteArray bytes = toUtf16Hex(context);

    while (bytes.length() > 0) {
        QByteArray bytes_one = bytes.left(12);
        bytes = bytes.mid(12);

        QByteArray msg(20, 0);
        msg[0] = NUSCallType::Command;
        msg[1] = NUSCallType::InfoAlert; // type
        msg[2] = alertType;
        msg[3] = (id | 0 << 7);

        msg[4] = bytes_one.length();
        msg[5] = 0x03; // 0x03 unicode, 0x00 Ascii BCD
        msg.replace(6, bytes_one.length(), bytes_one);
        msg[19] = getCrc(msg.left(19));

        // qDebug() << msg.toHex();
        writeValue(UUID_CHARACTERISTIC_NUS_WRITE, msg);
    }

}

void DK08NUSService::setAlarms() {

    QByteArray msg(20, 0);

    msg[0] = NUSCallType::Command;
    msg[1] = NUSCallType::Alarm;
    msg[2] = 0x00; // 0x00 (not sure about 0x00), (it looks like 0x80 1-5 alarm, 0x81 6-10, 0x82 11-15th alarm)

    // headr; enabled  ; 5x hour:minute                  ; zero checksum
    // 0 1 2; 3 4 5 6 7; 8 9  10 11  12 13  14 15   16 17;  18 19

    auto config = AmazfishConfig::instance();
    for (int i = 0, n = 1; i < 5; ++i, ++n) {
        msg[3+i] =  config->alarmEnabled(n) ? 1 : 2; // 1 and 2 are magic constants // this should be byte
        // FIXME mask (i.e. days are not used - it is confusing for user)
        // int repeatMask = config->alarmRepeatMask(n) == 0 ? 128 : config->alarmRepeatMask(n);
        msg[8+(2*i)] = twoDigitsToBCD(config->alarmHour(n));
        msg[9+(2*i)] = twoDigitsToBCD(config->alarmMinute(n));
    }

    msg[19] = getCrc(msg.left(19));

    writeAsync(UUID_CHARACTERISTIC_NUS_WRITE, msg);

    qDebug() << Q_FUNC_INFO << msg.toHex();
}


void DK08NUSService::setUserInfo() {
    auto config = AmazfishConfig::instance();
    auto height = config->profileHeight();
    auto weight = config->profileWeight();

    QByteArray msg(20, 0);
    msg[0] = NUSCallType::Command;
    msg[1] = NUSCallType::User;
    msg[2] = weight;
    msg[3] = height;
    msg[19] = getCrc(msg.left(19));

    writeAsync(UUID_CHARACTERISTIC_NUS_WRITE, msg);

    qDebug() << Q_FUNC_INFO << msg.toHex();
}

void DK08NUSService::setSportTarget() {
    auto goal = AmazfishConfig::instance()->profileFitnessGoal();
    QByteArray goalBytes = TypeConversion::fromInt24(goal);
    QByteArray msg(20, 0);
    msg[0] = NUSCallType::Command;
    msg[1] = NUSCallType::SportTarget;
    msg[2] = goalBytes[0];
    msg[3] = goalBytes[1];
    msg[4] = goalBytes[2];
    msg[19] = getCrc(msg.left(19));
    writeAsync(UUID_CHARACTERISTIC_NUS_WRITE, msg);
    qDebug() << Q_FUNC_INFO << msg.toHex();
}

void DK08NUSService::setUnit() {
    auto unit = AmazfishConfig::instance()->deviceDistanceUnit(); // metric 0, imperial 1
    QByteArray msg(20, 0);
    msg[0] = NUSCallType::Command;
    msg[1] = NUSCallType::Unit;
    msg[2] = unit;
    msg[19] = getCrc(msg.left(19));
    writeAsync(UUID_CHARACTERISTIC_NUS_WRITE, msg);
    qDebug() << Q_FUNC_INFO << msg.toHex();
}


void DK08NUSService::setTimeFormat() {
    auto timeformat = AmazfishConfig::instance()->deviceTimeFormat();
    QByteArray msg(20, 0);
    msg[0] = NUSCallType::Command;
    msg[1] = NUSCallType::HourSelect;
    msg[2] = timeformat; // 0 == 24, 1 == 12
    msg[19] = getCrc(msg.left(19));
    writeAsync(UUID_CHARACTERISTIC_NUS_WRITE, msg);
    qDebug() << Q_FUNC_INFO << msg.toHex();
}

/**
 *  LCD behavior when turn-over the hand
 */
void DK08NUSService::setPalming() {

    int enabled;
    if (AmazfishConfig::instance()->profileDisplayOnLiftWrist() == AmazfishConfig::DisplayLiftWristOff) {
        enabled = 0;
    } else  if (AmazfishConfig::instance()->profileDisplayOnLiftWrist() == AmazfishConfig::DisplayLiftWristOn) {
        enabled = 1;
    } else {
        qWarning() << "Not implemented";
        return;
    }

    QByteArray msg(20, 0);
    msg[0] = NUSCallType::Command;
    msg[1] = NUSCallType::Palming;
    msg[2] = enabled;
    msg[19] = getCrc(msg.left(19));
    writeAsync(UUID_CHARACTERISTIC_NUS_WRITE, msg);
    qDebug() << Q_FUNC_INFO << msg.toHex();
}


DK08NUSService::WeatherType DK08NUSService::weatherCodeToWeatherType(int code) {
    if (code == 800) {
        return Sunshine;
    }
    if (code >= 801 && code <= 804) {
        return Cloudy;
    }
    if (code >= 200 && code <= 599) {
        return Wet;
    }
    if (code >= 600 && code <= 699) {
        return Snow;
    }

    if (code == 701 || code == 711 || code == 721 || code == 741) {
        return Smaze;
    }

    if (code == 731 || code == 751 || code == 761 || code == 762) {
        return Mote;
    }

    qWarning() << "unknown weather type" << code;

    return Unknown;
}

void DK08NUSService::sendWeather(CurrentWeather *weather) {

    int fcCount = weather->forecastCount();
    if (fcCount < 3) {
        qWarning() << "Not enough weather data " << fcCount;
        return;
    }
    CurrentWeather::Forecast fc1 = weather->forecast(0);
    CurrentWeather::Forecast fc2 = weather->forecast(1);
    CurrentWeather::Forecast fc3 = weather->forecast(2);

    WeatherType wt0 = weatherCodeToWeatherType(weather->weatherCode());
    WeatherType wt1 = weatherCodeToWeatherType(fc1.weatherCode());
    WeatherType wt2 = weatherCodeToWeatherType(fc2.weatherCode());
    WeatherType wt3 = weatherCodeToWeatherType(fc3.weatherCode());

    if ((wt0 == WeatherType::Unknown) || (wt1 == WeatherType::Unknown)
        || (wt2 == WeatherType::Unknown) || (wt3 == WeatherType::Unknown)) {
        qWarning() << "Unknown weather type" << wt0 << wt1 << wt2 << wt3;
        return;
    }

    // convert to celsius

    int curT0 = weather->temperature() - 273.15;

    int minT0 = weather->minTemperature() - 273.15;
    int minT1 = fc1.minTemperature() - 273.15;
    int minT2 = fc2.minTemperature() - 273.15;
    int minT3 = fc3.minTemperature() - 273.15;

    int maxT0 = weather->maxTemperature() - 273.15;
    int maxT1 = fc1.maxTemperature() - 273.15;
    int maxT2 = fc2.maxTemperature() - 273.15;
    int maxT3 = fc3.maxTemperature() - 273.15;

    QByteArray msg(20, 0);
    msg[0]  = NUSCallType::Command;
    msg[1]  = NUSCallType::Weather;
    msg[2]  = 0x03; // number of days
    msg[3]  = wt0;
    msg[4]  = static_cast<uint8_t>( (minT0 < 0) ? 128 - minT0 : minT0 );
    msg[5]  = static_cast<uint8_t>( (maxT0 < 0) ? 128 - maxT0 : maxT0 );
    msg[6]  = static_cast<uint8_t>( (curT0 < 0) ? 128 - curT0 : curT0);
    msg[7]  = wt1;
    msg[8]  = static_cast<uint8_t>( (minT1 < 0) ? 128 - minT1 : minT1 );
    msg[9]  = static_cast<uint8_t>( (maxT1 < 0) ? 128 - maxT1 : maxT1 );
    msg[10] = 0xFF;
    msg[11] = wt2;
    msg[12] = static_cast<uint8_t>( (minT2 < 0) ? 128 - minT2 : minT2 );
    msg[13] = static_cast<uint8_t>( (maxT2 < 0) ? 128 - maxT2 : maxT2 );
    msg[14] = 0xFF;
    msg[15] = wt3;
    msg[16] = static_cast<uint8_t>( (minT3 < 0) ? 128 - minT3 : minT3 );
    msg[17] = static_cast<uint8_t>( (maxT3 < 0) ? 128 - maxT3 : maxT3 );
    msg[18] = 0xFF;
    msg[19] = getCrc(msg.left(19));
    writeAsync(UUID_CHARACTERISTIC_NUS_WRITE, msg);
    qDebug() << Q_FUNC_INFO << msg.toHex();

}

uint8_t DK08NUSService::getCrc(const QByteArray& data) {
    int d = 0;
    for (uint8_t byte : data) {
        d += byte;
    }
    return static_cast<uint8_t>((~d + 1) & 0xFF);
}

char DK08NUSService::twoDigitsToBCD(int i) {
    int high = (i / 10) % 10;
    int low = i % 10;
    return static_cast<char>((high << 4) | low);
}

int DK08NUSService::bcdToTwoDigits(char bcd) {
    int high = (bcd >> 4) & 0x0F;
    int low = bcd & 0x0F;
    return high * 10 + low;
}

QByteArray DK08NUSService::intToBCD(int i) {
    QByteArray out;

    while (i > 0) {
        int low = i % 10;
        i /= 10;
        int high = (i > 0) ? (i % 10) : 0;
        i /= 10;

        out.prepend(twoDigitsToBCD(high * 10 + low));
    }

    return out;
}

void DK08NUSService::test() {

    qDebug() << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";

    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("01 00 01 01 01 00 00 08 00 08 30 09 00 00 00 00 00 00 00 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("08 01 01 00 03 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("08 01 01 08 03 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("08 01 01 0C 03 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("08 01 C1 06 03 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("09 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("11 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("12 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("15 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("16 01 00 3C 12 00 14 00 09 00 21 00 00 64 00 00 00 00 00 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("17 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("18 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("19 00 80 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("19 80 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("19 81 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("22 01 01 01 00 1E 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("29 03 01 01 87 FF 00 01 87 FF 00 01 89 FF 00 00 00 FF 41 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("2E 00 19 10 07 30 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "));
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("2E 01 19 10 07 30 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "));

    // "language settings successful" in log
    characteristicRead(DK08NUSService::UUID_CHARACTERISTIC_NUS_NOTIFY,QByteArray::fromHex("34 01 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "));
    exit(1);
}
