#include "bangleacttrkrecord.h"
#include <QDebug>

BangleActTrkRecord::BangleActTrkRecord()
    : m_heartRate(0),
      m_confidence(0),
      m_batteryPercentage(0),
      m_batteryVoltage(0.0),
      m_charging(false),
      m_steps(0),
      m_accelX(0.0),
      m_accelY(0.0),
      m_accelZ(0.0),
      m_barometerTemperature(0.0),
      m_barometerPressure(0.0),
      m_barometerAltitude(0.0),
      m_latitude(0.0),
      m_longitude(0.0),
      m_altitude(0.0)
{
}


BangleActTrkRecord BangleActTrkRecord::fromCsvRow(const QStringList &c)
{
    BangleActTrkRecord r;

    if (c.size() < 17) {
        return r;
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    r.setTime(QDateTime::fromSecsSinceEpoch(c.at(0).toLongLong()));
#else
    r.setTime(QDateTime::fromTime_t(c.at(0).toLongLong()));
#endif

    r.setHeartRate(c.at(1).toInt());
    r.setConfidence(c.at(2).toInt());
    r.setSource(c.at(3));

    r.setBatteryPercentage(c.at(4).toInt());
    r.setBatteryVoltage(c.at(5).toDouble());
    r.setCharging(c.at(6) == "1" || c.at(6).toLower() == "true");

    r.setSteps(c.at(7).toInt());

    r.setAccelX(c.at(8).toDouble());
    r.setAccelY(c.at(9).toDouble());
    r.setAccelZ(c.at(10).toDouble());

    r.setBarometerTemperature(c.at(11).toDouble());
    r.setBarometerPressure(c.at(12).toDouble());
    r.setBarometerAltitude(c.at(13).toDouble());

    r.setLatitude(c.at(14).toDouble());
    r.setLongitude(c.at(15).toDouble());
    r.setAltitude(c.at(16).toDouble());

    return r;
}

void BangleActTrkRecord::setTime(const QDateTime &time) { m_time = time; }

void BangleActTrkRecord::setHeartRate(int heartRate) { m_heartRate = heartRate; }
void BangleActTrkRecord::setConfidence(int confidence) { m_confidence = confidence; }
void BangleActTrkRecord::setSource(const QString &source) { m_source = source; }

void BangleActTrkRecord::setBatteryPercentage(int percentage) { m_batteryPercentage = percentage; }
void BangleActTrkRecord::setBatteryVoltage(double voltage) { m_batteryVoltage = voltage; }
void BangleActTrkRecord::setCharging(bool charging) { m_charging = charging; }

void BangleActTrkRecord::setSteps(int steps) { m_steps = steps; }

void BangleActTrkRecord::setAccelX(double value) { m_accelX = value; }
void BangleActTrkRecord::setAccelY(double value) { m_accelY = value; }
void BangleActTrkRecord::setAccelZ(double value) { m_accelZ = value; }

void BangleActTrkRecord::setBarometerTemperature(double value) { m_barometerTemperature = value; }
void BangleActTrkRecord::setBarometerPressure(double value) { m_barometerPressure = value; }
void BangleActTrkRecord::setBarometerAltitude(double value) { m_barometerAltitude = value; }

void BangleActTrkRecord::setLatitude(double value) { m_latitude = value; }
void BangleActTrkRecord::setLongitude(double value) { m_longitude = value; }
void BangleActTrkRecord::setAltitude(double value) { m_altitude = value; }

QDateTime BangleActTrkRecord::time() const { return m_time; }

int BangleActTrkRecord::heartRate() const { return m_heartRate; }
int BangleActTrkRecord::confidence() const { return m_confidence; }
QString BangleActTrkRecord::source() const { return m_source; }

int BangleActTrkRecord::batteryPercentage() const { return m_batteryPercentage; }
double BangleActTrkRecord::batteryVoltage() const { return m_batteryVoltage; }
bool BangleActTrkRecord::charging() const { return m_charging; }

int BangleActTrkRecord::steps() const { return m_steps; }

double BangleActTrkRecord::accelX() const { return m_accelX; }
double BangleActTrkRecord::accelY() const { return m_accelY; }
double BangleActTrkRecord::accelZ() const { return m_accelZ; }

double BangleActTrkRecord::barometerTemperature() const { return m_barometerTemperature; }
double BangleActTrkRecord::barometerPressure() const { return m_barometerPressure; }
double BangleActTrkRecord::barometerAltitude() const { return m_barometerAltitude; }

double BangleActTrkRecord::latitude() const { return m_latitude; }
double BangleActTrkRecord::longitude() const { return m_longitude; }
double BangleActTrkRecord::altitude() const { return m_altitude; }

bool BangleActTrkRecord::isValid() const { return m_time.isValid(); }

QDebug operator<<(QDebug dbg, const BangleActTrkRecord &r)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "BangleActTrkRecord("
                  << "time=" << r.time().toString(Qt::ISODate)
                  << ", hr=" << r.heartRate()
                  << ", conf=" << r.confidence()
                  << ", src=\"" << r.source() << "\""
                  << ", batt%=" << r.batteryPercentage()
                  << ", battV=" << r.batteryVoltage()
                  << ", charging=" << r.charging()
                  << ", steps=" << r.steps()
                  << ", accel=("
                  << r.accelX() << ", "
                  << r.accelY() << ", "
                  << r.accelZ() << ")"
                  << ", baro=("
                  << r.barometerTemperature() << "°C, "
                  << r.barometerPressure() << "hPa, "
                  << r.barometerAltitude() << "m)"
                  << ", gps=("
                  << r.latitude() << ", "
                  << r.longitude() << ", "
                  << r.altitude() << "m)"
                  << ")";
    return dbg;
}
