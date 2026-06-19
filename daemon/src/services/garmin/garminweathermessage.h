#ifndef GARMINWEATHERMESSAGE_H
#define GARMINWEATHERMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"

#include <QObject>

class GarminWeatherMessage : public GarminGfdiMessage
{
public:
    GarminWeatherMessage(CommunicatorV2 *parent) {
        mCommunicator = parent;
    }
    void parse(const QByteArray& data);
    QByteArray generateResponse(const QByteArray& data);
    QByteArray encodeWeather(const CurrentWeather *weather);
};

#endif // GARMINWEATHERMESSAGE_H
