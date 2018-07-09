#include "bipactivitydetailparser.h"

BipActivityDetailParser::BipActivityDetailParser(ActivitySummary summary) {
    m_summary = summary;

    m_baseLongitude = summary.getBaseLongitude();
    m_baseLatitude = summary.getBaseLatitude();
    m_baseAltitude = summary.getBaseAltitude();
    m_baseDate = summary.getStartTime();

    //this.activityTrack = new ActivityTrack();
    //activityTrack.setUser(summary.getUser());
    //activityTrack.setDevice(summary.getDevice());
    //activityTrack.setName(summary.getName() + "-" + summary.getId());
}

bool BipActivityDetailParser::getSkipCounterByte()
{
    return skipCounterByte;
}

void BipActivityDetailParser::setSkipCounterByte(bool skip)
{
    skipCounterByte = skip;
}

ActivityTrack BipActivityDetailParser::parse(const QByteArray &bytes)
{
    int i = 0;
    try {
        long totalTimeOffset = 0;
        int lastTimeOffset = 0;
        while (i < bytes.length) {
            if (skipCounterByte && (i % 17) == 0) {
                i++;
            }

            byte type = bytes[i++];
            int timeOffset = BLETypeConversions.toUnsigned(bytes[i++]);
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
    } catch (IndexOutOfBoundsException ex) {
        throw new GBException("Error parsing activity details: " + ex.getMessage(), ex);
    }

    return activityTrack;
}

int BipActivityDetailParser::consumeGPSAndUpdateBaseLocation(const QByteArray &bytes, int offset, long timeOffset)
{
    int i = 0;
    int longitudeDelta = BLETypeConversions.toInt16(bytes[offset + i++], bytes[offset + i++]);
    int latitudeDelta = BLETypeConversions.toInt16(bytes[offset + i++], bytes[offset + i++]);
    int altitudeDelta = BLETypeConversions.toInt16(bytes[offset + i++], bytes[offset + i++]);

    baseLongitude += longitudeDelta;
    baseLatitude += latitudeDelta;
    baseAltitude += altitudeDelta;

    GPSCoordinate coordinate = new GPSCoordinate(
                convertHuamiValueToDecimalDegrees(baseLongitude),
                convertHuamiValueToDecimalDegrees(baseLatitude),
                baseAltitude);

    ActivityPoint ap = getActivityPointFor(timeOffset);
    ap.setLocation(coordinate);
    add(ap);

    return i;
}

double BipActivityDetailParser::convertHuamiValueToDecimalDegrees(long huamiValue)
{
    BigDecimal result = new BigDecimal(huamiValue).divide(HUAMI_TO_DECIMAL_DEGREES_DIVISOR, GPSCoordinate.GPS_DECIMAL_DEGREES_SCALE, RoundingMode.HALF_UP);
    return result.doubleValue();
}

int BipActivityDetailParser::consumeHeartRate(const QByteArray &bytes, int offset, long timeOffsetSeconds)
{
    int v1 = BLETypeConversions.toUint16(bytes[offset]);
    int v2 = BLETypeConversions.toUint16(bytes[offset + 1]);
    int v3 = BLETypeConversions.toUint16(bytes[offset + 2]);
    int v4 = BLETypeConversions.toUint16(bytes[offset + 3]);
    int v5 = BLETypeConversions.toUint16(bytes[offset + 4]);
    int v6 = BLETypeConversions.toUint16(bytes[offset + 5]);

    if (v2 == 0 && v3 == 0 && v4 == 0 && v5 == 0 && v6 == 0) {
        // new version
        //            LOG.info("detected heart rate in 'new' version, where version is: " + summary.getVersion());
        LOG.info("detected heart rate in 'new' version format");
        ActivityPoint ap = getActivityPointFor(timeOffsetSeconds);
        ap.setHeartRate(v1);
        add(ap);
    } else {
        ActivityPoint ap = getActivityPointFor(v1);
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

ActivityPoint BipActivityDetailParser::getActivityPointFor(long timeOffsetSeconds)
{
    Date time = makeAbsolute(timeOffsetSeconds);
    if (lastActivityPoint != null) {
        if (lastActivityPoint.getTime().equals(time)) {
            return lastActivityPoint;
        }
    }
    return new ActivityPoint(time);
}

QDateTime BipActivityDetailParser::makeAbsolute(long timeOffsetSeconds)
{
    return new Date(baseDate.getTime() + timeOffsetSeconds * 1000);
}

void BipActivityDetailParser::add(QPointF ap)
{
    if (ap != lastActivityPoint) {
        lastActivityPoint = ap;
        activityTrack.addTrackPoint(ap);
    } else {
        qDebug() << "skipping point!";
    }
}

int BipActivityDetailParser::consumeUnknown2(const QByteArray &bytes, int offset)
{
    return 6; // just guessing...
}

int BipActivityDetailParser::consumePause(const QByteArray &bytes, int i)
{
    return 6; // just guessing...
}

int BipActivityDetailParser::consumeSpeed4(const QByteArray &bytes, int offset)
{
    return 6;
}

int BipActivityDetailParser::consumeSpeed5(const QByteArray &bytes, int offset)
{
    return 6;
}

int BipActivityDetailParser::consumeSpeed6(const QByteArray &bytes, int offset)
{
    return 6;
}
