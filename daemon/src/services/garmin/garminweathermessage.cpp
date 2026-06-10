#include "garminweathermessage.h"



void GarminWeatherMessage::parse(const QByteArray& data) {
    qDebug() << Q_FUNC_INFO << "Garmin: parsing weather request";

    if (data.size() < 10) {
        return;
    }
    WeatherRequestMessage msg;
    msg.format = quint8(data[0]);
    msg.latitude = i32le(data, 1);
    msg.longitude = i32le(data, 5);
    msg.hoursOfForecast = quint8(data[9]);
    if (mCommunicator) mCommunicator->onWeatherRequestReceived(msg);
}

QByteArray GarminWeatherMessage::generateResponse(const QByteArray& data) {

}

QByteArray GarminWeatherMessage::encodeWeather(const CurrentWeather *weather) {

}
