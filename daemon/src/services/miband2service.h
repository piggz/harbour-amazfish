#ifndef MIBAND2SERVICE_H
#define MIBAND2SERVICE_H

#include "qble/qbleservice.h"

/*
{0000FEE1-0000-1000-8000-00805f9b34fb} MiBand2 Service
--00000009-0000-3512-2118-0009af100700 //Auth
--0000fedd-0000-1000-8000-00805f9b34fb //Unknown
--0000fede-0000-1000-8000-00805f9b34fb //Unknown
--0000fedf-0000-1000-8000-00805f9b34fb //Unknown
--0000fed0-0000-1000-8000-00805f9b34fb //Unknown
--0000fed1-0000-1000-8000-00805f9b34fb //Unknown
--0000fed2-0000-1000-8000-00805f9b34fb //Unknown
--0000fed3-0000-1000-8000-00805f9b34fb //Unknown
*/

class MiBand2Service : public QBLEService
{
    Q_OBJECT
public:
    MiBand2Service(const QString &path, char authByte, char crypyByte, bool requireAuthKey, QObject *parent);

    static const char* UUID_SERVICE_MIBAND2;
    static const char* UUID_CHARACTERISITIC_MIBAND2_AUTH;

    const char AUTH_SEND_KEY = 0x01;
    const char AUTH_REQUEST_RANDOM_AUTH_NUMBER = 0x02;
    const char AUTH_SEND_ENCRYPTED_AUTH_NUMBER = 0x03;
    const char RESPONSE = 0x10;
    const char SUCCESS = 0x01;
    const char FAIL = 0x04;
    const char AUTH_BYTE = 0x00; //0x08;
    const QByteArray AUTH_SECRET_KEY = "0123456789@ABCDE";

    void initialise(bool firstTime);

    Q_SIGNAL void authenticated(bool ready);


private:
    Q_SLOT void characteristicChanged(const QString &characteristic, const QByteArray &value);

    QByteArray handleAesAuth(QByteArray data, QByteArray secretKey);

    QByteArray getSecretKey();
    QByteArray requestAuthNumber();

    char m_authByte = 0x08; //0x08 = Bip, 0x00 = GTS
    char m_cryptByte = 0x00; //0x00 = Bip, 0x80 = GTS/BipLite
    bool m_requiresAuthKey = false;
};

#endif // MIBAND2SERVICE_H
