#ifndef PINETIMEMUSICSERVICE_H
#define PINETIMEMUSICSERVICE_H

#include <QObject>
#include "qble/qbleservice.h"
#include "abstractdevice.h"

/** c7e50001-00fc-48fe-8e23-433b3a1942d0 --Music Service
 * "c7e50002-00fc-48fe-8e23-433b3a1942d0", --Status
 * "c7e50003-00fc-48fe-8e23-433b3a1942d0", --Event
 * "c7e50004-00fc-48fe-8e23-433b3a1942d0", --Artist
 * "c7e50005-00fc-48fe-8e23-433b3a1942d0", --Track
 * "c7e50006-00fc-48fe-8e23-433b3a1942d0"  --Album
 */
class PineTimeMusicService : public QBLEService
{
    Q_OBJECT
public:
    PineTimeMusicService(const QString &path, QObject *parent);

    static const char *UUID_SERVICE_MUSIC;
    static const char *UUID_CHARACTERISTIC_MUSIC_EVENT;
    static const char *UUID_CHARACTERISTIC_MUSIC_STATUS;
    static const char *UUID_CHARACTERISTIC_MUSIC_ARTIST;
    static const char *UUID_CHARACTERISTIC_MUSIC_TRACK;
    static const char *UUID_CHARACTERISTIC_MUSIC_ALBUM;

    static const uint8_t EVENT_MUSIC_OPEN = 0xe0;
    static const uint8_t EVENT_MUSIC_PLAY = 0x00;
    static const uint8_t EVENT_MUSIC_PAUSE = 0x01;
    static const uint8_t EVENT_MUSIC_NEXT = 0x03;
    static const uint8_t EVENT_MUSIC_PREV = 0x04;
    static const uint8_t EVENT_MUSIC_VOLUP = 0x05;
    static const uint8_t EVENT_MUSIC_VOLDOWN = 0x06;

    void setStatus(bool playing);
    void setArtist(const QString &artist);
    void setTrack(const QString &track);
    void setAlbum(const QString &album);

    Q_SIGNAL void serviceEvent(char event);


private:
    void characteristicChanged(const QString &c, const QByteArray &value);


};

#endif // PINETIMEMUSICSERVICE_H
