#include "amazfish.h"

QString Amazfish::activityToString(Amazfish::ActivityType type)
{
    switch (type) {
    case Amazfish::ActivityType::NotMeasured:
        return "NotMeasured";
    case Amazfish::ActivityType::Unknown:
        return "Unknown";
    case Amazfish::ActivityType::Activity:
        return "Activity";
    case Amazfish::ActivityType::LightSleep:
        return "LightSleep";
    case Amazfish::ActivityType::DeepSleep:
        return "DeepSleep";
    case Amazfish::ActivityType::NotWorn:
        return "NotWorn";
    case Amazfish::ActivityType::Running:
        return "Running";
    case Amazfish::ActivityType::Walking:
        return "Walking";
    case Amazfish::ActivityType::Swimming:
        return "Swimming";
    case Amazfish::ActivityType::Cycling:
        return "Cycling";
    case Amazfish::ActivityType::Treadmill:
        return "Treadmill";
    case Amazfish::ActivityType::Exercise:
        return "Exercise";
    case Amazfish::ActivityType::OpenSwimming:
        return "Open Swimming";
    case Amazfish::ActivityType::IndoorCycling:
        return "Indoor Cycling";
    case Amazfish::ActivityType::EllipticalTrainer:
        return "Elliptical Trainer";
    case Amazfish::ActivityType::JumpRope:
        return "Jump Rope";
    case Amazfish::ActivityType::Yoga:
        return "Yoga";
    case Amazfish::ActivityType::TrailRunning:
        return "Trail Running";
    case Amazfish::ActivityType::Skiing:
        return "Skiing";
    default:
        return "Unknown";
    }
}
