#include "activitykind.h"
#include <QDebug>
ActivityKind::Type ActivityKind::fromBipType(int type)
{
    qDebug() << "Checking for bip activity type: " << type;
    if (type == 1) {
        return Running;
    } else if (type == 2) {
        return Treadmill;
    } else if (type == 3) {
        return Walking;
    } else if (type == 4) {
        return Cycling;
    } else if (type == 5) {
        return Exercise;
    } else if (type == 6) {
        return Swimming;
    } else if (type == 7) {
        return OpenSwimming;
    } else if (type == 8) {
        return IndoorCycling;
    } else if (type == 9) {
        return EllipticalTrainer;
    } else if (type == 21) {
        return JumpRope;
    } else if (type == 60) {
        return Yoga;
    } else if (type == 11) {
        return TrailRunning;
    } else if (type == 105) {
        return Skiing;
    }
    return Activity;
}

QString ActivityKind::toString(Type type)
{
    if (type == NotMeasured) {
        return "NotMeasured";
    }
    if (type == Unknown) {
        return "Unknown";
    }
    if (type == Activity) {
        return "Activity";
    }
    if (type == LightSleep) {
        return "LightSleep";
    }
    if (type == DeepSleep) {
        return "DeepSleep";
    }
    if (type == NotWorn) {
        return "NotWorn";
    }
    if (type == Running) {
        return "Running";
    }
    if (type == Walking) {
        return "Walking";
    }
    if (type == Swimming) {
        return "Swimming";
    }
    if (type == Cycling) {
        return "Cycling";
    }
    if (type == Treadmill) {
        return "Treadmill";
    }
    if (type == Exercise) {
        return "Exercise";
    }
    if (type == OpenSwimming) {
        return "Open Swimming";
    }
    if (type == IndoorCycling) {
        return "Indoor Cycling";
    }
    if (type == EllipticalTrainer) {
        return "Eliptical Trainer";
    }
    if (type == JumpRope) {
        return "Jump Rope";
    }
    if (type == Yoga) {
        return "Yoga";
    }
    if (type == TrailRunning) {
        return "Trail Running";
    }
    if (type == Skiing) {
        return "Skiing";
    }

    return "Unknown";
}
