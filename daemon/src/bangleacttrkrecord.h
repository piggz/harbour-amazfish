#ifndef BANGLEACTTRKRECORD__H
#define BANGLEACTTRKRECORD__H

#include <QString>
#include <QDateTime>
#include <QList>
#include <QDebug>

class BangleActTrkRecord
{
public:
    BangleActTrkRecord();

    static BangleActTrkRecord fromCsvRow(const QStringList &columns);
    static void fromCsvHeader(const QStringList &header);

    QDateTime time() const;

    int heartRate() const;
    int confidence() const;
    QString source() const;

    int batteryPercentage() const;
    double batteryVoltage() const;
    bool charging() const;

    int steps() const;

    double accelX() const;
    double accelY() const;
    double accelZ() const;

    double barometerTemperature() const;
    double barometerPressure() const;
    double barometerAltitude() const;

    double latitude() const;
    double longitude() const;
    double altitude() const;

    bool isValid() const;


    void setTime(const QDateTime &time);

    void setHeartRate(int heartRate);
    void setConfidence(int confidence);
    void setSource(const QString &source);

    void setBatteryPercentage(int percentage);
    void setBatteryVoltage(double voltage);
    void setCharging(bool charging);

    void setSteps(int steps);

    void setAccelX(double value);
    void setAccelY(double value);
    void setAccelZ(double value);

    void setBarometerTemperature(double value);
    void setBarometerPressure(double value);
    void setBarometerAltitude(double value);

    void setLatitude(double value);
    void setLongitude(double value);
    void setAltitude(double value);


private:
    QDateTime m_time;

    int m_heartRate;
    int m_confidence;
    QString m_source;

    int m_batteryPercentage;
    double m_batteryVoltage;
    bool m_charging;

    int m_steps;

    double m_accelX;
    double m_accelY;
    double m_accelZ;

    double m_barometerTemperature;
    double m_barometerPressure;
    double m_barometerAltitude;

    double m_latitude;
    double m_longitude;
    double m_altitude;

    struct CsvColumns {
        int time = -1;
        int heartRate = -1;
        int confidence = -1;
        int source = -1;

        int batteryPercentage = -1;
        int batteryVoltage = -1;
        int charging = -1;

        int steps = -1;

        int accelX = -1;
        int accelY = -1;
        int accelZ = -1;

        int barometerTemperature = -1;
        int barometerPressure = -1;
        int barometerAltitude = -1;

        int latitude = -1;
        int longitude = -1;
        int altitude = -1;
    };
    static CsvColumns m_columns;

};

QDebug operator<<(QDebug dbg, const BangleActTrkRecord &record);

#endif // BANGLEACTTRKRECORD__H
