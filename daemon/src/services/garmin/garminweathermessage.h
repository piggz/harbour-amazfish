#ifndef GARMINWEATHERMESSAGE_H
#define GARMINWEATHERMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"

#include <QByteArray>
#include <QObject>

// The two GFDI payloads a watch needs before it can show a weather update:
// the FIT record definitions first, then the records themselves.
struct FitWeatherFrames {
    QByteArray fitDefinition;
    QByteArray fitData;
};

class GarminWeatherMessage : public GarminGfdiMessage
{
public:
    GarminWeatherMessage(CommunicatorV2 *parent) {
        mCommunicator = parent;
    }
    void parse(const QByteArray& data);

    // Builds the frames from the phone's weather data. Returns empty frames
    // when there is nothing usable, so the caller can skip the exchange.
    static FitWeatherFrames encodeWeather(const CurrentWeather* weather);
};

#endif // GARMINWEATHERMESSAGE_H
