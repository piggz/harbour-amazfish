#include "daemoninterface.h"

DaemonInterface::DaemonInterface(QObject *parent) : QObject(parent)
{

}

QString DaemonInterface::activityToString(ActivityType type)
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

    return "Unknown";
}
