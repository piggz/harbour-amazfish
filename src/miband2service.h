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
    MiBand2Service(QObject *parent);

    static const char* UUID_SERVICE_MIBAND2;
    static const char* UUID_CHARACTERISITIC_MIBAND2_AUTH;

    const char AUTH_SEND_KEY = 0x01;
    const char AUTH_REQUEST_RANDOM_AUTH_NUMBER = 0x02;
    const char AUTH_SEND_ENCRYPTED_AUTH_NUMBER = 0x03;
    const char AUTH_RESPONSE = 0x10;
    const char AUTH_SUCCESS = 0x01;
    const char AUTH_FAIL = 0x04;
    const char AUTH_BYTE = 0x08;
    const QByteArray AUTH_SECRET_KEY = "0123456789@ABCDE";

    void initialise();

    Q_SIGNAL void authenticated();

private:
    Q_SLOT void characteristicChanged(const QString &characteristic, const QByteArray &value);
//    Q_SLOT void serviceReady(bool r);

    QByteArray handleAesAuth(QByteArray data, QByteArray secretKey);
};

#endif // MIBAND2SERVICE_H
