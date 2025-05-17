#ifndef ZEPPOSACTIVITYDETAILPARSER_H
#define ZEPPOSACTIVITYDETAILPARSER_H

#include "abstractactivitydetailparser.h"

class ZeppOsActivityDetailParser : public AbstractActivityDetailParser
{
public:
    ZeppOsActivityDetailParser();
    void parse(const QByteArray &bytes) override;

private:
    enum Type{
        UNKNOWN = 0,
        TIMESTAMP = 1,
        GPS_COORDS = 2,
        GPS_DELTA = 3,
        STATUS = 4,
        SPEED = 5,
        ALTITUDE = 7,
        HEARTRATE = 8,
        STRENGTH_SET = 15
    };

    QMap<Type, int> m_types;
    QMap<char, int> m_unknownTypes;

    Type typeFromCode(char code);
    int typeLength(Type t);

    int consumeTimestamp(const QByteArray &bytes, int offset);
    int consumeTimestampOffset(const QByteArray &bytes, int offset);
    int consumeGpsCoords(const QByteArray &bytes, int offset);
    int consumeGpsDelta(const QByteArray &bytes, int offset);
    int consumeStatus(const QByteArray &bytes, int offset);
    int consumeSpeed(const QByteArray &bytes, int offset);
    int consumeAltitude(const QByteArray &bytes, int offset);
    int consumeHeartRate(const QByteArray &bytes, int offset);
    int consumeStrengthSet(const QByteArray &bytes, int offset);

    ActivityCoordinate getCurrentActivityPoint();
    void addNewGpsCoordinate();
    void add(const ActivityCoordinate &ap);

    QDateTime m_lastTimestamp;
    QGeoCoordinate m_lastCoordinate;
    ActivityCoordinate m_lastActivityPoint;
    int m_lastHeartrate = 0;
    long m_offset = 0;

    long m_baseLongitude = 0;
    long m_baseLatitude = 0;
    double m_baseAltitude = 0;
};

#endif // ZEPPOSACTIVITYDETAILPARSER_H
