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
        return Biking;
    } else if (type == 8) {
        return IndoorBiking;
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
    if (type == Biking) {
        return "Biking";
    }
    if (type == Treadmill) {
        return "Treadmill";
    }
    if (type == IndoorBiking) {
        return "Indoor Biking";
    }
}
