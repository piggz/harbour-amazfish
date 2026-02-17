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

    auto has = [&](int idx) {
        return idx >= 0 && idx < c.size();
    };

    if (has(m_columns.time)) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
        r.setTime(QDateTime::fromSecsSinceEpoch(c.at(m_columns.time).toLongLong()));
#else
        r.setTime(QDateTime::fromTime_t(c.at(m_columns.time).toLongLong()));
#endif
    }

    if (has(m_columns.heartRate)) {
        r.setHeartRate(c.at(m_columns.heartRate).toInt());
    }
    if (has(m_columns.confidence)) {
        r.setConfidence(c.at(m_columns.confidence).toInt());
    }
    if (has(m_columns.source)) {
        r.setSource(c.at(m_columns.source));
    }

    if (has(m_columns.batteryPercentage)) {
        r.setBatteryPercentage(c.at(m_columns.batteryPercentage).toInt());
    }
    if (has(m_columns.batteryVoltage)) {
        r.setBatteryVoltage(c.at(m_columns.batteryVoltage).toDouble());
    }
    if (has(m_columns.charging)) {
        const QString v = c.at(m_columns.charging);
        r.setCharging(v == "1" || v == "true" || v == "True");
    }

    if (has(m_columns.steps)) {
        r.setSteps(c.at(m_columns.steps).toInt());
    }

    if (has(m_columns.accelX)) {
        r.setAccelX(c.at(m_columns.accelX).toDouble());
    }
    if (has(m_columns.accelY)) {
        r.setAccelY(c.at(m_columns.accelY).toDouble());
    }
    if (has(m_columns.accelZ)) {
        r.setAccelZ(c.at(m_columns.accelZ).toDouble());
    }

    if (has(m_columns.barometerTemperature)) {
        r.setBarometerTemperature(c.at(m_columns.barometerTemperature).toDouble());
    }
    if (has(m_columns.barometerPressure)) {
        r.setBarometerPressure(c.at(m_columns.barometerPressure).toDouble());
    }
    if (has(m_columns.barometerAltitude)) {
        r.setBarometerAltitude(c.at(m_columns.barometerAltitude).toDouble());
    }

    if (has(m_columns.latitude)) {
        r.setLatitude(c.at(m_columns.latitude).toDouble());
    }
    if (has(m_columns.longitude)) {
        r.setLongitude(c.at(m_columns.longitude).toDouble());
    }
    if (has(m_columns.altitude)) {
        r.setAltitude(c.at(m_columns.altitude).toDouble());
    }

    return r;
}


BangleActTrkRecord::CsvColumns BangleActTrkRecord::m_columns;

void BangleActTrkRecord::fromCsvHeader(const QStringList &header)
{
    m_columns = CsvColumns(); // reset

    for (int i = 0; i < header.size(); ++i) {
        const QString name = header.at(i).trimmed();

        if (name == "Time") {
            m_columns.time = i;
        } else if (name == "Battery Percentage") {
            m_columns.batteryPercentage = i;
        } else if (name == "Battery Voltage") {
            m_columns.batteryVoltage = i;
        } else if (name == "Charging") {
            m_columns.charging = i;
        } else if (name == "Steps") {
            m_columns.steps = i;
        } else if (name == "Accel X") {
            m_columns.accelX = i;
        } else if (name == "Accel Y") {
            m_columns.accelY = i;
        } else if (name == "Accel Z") {
            m_columns.accelZ = i;
        } else if (name == "Barometer Temperature") {
            m_columns.barometerTemperature = i;
        } else if (name == "Barometer Pressure") {
            m_columns.barometerPressure = i;
        } else if (name == "Barometer Altitude") {
            m_columns.barometerAltitude = i;
        } else if (name == "Latitude") {
            m_columns.latitude = i;
        } else if (name == "Longitude") {
            m_columns.longitude = i;
        } else if (name == "Altitude") {
            m_columns.altitude = i;
        } else if (name == "Heartrate") {
            m_columns.heartRate = i;
        } else if (name == "Confidence") {
            m_columns.confidence = i;
        } else if (name == "Source") {
            m_columns.source = i;
        }
    }
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
