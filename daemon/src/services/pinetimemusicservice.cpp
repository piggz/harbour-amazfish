#include "pinetimemusicservice.h"

const char* PineTimeMusicService::UUID_SERVICE_MUSIC = "c7e50001-00fc-48fe-8e23-433b3a1942d0";
const char* PineTimeMusicService::UUID_CHARACTERISTIC_MUSIC_EVENT = "c7e50002-00fc-48fe-8e23-433b3a1942d0";
const char* PineTimeMusicService::UUID_CHARACTERISTIC_MUSIC_STATUS = "c7e50003-00fc-48fe-8e23-433b3a1942d0";
const char* PineTimeMusicService::UUID_CHARACTERISTIC_MUSIC_ARTIST = "c7e50004-00fc-48fe-8e23-433b3a1942d0";
const char* PineTimeMusicService::UUID_CHARACTERISTIC_MUSIC_TRACK = "c7e50005-00fc-48fe-8e23-433b3a1942d0";
const char* PineTimeMusicService::UUID_CHARACTERISTIC_MUSIC_ALBUM = "c7e50006-00fc-48fe-8e23-433b3a1942d0";

PineTimeMusicService::PineTimeMusicService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_MUSIC, path, parent)
{
    qDebug() << Q_FUNC_INFO;
    connect(this, &QBLEService::characteristicChanged, this, &PineTimeMusicService::characteristicChanged);
}

void PineTimeMusicService::setArtist(const QString &artist)
{
    writeValue(UUID_CHARACTERISTIC_MUSIC_ARTIST, artist.toLocal8Bit());
}

void PineTimeMusicService::setTrack(const QString &track)
{
    writeValue(UUID_CHARACTERISTIC_MUSIC_TRACK, track.toLocal8Bit());
}

void PineTimeMusicService::setAlbum(const QString &album)
{
    writeValue(UUID_CHARACTERISTIC_MUSIC_ALBUM, album.toLocal8Bit());
}

void PineTimeMusicService::characteristicChanged(const QString &c, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << c << value;

}

