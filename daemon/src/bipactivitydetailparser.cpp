#include "bipactivitydetailparser.h"
#include "typeconversion.h"
#include <QDebug>
#include <QTimeZone>
#include "amazfishconfig.h"

#if QT_VERSION >= 0x051400
#define endl Qt::endl
#endif

BipActivityDetailParser::BipActivityDetailParser(const ActivitySummary &summary) : m_summary(summary)
{
    m_baseLongitude = summary.baseLongitude();
    m_baseLatitude = summary.baseLatitude();
    m_baseAltitude = summary.baseAltitude();
    m_baseDate = summary.startTime();

    qDebug() << Q_FUNC_INFO;
    qDebug() << "Base data:" << m_baseLongitude << m_baseLatitude << m_baseAltitude << m_baseDate;
    qDebug() << summary.startTime() << m_summary.startTime();
    qDebug() << convertHuamiValueToDecimalDegrees(m_baseLongitude) << convertHuamiValueToDecimalDegrees(m_baseLatitude);
    
    //activityTrack.setUser(summary.getUser());
    //activityTrack.setDevice(summary.getDevice());
    //activityTrack.setName(summary.getName() + "-" + summary.getId());
}

bool BipActivityDetailParser::getSkipCounterByte()
{
    return m_skipCounterByte;
}

void BipActivityDetailParser::setSkipCounterByte(bool skip)
{
    m_skipCounterByte = skip;
}

void BipActivityDetailParser::parse(const QByteArray &bytes)
{
    int i = 0;

    long totalTimeOffset = 0;
    int lastTimeOffset = 0;
    bool have_gps = false;
    while (i < bytes.length()) {
        if (m_skipCounterByte && (i % 17) == 0) {
            i++;
        }

        char type = bytes[i++];
        int timeOffset = TypeConversion::toUnsigned(bytes[i++]);
        // handle timeOffset overflows (1 byte, always increasing, relative to base)
        if (lastTimeOffset <= timeOffset) {
            timeOffset = timeOffset - lastTimeOffset;
            lastTimeOffset += timeOffset;
        } else {
            lastTimeOffset = timeOffset;
        }
        totalTimeOffset += timeOffset;

        qDebug() << "timeOffset:" << timeOffset << totalTimeOffset << (int)type;

        ActivityCoordinate ap;
        switch (type) {
        case TYPE_GPS:
            have_gps = true;
            i += consumeGPSAndUpdateBaseLocation(bytes, i, totalTimeOffset);
            break;
        case TYPE_HR:
            i += consumeHeartRate(bytes, i, totalTimeOffset);
            if (!have_gps) {
                ap.setHeartRate(m_lastHeartrate);
                ap.setTimeStamp(makeAbsolute(totalTimeOffset));
                m_activityTrack << ap;
            }
            break;
        case TYPE_UNKNOWN2:
            i += consumeUnknown2(bytes, i);
            break;
        case TYPE_PAUSE:
            i += consumePause(bytes, i);
            break;
        case TYPE_SPEED4:
            i += consumeSpeed4(bytes, i);
            break;
        case TYPE_SPEED5:
            i += consumeSpeed5(bytes, i);
            break;
        case TYPE_GPS_SPEED6:
            i += consumeSpeed6(bytes, i);
            break;
        default:
            qDebug() << Q_FUNC_INFO << "unknown packet type" << type;
            i+=6;
        }

        if (type == TYPE_GPS || type == TYPE_HR) {

        }
    }
}

int BipActivityDetailParser::consumeGPSAndUpdateBaseLocation(const QByteArray &bytes, int offset, long timeOffset)
{
    int16_t longitudeDelta = TypeConversion::toInt16(bytes[offset + 0], bytes[offset + 1]);
    int16_t latitudeDelta = TypeConversion::toInt16(bytes[offset + 2], bytes[offset + 3]);
    int16_t altitudeDelta = TypeConversion::toInt16(bytes[offset + 4], bytes[offset + 5]);

    m_baseLongitude += longitudeDelta;
    m_baseLatitude += latitudeDelta;
    m_baseAltitude += altitudeDelta;
    
    m_lastCoordinate.setLongitude(convertHuamiValueToDecimalDegrees(m_baseLongitude));
    m_lastCoordinate.setLatitude(convertHuamiValueToDecimalDegrees(m_baseLatitude));
    m_lastCoordinate.setAltitude(m_baseAltitude);

    ActivityCoordinate ap = getActivityPointFor(timeOffset);
    ap.setCoordinate(m_lastCoordinate);
    ap.setHeartRate(m_lastHeartrate);
    add(ap);
    return 6;
}

