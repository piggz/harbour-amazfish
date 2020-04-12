#ifndef ACTIVITYKIND_H
#define ACTIVITYKIND_H

#include <QString>

class ActivityKind
{
    public:
    enum Type {
        NotMeasured = -1,
        Unknown = 0,
        Activity = 1,
        LightSleep = 2,
        DeepSleep = 4,
        NotWorn = 8,
        Running = 16,
        Walking = 32,
        Swimming = 64,
        Biking = 128,
        Treadmill = 256,
        IndoorBiking = 512
    };
    
    static Type fromBipType(int type);
    static QString toString(Type type);
 
 };

#endif
