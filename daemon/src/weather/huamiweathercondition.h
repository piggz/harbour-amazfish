#ifndef HUAMIWEATHERCONDITION_H
#define HUAMIWEATHERCONDITION_H


class HuamiWeatherCondition
{
public:
    static const char CLEAR_SKY = 0;
    static const char SCATTERED_CLOUDS = 1;
    static const char CLOUDY = 2;
    static const char RAIN_WITH_SUN = 3;
    static const char THUNDERSTORM = 4;
    static const char HAIL = 5;
    static const char RAIN_AND_SNOW = 6;
    static const char LIGHT_RAIN = 7;
    static const char MEDIUM_RAIN = 8;
    static const char HEAVY_RAIN = 9;
    static const char EXTREME_RAIN = 10;
    static const char SUPER_EXTREME_RAIN = 11;
    static const char TORRENTIAL_RAIN = 12;
    static const char SNOW_AND_SUN = 13;
    static const char LIGHT_SNOW = 14;
    static const char MEDIUM_SNOW = 15;
    static const char HEAVY_SNOW = 16;
    static const char EXTREME_SNOW = 17;
    static const char MIST = 18;
    static const char DRIZZLE = 19;
    static const char WIND_AND_RAIN = 20;
    // 21- various types of rain

    static char mapToAmazfitBipWeatherCode(int openWeatherMapCondition);
};

#endif // HUAMIWEATHERCONDITION_H