double BipActivityDetailParser::convertHuamiValueToDecimalDegrees(long huamiValue)
{
    return huamiValue / HUAMI_TO_DECIMAL_DEGREES_DIVISOR;
}

int BipActivityDetailParser::consumeHeartRate(const QByteArray &bytes, int offset, long timeOffsetSeconds)
{
    Q_UNUSED(timeOffsetSeconds);
    int v1 = TypeConversion::toUint16(bytes[offset]);
    int v2 = TypeConversion::toUint16(bytes[offset + 1]);
    int v3 = TypeConversion::toUint16(bytes[offset + 2]);
    int v4 = TypeConversion::toUint16(bytes[offset + 3]);
    int v5 = TypeConversion::toUint16(bytes[offset + 4]);
    int v6 = TypeConversion::toUint16(bytes[offset + 5]);

    if (v2 == 0 && v3 == 0 && v4 == 0 && v5 == 0 && v6 == 0) {
        // new version
        //            LOG.info("detected heart rate in 'new' version, where version is: " + summary.getVersion());
        //LOG.info("detected heart rate in 'new' version format");
        //ActivityCoordinate ap = getActivityPointFor(timeOffsetSeconds);
        //ap.setHeartRate(v1);
        //add(ap);
        m_lastHeartrate = v1;
    } else {
        //ActivityCoordinate ap = getActivityPointFor(v1);
        //ap.setHeartRate(v2);
        //add(ap);

        //ap = getActivityPointFor(v3);
        //ap.setHeartRate(v4);
        //add(ap);

        //ap = getActivityPointFor(v5);
        //ap.setHeartRate(v6);
        //add(ap);
        m_lastHeartrate = v6;
    }
    return 6;
}

ActivityCoordinate BipActivityDetailParser::getActivityPointFor(long timeOffsetSeconds)
{
    QDateTime time = makeAbsolute(timeOffsetSeconds);
    if (m_lastActivityPoint.timeStamp() == time) {
        return m_lastActivityPoint;
    }

    ActivityCoordinate p;
    p.setTimeStamp(time);
    return p;
}

QDateTime BipActivityDetailParser::makeAbsolute(long timeOffsetSeconds)
{
    return m_baseDate.addSecs(timeOffsetSeconds);
}

void BipActivityDetailParser::add(const ActivityCoordinate &ap)
{
    if (ap.coordinate() != m_lastActivityPoint.coordinate()) {
        if (ap.timeStamp() == m_lastActivityPoint.timeStamp() || m_tempTrack.isEmpty()) {
            m_tempTrack << ap;
        } else {
            //Timestamp changed, process temp points
            int duration = ap.timeStamp().toMSecsSinceEpoch() - m_tempTrack.first().timeStamp().toMSecsSinceEpoch();
            int interval = duration / m_tempTrack.size();

            //Loop over the previous count of entries and add seconds to them
            for (int i = 0; i < m_tempTrack.size(); i++) {
                m_tempTrack[i].setTimeStamp(m_tempTrack[i].timeStamp().addMSecs(i * interval));
            }

            m_activityTrack << m_tempTrack;
            m_tempTrack.clear();
            m_tempTrack << ap;
        }
        m_lastActivityPoint = ap;
    } else {
        qDebug() << Q_FUNC_INFO << "skipping point!";
    }
}

int BipActivityDetailParser::consumeUnknown2(const QByteArray &bytes, int offset)
{
    Q_UNUSED(bytes);
    Q_UNUSED(offset)
    return 6; // just guessing...
}

int BipActivityDetailParser::consumePause(const QByteArray &bytes, int offset)
{
    Q_UNUSED(bytes);
    Q_UNUSED(offset)
    return 6; // just guessing...
}

