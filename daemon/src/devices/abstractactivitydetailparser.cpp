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
    out << "xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd http://www.garmin.com/xmlschemas/GpxExtensions/v3 http://www.garmin.com/xmlschemas/GpxExtensionsv3.xsd http://www.garmin.com/xmlschemas/TrackPointExtension/v2 http://www.garmin.com/xmlschemas/TrackPointExtensionv2.xsd\"" << endl;
    out << "xmlns:gpxtpx=\"http://www.garmin.com/xmlschemas/TrackPointExtension/v2\">" << endl;

    //Laufhelden compatible metadata
    out << "<metadata>" << endl;
    out << "  <name>" << m_summary.name() << "</name>" << endl;
    out << "  <desc></desc>" << endl;
    out << "  <extensions>" << endl;
    out << "    <meerun activity=\"" << ActivityKind::toString(m_summary.activityKind()).toLower() << "\" />" << endl;
    out << "  </extensions>" << endl;
    out << "</metadata>" << endl;

    out << "<trk>" << endl;
    out << "<trkseg>" << endl;

    foreach(ActivityCoordinate pos, m_activityTrack) {
        if (pos.coordinate().isValid()) {
            out << "<trkpt lat=\""<< pos.coordinate().latitude() << "\" lon=\"" << pos.coordinate().longitude() << "\">" << endl;
            out << "  <ele>" << pos.coordinate().altitude() << "</ele>" << endl;
            QDateTime dt = pos.timeStamp();
            //dt.setTimeZone(QTimeZone::systemTimeZone());
            //dt.setTimeSpec(Qt::OffsetFromUTC);
            out << "  <time>" << dt.toTimeSpec(Qt::OffsetFromUTC).toString(Qt::ISODate) << "</time>" << endl;
            out << "  <extensions>" << endl;
            out << "    <gpxtpx:TrackPointExtension>" << endl;
            out << "      <gpxtpx:hr>" << pos.heartRate() << "</gpxtpx:hr>" << endl;
            out << "      <gpxtpx:cad>" << pos.cadence() / 2 << "</gpxtpx:cad>" << endl; // The Cadence field in tcx is designed for rpm (e.g. cycling), so we have to divide spm by 2 to indicate the cadence value for a single leg. The average cadence for both legs goes in the AvgRunCadence field.
            out << "      <gpxtpx:speed>" << pos.speed() << "</gpxtpx:speed>" << endl;
            out << "    </gpxtpx:TrackPointExtension>" << endl;
            out << "  </extensions>" << endl;
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
    out << "xmlns:ns2=\"http://www.garmin.com/xmlschemas/ActivityExtension/v2\"" << endl;
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

    //MaximumSpeed
    m = m_summary.metaData("maxSpeed");
    if (m.key == "maxSpeed") {
        out << "<MaximumSpeed>" << m.value << "</MaximumSpeed>" << endl;
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

    //AverageCadence
    m = m_summary.metaData("stepsAvgCadence");
    if (m.key == "stepsAvgCadence") {
        out << "<Cadence>" << m.value.toInt() / 2 << "</Cadence>" << endl; // The Cadence field in tcx is designed for rpm (e.g. cycling), so we have to divide spm by 2 to indicate the cadence value for a single leg. The average cadence for both legs goes in the AvgRunCadence field.
    }

    out << "<Extensions>" << endl;
    out << "  <ns2:LX>" << endl;

    //AverageSpeed
    m = m_summary.metaData("averageSpeed");
    if (m.key == "averageSpeed") {
        out << "    <ns2:AvgSpeed>" << m.value << "</ns2:AvgSpeed>" << endl;
    }

    //AverageCadence
    m = m_summary.metaData("stepsAvgCadence");
    if (m.key == "stepsAvgCadence") {
        out << "    <ns2:AvgRunCadence>" << m.value << "</ns2:AvgRunCadence>" << endl;
    }

    //MaxCadence
    m = m_summary.metaData("stepsMaxCadence");
    if (m.key == "stepsMaxCadence") {
        out << "    <ns2:MaxRunCadence>" << m.value << "</ns2:MaxRunCadence>" << endl;
    }

    //Steps
    m = m_summary.metaData("steps");
    if (m.key == "steps") {
        out << "    <ns2:Steps>" << m.value << "</ns2:Steps>" << endl;
    }

    out << "  </ns2:LX>" << endl;
    out << "</Extensions>" << endl;
    out << "<Track>" << endl;

    foreach(ActivityCoordinate pos, m_activityTrack) {
        out << "<Trackpoint>" << endl;
        QDateTime dt = pos.timeStamp();
        //dt.setTimeZone(QTimeZone::systemTimeZone());
        //dt.setTimeSpec(Qt::OffsetFromUTC);
        out << "  <Time>" << dt.toTimeSpec(Qt::OffsetFromUTC).toString(Qt::ISODate) << "</Time>" << endl;

        if (pos.coordinate().isValid()) {
            out << "  <Position>" << endl;
            out << "    <LatitudeDegrees>" << pos.coordinate().latitude() << "</LatitudeDegrees>" << endl;
            out << "    <LongitudeDegrees>" << pos.coordinate().longitude() << "</LongitudeDegrees>" << endl;
            out << "  </Position>" << endl;
            out << "  <AltitudeMeters>" << pos.coordinate().altitude() << "</AltitudeMeters>" << endl;
        }
        out << "  <HeartRateBpm><Value>" << pos.heartRate() << "</Value></HeartRateBpm>" << endl;
        out << "  <Cadence>" << pos.cadence() / 2 << "</Cadence>" << endl; // The Cadence field in tcx is designed for rpm (e.g. cycling), so we have to divide spm by 2 to indicate the cadence value for a single leg. The cadence for both legs goes in the RunCadence field.
        out << "  <Extensions>" << endl;
        out << "    <ns2:TPX CadenceSensor=\"Footpod\">" << endl;
        out << "      <ns2:Speed>" << pos.speed() << "</ns2:Speed>" << endl;
        out << "      <ns2:RunCadence>" << pos.cadence() << "</ns2:RunCadence>" << endl;
        out << "    </ns2:TPX>" << endl;
        out << "  </Extensions>" << endl;
        out << "</Trackpoint>" << endl;
    }

    out << "</Track>" << endl;
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
