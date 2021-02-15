#include "pinetimemusicservice.h"

const char* PineTimeMusicService::UUID_SERVICE_MUSIC = "00000000-78fc-48fe-8e23-433b3a1942d0";
const char* PineTimeMusicService::UUID_CHARACTERISTIC_MUSIC_EVENT = "00000001-78fc-48fe-8e23-433b3a1942d0";
const char* PineTimeMusicService::UUID_CHARACTERISTIC_MUSIC_STATUS = "00000002-78fc-48fe-8e23-433b3a1942d0";
const char* PineTimeMusicService::UUID_CHARACTERISTIC_MUSIC_ARTIST = "00000003-78fc-48fe-8e23-433b3a1942d0";
const char* PineTimeMusicService::UUID_CHARACTERISTIC_MUSIC_TRACK = "00000004-78fc-48fe-8e23-433b3a1942d0";
const char* PineTimeMusicService::UUID_CHARACTERISTIC_MUSIC_ALBUM = "00000005-78fc-48fe-8e23-433b3a1942d0";

PineTimeMusicService::PineTimeMusicService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_MUSIC, path, parent)
{
    qDebug() << Q_FUNC_INFO;
    connect(this, &QBLEService::characteristicChanged, this, &PineTimeMusicService::characteristicChanged);
}

void PineTimeMusicService::setStatus(bool playing)
{
    writeValue(UUID_CHARACTERISTIC_MUSIC_STATUS, playing ? QByteArray(1, char(0x01)) : QByteArray(1, char(0x00)));
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
    emit serviceEvent(c, value[0]);
}