int BipActivityDetailParser::consumeSpeed4(const QByteArray &bytes, int offset)
{
    Q_UNUSED(bytes);
    Q_UNUSED(offset)
    return 6;
}

int BipActivityDetailParser::consumeSpeed5(const QByteArray &bytes, int offset)
{
    Q_UNUSED(bytes);
    Q_UNUSED(offset)
    return 6;
}

int BipActivityDetailParser::consumeSpeed6(const QByteArray &bytes, int offset)
{
    Q_UNUSED(bytes);
    Q_UNUSED(offset)
    return 6;
}

QString BipActivityDetailParser::toText()
{
    QString str;
    QTextStream out(&str);
    out.setRealNumberPrecision(10);

    out << "<?xml version=\"1.0\" standalone=\"yes\"?>" << endl;
    out << "<?xml-stylesheet type=\"text/xsl\" href=\"details.xsl\"?>" << endl;
    out << "<gpx" << endl;
    out << "version=\"1.1\"" << endl;
    out << "creator=\"Amazfish for SailfishOS\"" << endl;
    out << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << endl;
    out << "xmlns=\"http://www.topografix.com/GPX/1/1\"" << endl;
    out << "xmlns:topografix=\"http://www.topografix.com/GPX/Private/TopoGrafix/0/1\"" << endl;
    out << "xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd http://www.garmin.com/xmlschemas/GpxExtensions/v3 http://www.garmin.com/xmlschemas/GpxExtensionsv3.xsd http://www.garmin.com/xmlschemas/TrackPointExtension/v1 http://www.garmin.com/xmlschemas/TrackPointExtensionv1.xsd\"" << endl;
    out << "xmlns:gpxtpx=\"http://www.garmin.com/xmlschemas/TrackPointExtension/v1\">" << endl;

    //Laufhelden compatible metadata
    out << "<metadata>" << endl;
    out << "<name>" << m_summary.name() << "</name>" << endl;
    out << "<desc></desc>" << endl;
    out << "<extensions>" << endl;
    out << "<meerun activity=\"" << ActivityKind::toString(m_summary.activityKind()).toLower() << "\" />" << endl;
    out << "</extensions>" << endl;
    out << "</metadata>" << endl;

    out << "<trk>" << endl;
    out << "<trkseg>" << endl;

    foreach(ActivityCoordinate pos, m_activityTrack) {
        if (pos.coordinate().isValid()) {
            out << "<trkpt lat=\""<< pos.coordinate().latitude() << "\" lon=\"" << pos.coordinate().longitude() << "\">" << endl;
            out << "<ele>" << pos.coordinate().altitude() << "</ele>" << endl;
            QDateTime dt = pos.timeStamp();
            //dt.setTimeZone(QTimeZone::systemTimeZone());
            //dt.setTimeSpec(Qt::OffsetFromUTC);
            out << "<time>" << dt.toTimeSpec(Qt::OffsetFromUTC).toString(Qt::ISODate) << "</time>" << endl;
            out << "<extensions><gpxtpx:TrackPointExtension><gpxtpx:hr>" << pos.heartRate() << "</gpxtpx:hr></gpxtpx:TrackPointExtension></extensions>" << endl;
            out << "</trkpt>" << endl;
        }
    }
    out << "</trkseg>" << endl;
    out << "</trk>" << endl;
    out << "</gpx>" << endl;

    return str;
}

