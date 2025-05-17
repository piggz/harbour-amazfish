#include "zepposactivitydetailparser.h"
#include "typeconversion.h"


ZeppOsActivityDetailParser::ZeppOsActivityDetailParser()
{
    m_types = {
        {TIMESTAMP,12},
        {GPS_COORDS,20},
        {GPS_DELTA,8},
        {STATUS,4},
        {SPEED,8},
        {ALTITUDE,6},
        {HEARTRATE,3},
        {STRENGTH_SET,34},
    };
}

void ZeppOsActivityDetailParser::parse(const QByteArray &bytes)
{
    qDebug() << Q_FUNC_INFO << bytes.toHex();

    int i = 0;

    while (i < bytes.length()) {
        char typeCode = bytes[i++];

        // FIXME: This is probably not right, but type 31 makes the parser get out of sync otherwise
        if (typeCode == 31) {
            typeCode = bytes[i++];
        }

        char lengthByte = bytes[i++];
        if (lengthByte == -127) {
            lengthByte = bytes[i++];
        }
        int length = lengthByte & 0xff;
        int initialPosition = i - 1;

        Type type = typeFromCode(typeCode);

        //trace("Read typeCode={}, type={}, length={}, initialPosition={}", typeCode, type, length, initialPosition);

        if (type == UNKNOWN) {
            if (!m_unknownTypes.contains(typeCode)) {
                m_unknownTypes[typeCode] = 0;
            }

            m_unknownTypes[typeCode] = m_unknownTypes[typeCode] + 1;
            i += length;
            continue;
        } else if (length != typeLength(type)) {
            qDebug() << "Unexpected length " << length << " for type " << type;
            // Consume the reported length
            i += length;
            continue;
        }

        // Consume
        switch (type) {
        case TIMESTAMP:
            i += consumeTimestamp(bytes, i);
            break;
        case GPS_COORDS:
            i += consumeGpsCoords(bytes, i);
            break;
        case GPS_DELTA:
            i += consumeGpsDelta(bytes, i);
            break;
        case STATUS:
            i += consumeStatus(bytes, i);
            break;
        case SPEED:
            i += consumeSpeed(bytes, i);
            break;
        case ALTITUDE:
            i += consumeAltitude(bytes, i);
            break;
        case HEARTRATE:
            i += consumeHeartRate(bytes, i);
            break;
        case STRENGTH_SET:
            i += consumeStrengthSet(bytes, i);
            break;
        default:
            qDebug() << "No consumer for for type " << type;
            // Consume the reported length
            i += length;
            continue;
        }

        int expectedPosition = initialPosition + length + 1;
        if (i != expectedPosition) {
            // Should never happen unless there's a bug in one of the consumers
            qDebug() << "Unexpected position " << i << ", expected "  << expectedPosition << ", after consuming " << type;
        }
    }

    //if (!unknownTypeCodes.isEmpty()) {
    //    for (final Map.Entry<Byte, Integer> e : unknownTypeCodes.entrySet()) {
    //        LOG.warn("Unknown type code {} seen {} times", String.format("0x%X", e.getKey()), e.getValue());
    //    }
    //}
}

int ZeppOsActivityDetailParser::typeLength(Type t)
{
    return m_types[t];
}

int ZeppOsActivityDetailParser::consumeTimestamp(const QByteArray &bytes, int offset)
{
    qDebug() << Q_FUNC_INFO;
    int unused = TypeConversion::toInt32(bytes[offset], bytes[offset + 1], bytes[offset + 2], bytes[offset + 3]);
    int64_t ts = TypeConversion::toInt64(bytes[offset], bytes[offset + 1], bytes[offset + 2], bytes[offset + 3], bytes[offset + 4], bytes[offset + 5], bytes[offset + 6], bytes[offset + 7]);
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(ts);
    m_lastTimestamp = dt;
    m_offset = 0;

    return 12;
}

int ZeppOsActivityDetailParser::consumeTimestampOffset(const QByteArray &bytes, int offset)
{
    qDebug() << Q_FUNC_INFO;
    short tso = TypeConversion::toInt16(bytes[offset], bytes[offset + 1]);
    m_offset = tso;
    return 2;
}

int ZeppOsActivityDetailParser::consumeGpsCoords(const QByteArray &bytes, int offset)
{
    qDebug() << Q_FUNC_INFO;
    // TODO which one is the time offset? Not sure it is the first

    m_baseLongitude = TypeConversion::toInt32(bytes[offset + 6], bytes[offset + 7], bytes[offset + 8], bytes[offset + 9]);
    m_baseLatitude = TypeConversion::toInt32(bytes[offset + 10], bytes[offset + 11], bytes[offset + 12], bytes[offset + 13]);

    addNewGpsCoordinate();

    return 20;
}

