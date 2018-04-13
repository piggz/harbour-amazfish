#ifndef MIBAND2SERVICE_H
#define MIBAND2SERVICE_H

#include "bipservice.h"

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

class MiBand2Service : public BipService
{
public:
    MiBand2Service(QObject *parent);

    static const char* UUID_SERVICE_MIBAND2;
    static const char* UUID_CHARACTERISITIC_MIBAND2_AUTH;

};

#endif // MIBAND2SERVICE_H
