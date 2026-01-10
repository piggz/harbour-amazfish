#include "bipactivitydetailparser.h"
#include "typeconversion.h"
#include <QDebug>
#include <QTimeZone>

#if QT_VERSION >= 0x051400
#define endl Qt::endl
#endif

BipActivityDetailParser::BipActivityDetailParser()
{

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
    qDebug() << Q_FUNC_INFO << bytes.toHex();

    m_baseLongitude = m_summary.baseLongitude();
    m_baseLatitude = m_summary.baseLatitude();
    m_baseAltitude = m_summary.baseAltitude();
    m_baseDate = m_summary.startTime();

    qDebug() << Q_FUNC_INFO;
    qDebug() << "Base data:" << m_baseLongitude << m_baseLatitude << m_baseAltitude << m_baseDate;
    qDebug() << m_summary.startTime() << m_summary.startTime();
    qDebug() << convertHuamiValueToDecimalDegrees(m_baseLongitude) << convertHuamiValueToDecimalDegrees(m_baseLatitude);


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

        qDebug() << i << (int)type << timeOffset << totalTimeOffset;

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
        case TYPE_SWIMMING:
            i += consumeSwimming(bytes, i);
            break;
        default:
            //qDebug() << Q_FUNC_INFO << "unknown packet type" << type;
            i+=6;
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

int BipActivityDetailParser::consumeSwimming(const QByteArray &bytes, int offset)
{
    Q_UNUSED(bytes);
    Q_UNUSED(offset);
    return 6;
}