QString BipActivityDetailParser::toTCX()
{
    QString str;
    QTextStream out(&str);
    out.setRealNumberPrecision(10);

    out << "<?xml version=\"1.0\" standalone=\"yes\"?>" << endl;
    out << "<TrainingCenterDatabase" << endl;
    out << "xmlns=\"http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2\"" << endl;
    out << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << endl;
    out << "xsi:schemaLocation=\"http://www.garmin.com/xmlschemas/ActivityExtension/v2" << endl;
    out << "http://www.garmin.com/xmlschemas/ActivityExtensionv2.xsd" << endl;
    out << "http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2" << endl;
    out << "http://www.garmin.com/xmlschemas/TrainingCenterDatabasev2.xsd\">" << endl;

    out << "<Activities>" << endl;
    out << "<Activity Sport=\"" + ActivityKind::toString(m_summary.activityKind()) + "\">" << endl;
    out << "<Id>" << m_summary.name() << "</Id>" << endl;
    out << "<Lap StartTime=\"" << m_summary.startTime().toTimeSpec(Qt::OffsetFromUTC).toString(Qt::ISODate) << "\">" << endl;

    //TotalTimeSeconds
    ActivitySummary::meta m = m_summary.metaData("activeSeconds");
    if (m.key == "activeSeconds") {
        out << "<TotalTimeSeconds>" << m.value << "</TotalTimeSeconds>" << endl;
    }

    //DistanceMeters
    m = m_summary.metaData("distanceMeters");
    if (m.key == "distanceMeters") {
        out << "<DistanceMeters>" << m.value << "</DistanceMeters>" << endl;
    }

    //Calories
    m = m_summary.metaData("caloriesBurnt");
    if (m.key == "caloriesBurnt") {
        out << "<Calories>" << m.value << "</Calories>" << endl;
    }

    //AverageHeartRateBpm
    m = m_summary.metaData("averageHR");
    if (m.key == "averageHR") {
        out << "<AverageHeartRateBpm><Value>" << m.value << "</Value></AverageHeartRateBpm>" << endl;
    }

    //MaximumHeartRateBpm
    m = m_summary.metaData("maxHR");
    if (m.key == "maxHR") {
        out << "<MaximumHeartRateBpm><Value>" << m.value << "</Value></MaximumHeartRateBpm>" << endl;
    }

    out << "<Track>" << endl;

    foreach(ActivityCoordinate pos, m_activityTrack) {
        out << "<Trackpoint>" << endl;
        QDateTime dt = pos.timeStamp();
        //dt.setTimeZone(QTimeZone::systemTimeZone());
        //dt.setTimeSpec(Qt::OffsetFromUTC);
        out << "<Time>" << dt.toTimeSpec(Qt::OffsetFromUTC).toString(Qt::ISODate) << "</Time>" << endl;

        if (pos.coordinate().isValid()) {
            out << "  <Position>" << endl;
            out << "    <LatitudeDegrees>" << pos.coordinate().latitude() << "</LatitudeDegrees>" << endl;
            out << "    <LongitudeDegrees>" << pos.coordinate().longitude() << "</LongitudeDegrees>" << endl;
            out << "  </Position>" << endl;
            out << "  <AltitudeMeters>" << pos.coordinate().altitude() << "</AltitudeMeters>" << endl;
        }
        out << "<HeartRateBpm xsi:type=\"HeartRateInBeatsPerMinute_t\"><Value>" << pos.heartRate() << "</Value></HeartRateBpm>" << endl;
        out << "</Trackpoint>" << endl;
    }

    out << "</Track>" << endl;

    //Steps
    m = m_summary.metaData("steps");
    if (m.key == "steps") {
        out << "<Extensions>" << endl;
        out << "  <LX xmlns=\"http://www.garmin.com/xmlschemas/ActivityExtension/v2\">" << endl;
        out << "    <Steps>" << m.value << "</Steps>" << endl;
        out << "  </LX>" << endl;
        out << "</Extensions>" << endl;
    }

    out << "</Lap>" << endl;

    //Creator
    out << "<Creator xsi:type=\"Device_t\"><Name>" << AmazfishConfig::instance()->pairedName() << "</Name><UnitId>0000000000</UnitId><ProductId>0000</ProductId><Version><VersionMajor>1</VersionMajor><VersionMinor>0</VersionMinor><BuildMajor>1</BuildMajor><BuildMinor>0</BuildMinor></Version></Creator>" << endl;

    out << "</Activity>" << endl;
    out << "</Activities>" << endl;

    //Author
    out << "<Author xsi:type=\"Application_t\"><Name>Amazfish</Name><Build><Version><VersionMajor>1</VersionMajor><VersionMinor>0</VersionMinor><BuildMajor>1</BuildMajor><BuildMinor>0</BuildMinor></Version></Build><LangID>en</LangID><PartNumber>000-00000-00</PartNumber></Author>" << endl;

    out << "</TrainingCenterDatabase>" << endl;

    return str;
}
