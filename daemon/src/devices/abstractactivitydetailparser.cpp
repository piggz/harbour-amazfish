#include "abstractactivitydetailparser.h"
#include "amazfishconfig.h"

AbstractActivityDetailParser::~AbstractActivityDetailParser()
{

}

void AbstractActivityDetailParser::setSummary(const ActivitySummary &summary)
{
    m_summary = summary;
}

QString AbstractActivityDetailParser::toText()
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

QString AbstractActivityDetailParser::toTCX()
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

double AbstractActivityDetailParser::convertHuamiValueToDecimalDegrees(long huamiValue)
{
    return huamiValue / HUAMI_TO_DECIMAL_DEGREES_DIVISOR;
}
