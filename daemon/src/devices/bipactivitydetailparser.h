#ifndef BIPACTIVITYDETAILPARSER_H
#define BIPACTIVITYDETAILPARSER_H

#include "activitycoordinate.h"
#include "abstractactivitydetailparser.h"

class BipActivityDetailParser : public AbstractActivityDetailParser
{
public:
    explicit BipActivityDetailParser();

    void parse(const QByteArray &bytes) override;

private:
    static const char TYPE_GPS = 0x00;
    static const char TYPE_HR = 0x01;
    static const char TYPE_UNKNOWN2 = 0x02;
    static const char TYPE_PAUSE = 0x03;
    static const char TYPE_SPEED4 = 0x04;
    static const char TYPE_SPEED5 = 0x05;
    static const char TYPE_GPS_SPEED6 = 0x06;
    static const char TYPE_SWIMMING = 0x08;

    int m_baseLongitude = 0;
    int m_baseLatitude = 0;
    int m_baseAltitude = 0;
    QDateTime m_baseDate;
    bool m_skipCounterByte = false;
    int m_lastHeartrate = 0;
    QGeoCoordinate m_lastCoordinate;
    ActivityCoordinate m_lastActivityPoint;
    QList<ActivityCoordinate> m_tempTrack;

    bool getSkipCounterByte();
    void setSkipCounterByte(bool skip);
    int consumeGPSAndUpdateBaseLocation(const QByteArray &bytes, int offset, long timeOffset);
    int consumeHeartRate(const QByteArray &bytes, int offset, long timeOffsetSeconds);
    ActivityCoordinate getActivityPointFor(long timeOffsetSeconds);
    QDateTime makeAbsolute(long timeOffsetSeconds);
    void add(const ActivityCoordinate &ap);
    int consumeUnknown2(const QByteArray &bytes, int offset);
    int consumePause(const QByteArray &bytes, int i);
    int consumeSpeed4(const QByteArray &bytes, int offset);
    int consumeSpeed5(const QByteArray &bytes, int offset);
    int consumeSpeed6(const QByteArray &bytes, int offset);
    int consumeSwimming(const QByteArray &bytes, int offset);

};

#endif // BIPACTIVITYDETAILPARSER_H
