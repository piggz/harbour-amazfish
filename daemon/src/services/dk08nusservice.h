#ifndef DK08NUSSERVICE_H
#define DK08NUSSERVICE_H

#include "qble/qbleservice.h"
#include "devices/abstractdevice.h"

class DK08NUSService : public QBLEService
{
    Q_OBJECT
public:
    enum NUSCallType : uint8_t {
        Time = 0x00,
        Alarm = 0x01,
        Shake = 0x02,
        Sport = 0x03,
        ClearSport = 0x04,
        Gps = 0x05,
        User = 0x06,
        SportTarget = 0x07,
        InfoAlert = 0x08,
        HeartRateTest = 0x09,
        HeartRate = 0x0A,
        SleepInfo = 0x0C,
        NotifyGps = 0x0D,
        FirmwareVersion = 0x0F,
        FindOrLost = 0x10,
        BloodPressure = 0x11,
        BloodOxygen = 0x12,
        DoubleOnoff = 0x13,
        FactoryTest = 0x14,
        Palming = 0x15,
        SedentaryAlert = 0x16,
        Unit = 0x17,
        HourSelect = 0x18,
        Camera = 0x19,
        StepSection = 0x1A,
        ParseSport = 0x1B,
        HrTimingTest = 0x22,
        SleepStats = 0x23,
        UltraViolet = 0x28,
        Weather = 0x29,
        MedicationReminder = 0x2B,
        HydrationReminder = 0x2C,
        DND = 0x2E,
        Fatigue = 0x31,
        MicroTest = 0x32,
        MicroInfo = 0x33,
        EcgHeartRate = 0x41,
        HrBaseLine = 0x42,
        Command = 0xFC,
    };
    Q_ENUM(NUSCallType)

    enum InfoType {
        Current = 0,
        History = 1,
        HistoryNum = 2,
        Notify = 3,
    };
    Q_ENUM(InfoType)

    enum AlertType {
        PhoneAlert = 0,
        SMSAlert = 1,
        QQ = 2,
        HangupOrAnswer = 3, // remove notification
        WeChat = 4,
        Whatsapp = 5,
        Facebook = 6,
        Line = 7,
        Twitter = 8,
        Skype = 9,
        Instagram = 10,
    };
    Q_ENUM(AlertType)

    enum SleepType {
        Deep = 1,
        Light = 2,
        Awake = 3,
    };
    Q_ENUM(SleepType)

    enum WeatherType {
        Sunshine,
        Cloudy,
        Wet,
        Snow,
        Smaze,
        Mote,
        Unknown
    };
    Q_ENUM(WeatherType);

    DK08NUSService(const QString &path, QObject *parent);

    static const char* UUID_SERVICE_NUS;
    static const char* UUID_CHARACTERISTIC_NUS_NOTIFY;
    static const char* UUID_CHARACTERISTIC_NUS_WRITE;

    Q_INVOKABLE void refreshInformation();

    Q_SIGNAL void informationChanged(Amazfish::Info key, const QString &val);

    void setTime();
    void getBattery();
    void getFirmware();
    void getHrInfo(InfoType type);
    void getSportInfo(InfoType type);
    void getSleep(InfoType type);
    void getSleepStats();
    void getStepSection(InfoType);
    void confirmSportHistoryData(int num);
    void setInfoAlert(AlertType t);
    void setPhoneAlert(const QString &caller);
    void setSMSAlertName(int id, AlertType alertType, const QString &name);
    void setSMSAlertContext(int id, AlertType alertType, const QString &context);
    void setAlarms();
    void setUserInfo();
    void setSportTarget();
    void setUnit();
    void setTimeFormat();
    void setPalming();
    void sendWeather(CurrentWeather *weather);

    void test();

private:
    Q_SLOT void characteristicRead(const QString &c, const QByteArray &value);

    WeatherType weatherCodeToWeatherType(int code);


    uint8_t getCrc(const QByteArray& data);
    char twoDigitsToBCD(int i);
    int bcdToTwoDigits(char i);
    QByteArray intToBCD(int i);
    QByteArray toUtf16Hex(QString str);
    QDateTime bcdToDateTime(QByteArray bytes);

    void parseSport(QByteArray value);
    void parseStep(QByteArray value);
    void parseStepSection(QByteArray value);
    void parseSleep(QByteArray value);
    void parseUser(QByteArray value);
    void parseSleepStats(QByteArray value);
    void parseFirmware(QByteArray value);
    void parseHr(QByteArray value);

};

#endif // DK08NUSSERVICE_H
