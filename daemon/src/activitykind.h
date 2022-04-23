#ifndef ACTIVITYKIND_H
#define ACTIVITYKIND_H

#include <QString>

class ActivityKind
{
    public:
    enum Type {
        NotMeasured = -1,
        Unknown = 0x00000000,
        Activity = 0x00000001,
        LightSleep = 0x00000002,
        DeepSleep = 0x00000004,
        NotWorn = 0x00000008,
        Running = 0x00000010,
        Walking = 0x00000020,
        Swimming = 0x00000040,
        Cycling = 0x00000080,
        Treadmill = 0x00000100,
        Exercise = 0x00000200,
        OpenSwimming = 0x00000400,
        IndoorCycling = 0x00000800,
        EllipticalTrainer = 0x00001000,
        JumpRope = 0x00002000,
        Yoga = 0x00004000,
        TrailRunning = 0x00008000,
        Skiing = 0x00010000
    };
    
    static Type fromBipType(int type);
    static QString toString(Type type);
 
 };

#endif
