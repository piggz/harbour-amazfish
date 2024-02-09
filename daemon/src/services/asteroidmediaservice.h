#ifndef ASTEROID_MEDIA_SERVICE_H
#define ASTEROID_MEDIA_SERVICE_H

#include <QObject>
#include "qble/qbleservice.h"
#include "abstractdevice.h"

class AsteroidMediaService : public QBLEService
{
    Q_OBJECT
public:
    AsteroidMediaService(const QString &path, QObject *parent);

    static const char* UUID_SERVICE_MEDIA;
    static const char* UUID_CHARACTERISTIC_MEDIA_TITLE;
    static const char* UUID_CHARACTERISTIC_MEDIA_ALBUM;
    static const char* UUID_CHARACTERISTIC_MEDIA_ARTIST;
    static const char* UUID_CHARACTERISTIC_MEDIA_STATUS;
    static const char* UUID_CHARACTERISTIC_MEDIA_COMMAND;
    static const char* UUID_CHARACTERISTIC_MEDIA_VOLUME;

    static const uint8_t EVENT_MEDIA_PREV = 0x00;
    static const uint8_t EVENT_MEDIA_NEXT = 0x01;
    static const uint8_t EVENT_MEDIA_PLAY = 0x02;
    static const uint8_t EVENT_MEDIA_PAUSE = 0x03;
    static const uint8_t EVENT_MEDIA_VOL = 0x04;

    void setStatus(bool playing);
    void setArtist(const QString &artist);
    void setTrack(const QString &track);
    void setAlbum(const QString &album);
    void setVolume(const int volume);

    Q_SIGNAL void serviceEvent(const QString &c, uint8_t event, uint8_t data);

private:
    void characteristicChanged(const QString &c, const QByteArray &value);


};

#endif // ASTEROID_MEDIA_SERVICE_H