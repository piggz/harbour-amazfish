#ifndef BIPACTIVITYDETAILPARSER_H
#define BIPACTIVITYDETAILPARSER_H

#include "activitysummary.h"
#include "activitycoordinate.h"

class BipActivityDetailParser
{
public:
    BipActivityDetailParser(ActivitySummary summary);

    void parse(const QByteArray &bytes);
    QString toText();

private:
    static const char TYPE_GPS = 0x00;
    static const char TYPE_HR = 0x01;
    static const char TYPE_UNKNOWN2 = 0x02;
    static const char TYPE_PAUSE = 0x03;
    static const char TYPE_SPEED4 = 0x04;
    static const char TYPE_SPEED5 = 0x05;
    static const char TYPE_GPS_SPEED6 = 0x06;
    static constexpr double HUAMI_TO_DECIMAL_DEGREES_DIVISOR = 3000000.0;

    long m_baseLongitude = 0.0;
    long m_baseLatitude = 0.0;
    int m_baseAltitude = 0;
    QDateTime m_baseDate;
    bool m_skipCounterByte = false;
    ActivityCoordinate m_lastActivityPoint;
    QList<ActivityCoordinate> m_activityTrack;
    ActivitySummary m_summary;

    bool getSkipCounterByte();
    void setSkipCounterByte(bool skip);
    int consumeGPSAndUpdateBaseLocation(const QByteArray &bytes, int offset, long timeOffset);
    double convertHuamiValueToDecimalDegrees(long huamiValue);
    int consumeHeartRate(const QByteArray &bytes, int offset, long timeOffsetSeconds);
    ActivityCoordinate getActivityPointFor(long timeOffsetSeconds);
    QDateTime makeAbsolute(long timeOffsetSeconds);
    void add(const ActivityCoordinate &ap);
    int consumeUnknown2(const QByteArray &bytes, int offset);
    int consumePause(const QByteArray &bytes, int i);
    int consumeSpeed4(const QByteArray &bytes, int offset);
    int consumeSpeed5(const QByteArray &bytes, int offset);
    int consumeSpeed6(const QByteArray &bytes, int offset);

};

#endif // BIPACTIVITYDETAILPARSER_H
