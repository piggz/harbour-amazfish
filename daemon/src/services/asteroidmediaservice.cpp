#include "asteroidmediaservice.h"
#include "typeconversion.h"

const char* AsteroidMediaService::UUID_SERVICE_MEDIA                = "00007071-0000-0000-0000-00a57e401d05";
const char* AsteroidMediaService::UUID_CHARACTERISTIC_MEDIA_TITLE   = "00007001-0000-0000-0000-00a57e401d05";
const char* AsteroidMediaService::UUID_CHARACTERISTIC_MEDIA_ALBUM   = "00007002-0000-0000-0000-00a57e401d05";
const char* AsteroidMediaService::UUID_CHARACTERISTIC_MEDIA_ARTIST  = "00007003-0000-0000-0000-00a57e401d05";
const char* AsteroidMediaService::UUID_CHARACTERISTIC_MEDIA_STATUS  = "00007004-0000-0000-0000-00a57e401d05";
const char* AsteroidMediaService::UUID_CHARACTERISTIC_MEDIA_COMMAND = "00007005-0000-0000-0000-00a57e401d05";
const char* AsteroidMediaService::UUID_CHARACTERISTIC_MEDIA_VOLUME  = "00007006-0000-0000-0000-00a57e401d05";

AsteroidMediaService::AsteroidMediaService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_MEDIA, path, parent)
{
    qDebug() << Q_FUNC_INFO;
    setVolume(50);
    connect(this, &QBLEService::characteristicChanged, this, &AsteroidMediaService::characteristicChanged);
}

void AsteroidMediaService::setStatus(bool playing)
{
    writeValue(UUID_CHARACTERISTIC_MEDIA_STATUS, playing ? QByteArray(1, char(0x01)) : QByteArray(1, char(0x00)));
}

void AsteroidMediaService::setArtist(const QString &artist)
{
    writeValue(UUID_CHARACTERISTIC_MEDIA_ARTIST, artist.toLocal8Bit());
}

void AsteroidMediaService::setTrack(const QString &track)
{
    writeValue(UUID_CHARACTERISTIC_MEDIA_TITLE, track.toLocal8Bit());
}

void AsteroidMediaService::setAlbum(const QString &album)
{
    writeValue(UUID_CHARACTERISTIC_MEDIA_ALBUM, album.toLocal8Bit());
}

void AsteroidMediaService::setVolume(const int volume)
{
    writeValue(UUID_CHARACTERISTIC_MEDIA_VOLUME, TypeConversion::fromInt8(volume));
}

void AsteroidMediaService::characteristicChanged(const QString &c, const QByteArray &value)
{
//    qDebug() << Q_FUNC_INFO << c << value;
    emit serviceEvent(c, value[0], value[1]);
}

