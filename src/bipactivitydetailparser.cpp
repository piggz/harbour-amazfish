#include "bipactivitydetailparser.h"
#include "typeconversion.h"
#include <QDebug>

BipActivityDetailParser::BipActivityDetailParser(ActivitySummary summary) {
    m_summary = summary;

    m_baseLongitude = summary.baseLongitude();
    m_baseLatitude = summary.baseLatitude();
    m_baseAltitude = summary.baseAltitude();
    m_baseDate = summary.startTime();

    //this.activityTrack = new ActivityTrack();
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

QString BipActivityDetailParser::parse(const QByteArray &bytes)
{
    int i = 0;
 
        long totalTimeOffset = 0;
        int lastTimeOffset = 0;
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

            switch (type) {
            case TYPE_GPS:
                i += consumeGPSAndUpdateBaseLocation(bytes, i, totalTimeOffset);
                break;
            case TYPE_HR:
                i += consumeHeartRate(bytes, i, totalTimeOffset);
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
            }
        }
    

    return QString();
}

int BipActivityDetailParser::consumeGPSAndUpdateBaseLocation(const QByteArray &bytes, int offset, long timeOffset)
{
    int i = 0;
    int longitudeDelta = TypeConversion::toInt16(bytes[offset + i++], bytes[offset + i++]);
    int latitudeDelta = TypeConversion::toInt16(bytes[offset + i++], bytes[offset + i++]);
    int altitudeDelta = TypeConversion::toInt16(bytes[offset + i++], bytes[offset + i++]);

    m_baseLongitude += longitudeDelta;
    m_baseLatitude += latitudeDelta;
    m_baseAltitude += altitudeDelta;

    QGeoCoordinate coordinate;
    
    coordinate.setLongitude(convertHuamiValueToDecimalDegrees(m_baseLongitude));
    coordinate.setLatitude(convertHuamiValueToDecimalDegrees(m_baseLatitude));
     coordinate.setAltidude(m_baseAltitude);

    ActivityCoordinate ap = getActivityPointFor(timeOffset);
    ap.setCoordinate(coordinate);
    add(ap);

    return i;
}

double BipActivityDetailParser::convertHuamiValueToDecimalDegrees(long huamiValue)
{
    return huamiValue / HUAMI_TO_DECIMAL_DEGREES_DIVISOR;
}

int BipActivityDetailParser::consumeHeartRate(const QByteArray &bytes, int offset, long timeOffsetSeconds)
{
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
        ActivityCoordiiate ap = getActivityPointFor(timeOffsetSeconds);
        ap.setHeartRate(v1);
        add(ap);
    } else {
        ActivityCoordinate ap = getActivityPointFor(v1);
        ap.setHeartRate(v2);
        add(ap);

        ap = getActivityPointFor(v3);
        ap.setHeartRate(v4);
        add(ap);

        ap = getActivityPointFor(v5);
        ap.setHeartRate(v6);
        add(ap);
    }
    return 6;
}

ActivityCoordinate BipActivityDetailParser::getActivityPointFor(long timeOffsetSeconds)
{
    QDateTime time = makeAbsolute(timeOffsetSeconds);
    if (m_lastActivityPoint.isValid()) {
        if (m_lastActivityPoint.timeStamp() == time) {
            return m_lastActivityPoint;
        }
    }
    ActivityCoordinate p;
    p.setDateTime(time);
    return p;
}

QDateTime BipActivityDetailParser::makeAbsolute(long timeOffsetSeconds)
{
    return m_baseDate.addSecs(timeOffsetSeconds);
}

void BipActivityDetailParser::add(const ActivityCoordinate &ap)
{
    if (ap != m_lastActivityPoint) {
        m_lastActivityPoint = ap;
        m_activityTrack << ap;
    } else {
        qDebug() << "skipping point!";
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