int ZeppOsActivityDetailParser::consumeGpsDelta(const QByteArray &bytes, int offset)
{
    qDebug() << Q_FUNC_INFO;
    offset += consumeTimestampOffset(bytes, offset);
    short longitudeDelta = TypeConversion::toInt16(bytes[offset], bytes[offset + 1]);
    short latitudeDelta = TypeConversion::toInt16(bytes[offset + 2], bytes[offset + 3]);
    short two = TypeConversion::toInt16(bytes[offset + 4], bytes[offset + 5]);; // ? seems to always be 2

    m_baseLongitude += longitudeDelta;
    m_baseLatitude += latitudeDelta;

    addNewGpsCoordinate();

    return 8;
}

int ZeppOsActivityDetailParser::consumeStatus(const QByteArray &bytes, int offset)
{
    qDebug() << Q_FUNC_INFO;
    offset += consumeTimestampOffset(bytes, offset);

    int statusCode = TypeConversion::toInt16(bytes[offset], bytes[offset + 1]);
    QString status;
    switch (statusCode) {
    case 1:
        status = "start";
        break;
    case 4:
        status = "pause";
        //activityTrack.startNewSegment();
        break;
    case 5:
        status = "resume";
        //activityTrack.startNewSegment();
        break;
    case 6:
        status = "stop";
        break;
    default:
        status = QString("unknown (%1)").arg(statusCode);
    }

    return 4;
}

int ZeppOsActivityDetailParser::consumeSpeed(const QByteArray &bytes, int offset)
{
    qDebug() << Q_FUNC_INFO;
    offset += consumeTimestampOffset(bytes, offset);

    short cadence = TypeConversion::toInt16(bytes[offset], bytes[offset + 1]); // spm
    short stride = TypeConversion::toInt16(bytes[offset + 2], bytes[offset + 3]); // cm
    short pace = TypeConversion::toInt16(bytes[offset + 4], bytes[offset + 5]); // sec/km

    //TODO
    //if (ap != null) {
    //    ap.setCadence(cadence);
    //    if (pace != 0) {
    //        ap.setSpeed(1000f / pace); // s/km -> m/s
    //    }
    //}
    return 8;
}

int ZeppOsActivityDetailParser::consumeAltitude(const QByteArray &bytes, int offset)
{
    qDebug() << Q_FUNC_INFO;
    offset += consumeTimestampOffset(bytes, offset);
    m_baseAltitude = (int) (TypeConversion::toInt32(bytes[offset], bytes[offset + 1], bytes[offset + 2], bytes[offset + 3]) / 100.0f);

    return 6;
}

int ZeppOsActivityDetailParser::consumeHeartRate(const QByteArray &bytes, int offset)
{
    qDebug() << Q_FUNC_INFO;
    offset += consumeTimestampOffset(bytes, offset);
    m_lastHeartrate = bytes[offset] & 0xff;

    return 3;
}

int ZeppOsActivityDetailParser::consumeStrengthSet(const QByteArray &bytes, int offset)
{
    qDebug() << Q_FUNC_INFO;
    return 34;
}

ZeppOsActivityDetailParser::Type ZeppOsActivityDetailParser::typeFromCode(char code)
{
    for (auto key : m_types.keys()) {
        if (key == (Type)code) {
            return key;
        }
    }

    return UNKNOWN;
}

ActivityCoordinate ZeppOsActivityDetailParser::getCurrentActivityPoint()
{
    qDebug() << Q_FUNC_INFO;
    // Round to the nearest second
    long currentTime = m_lastTimestamp.toMSecsSinceEpoch() + m_offset;

    if (currentTime - m_lastActivityPoint.timeStamp().toMSecsSinceEpoch() > 500) {
        addNewGpsCoordinate();
    }

    return m_lastActivityPoint;
}

void ZeppOsActivityDetailParser::addNewGpsCoordinate()
{

    m_lastCoordinate.setLongitude(convertHuamiValueToDecimalDegrees(m_baseLongitude));
    m_lastCoordinate.setLatitude(convertHuamiValueToDecimalDegrees(m_baseLatitude));
    m_lastCoordinate.setAltitude(m_baseAltitude);

    ActivityCoordinate ac;

    ac.setCoordinate(m_lastCoordinate);
    ac.setHeartRate(m_lastHeartrate);
    ac.setTimeStamp(m_lastTimestamp.addMSecs(m_offset));

    add(ac);
}

void ZeppOsActivityDetailParser::add(const ActivityCoordinate &ap)
{
    qDebug() << Q_FUNC_INFO;
    if (ap.coordinate() != m_lastActivityPoint.coordinate()) {
        m_activityTrack << ap;
    } else {
        qDebug() << Q_FUNC_INFO << "skipping point!";
    }
}
