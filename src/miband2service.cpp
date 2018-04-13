#include "miband2service.h"

const char* MiBand2Service::UUID_SERVICE_MIBAND2  = "{0000FEE1-0000-1000-8000-00805f9b34fb}";
const char* MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH = "{00000009-0000-3512-2118-0009af100700}";

MiBand2Service::MiBand2Service(QObject *parent) : BipService("{0000FEE1-0000-1000-8000-00805f9b34fb}", parent)
{

}
