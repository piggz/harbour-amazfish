#ifndef AMAZFISH_H
#define AMAZFISH_H

#include <QObject>
#include <QVariantMap>
#include <zlib.h>

//Container class for enums
class Amazfish : public QObject {
    Q_OBJECT
public:
    enum class Feature{
        FEATURE_HRM = 1,
        FEATURE_WEATHER = 2,
        FEATURE_ACTIVITY = 4,
        FEATURE_STEPS = 8,
        FEATURE_ALARMS = 16,
        FEATURE_ALERT = 32,
        FEATURE_EVENT_REMINDER = 64,
        FEATURE_MUSIC_CONTROL = 128,
        FEATURE_BUTTON_ACTION = 256,
        FEATURE_SCREENSHOT = 512,
    };
    Q_ENUM(Feature)
    Q_DECLARE_FLAGS(Features, Feature);

    enum class Info {
        INFO_SWVER = 1,
        INFO_HWVER,
        INFO_SERIAL,
        INFO_SYSTEMID,
        INFO_PNPID,
        INFO_GPSVER,
        INFO_BATTERY,
        INFO_STEPS,
        INFO_HEARTRATE,
        INFO_MODEL,
        INFO_FW_REVISION,
        INFO_MANUFACTURER
    };
    Q_ENUM(Info)

    enum class Settings {
        SETTING_USER_PROFILE,
        SETTING_USER_GOAL,
        SETTING_USER_ALERT_GOAL,
        SETTING_USER_ALL_DAY_HRM,
        SETTING_USER_HRM_SLEEP_DETECTION,
        SETTING_USER_DISPLAY_ON_LIFT,
        SETTING_ALARMS,
        SETTING_DEVICE_DISPLAY_ITEMS,
        SETTING_DEVICE_LANGUAGE,
        SETTING_DEVICE_DATE,
        SETTING_DEVICE_TIME,
        SETTING_DEVICE_UNIT,
        SETTING_DISCONNECT_NOTIFICATION,
        SETTING_DEVICE_REALTIME_HRM_MEASUREMENT
    };
    Q_ENUM(Settings)

    enum class DataType{
        TYPE_NONE = 0,
        TYPE_DEBUGLOG = 1,
        TYPE_ACTIVITY = 2,
        TYPE_GPS_TRACK = 4,
        TYPE_STRESS = 8,
        TYPE_SPO2 = 16,
        TYPE_PAI = 32,
        TYPE_HEART_RATE = 64,
        TYPE_SLEEP_RESPIRATORY_RATE = 128,
        TYPE_TEMPERATURE = 256,
        TYPE_SLEEP = 512,
        TYPE_HUAMI_STATISTICS = 1024
    };
    Q_ENUM(DataType)
    Q_DECLARE_FLAGS(DataTypes, DataType);

    struct WatchNotification
    {
        int id;
        QString appId;
        QString appName;
        QString summary;
        QString body;

        QVariantMap toQVariantMap() const {
            QVariantMap map;
            map["id"] = id;
            map["appId"] = appId;
            map["appName"] = appName;
            map["summary"] = summary;
            map["body"] = body;
            return map;
        }

        static WatchNotification fromQVariantMap(const QVariantMap &map) {
            WatchNotification n;
            n.id = map.value("id").toInt();
            n.appId = map.value("appId").toString();
            n.appName = map.value("appName").toString();
            n.summary = map.value("summary").toString();
            n.body = map.value("body").toString();
            return n;
        }
    };

    static int calculateCRC32(const QByteArray &bytes)
    {
        const char *b = bytes.data();
        int len = bytes.length();

        int  crc = crc32(0L, Z_NULL, 0);
        crc = crc32(crc, (const unsigned char*)b, len);

        return crc;
    }

    static QByteArray compressData(const QByteArray &data)
    {
        QByteArray compressedData = qCompress(data);
        compressedData.remove(0, 4);

        return compressedData;
    }

    static QByteArray unCompressData(const QByteArray &data)
    {
        QByteArray raw;
        raw += QByteArray(4, (char)0x00);
        raw += data;
        QByteArray un = qUncompress(raw);

        return un;
    }

};
Q_DECLARE_METATYPE(Amazfish::Feature)
Q_DECLARE_METATYPE(Amazfish::Info)
Q_DECLARE_METATYPE(Amazfish::Settings)
Q_DECLARE_METATYPE(Amazfish::WatchNotification)
Q_DECLARE_METATYPE(Amazfish::DataType)
Q_DECLARE_OPERATORS_FOR_FLAGS(Amazfish::Features)
Q_DECLARE_OPERATORS_FOR_FLAGS(Amazfish::DataTypes)

#endif // AMAZFISH_H
