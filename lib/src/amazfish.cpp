#include "amazfish.h"

QString Amazfish::activityToString(Amazfish::ActivityType type)
{
    if (type == Amazfish::ActivityType::NotMeasured) {
        return "NotMeasured";
    }
    if (type == Amazfish::ActivityType::Unknown) {
        return "Unknown";
    }
    if (type == Amazfish::ActivityType::Activity) {
        return "Activity";
    }
    if (type == Amazfish::ActivityType::LightSleep) {
        return "LightSleep";
    }
    if (type == Amazfish::ActivityType::DeepSleep) {
        return "DeepSleep";
    }
    if (type == Amazfish::ActivityType::NotWorn) {
        return "NotWorn";
    }
    if (type == Amazfish::ActivityType::Running) {
        return "Running";
    }
    if (type == Amazfish::ActivityType::Walking) {
        return "Walking";
    }
    if (type == Amazfish::ActivityType::Swimming) {
        return "Swimming";
    }
    if (type == Amazfish::ActivityType::Cycling) {
        return "Cycling";
    }
    if (type == Amazfish::ActivityType::Treadmill) {
        return "Treadmill";
    }
    if (type == Amazfish::ActivityType::Exercise) {
        return "Exercise";
    }
    if (type == Amazfish::ActivityType::OpenSwimming) {
        return "Open Swimming";
    }
    if (type == Amazfish::ActivityType::IndoorCycling) {
        return "Indoor Cycling";
    }
    if (type == Amazfish::ActivityType::EllipticalTrainer) {
        return "Eliptical Trainer";
    }
    if (type == Amazfish::ActivityType::JumpRope) {
        return "Jump Rope";
    }
    if (type == Amazfish::ActivityType::Yoga) {
        return "Yoga";
    }
    if (type == Amazfish::ActivityType::TrailRunning) {
        return "Trail Running";
    }
    if (type == Amazfish::ActivityType::Skiing) {
        return "Skiing";
    }

    return "Unknown";
}
