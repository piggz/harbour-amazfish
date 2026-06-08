#include "vivosmart5.h"



Vivosmart5Device::Vivosmart5Device(const QString &pairedName, QObject *parent) : GarminDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO << pairedName;
    connect(this, &QBLEDevice::propertiesChanged, this, &GarminDevice::onPropertiesChanged, Qt::UniqueConnection);
}

Amazfish::Features Vivosmart5Device::supportedFeatures() const
{
    return  Amazfish::Feature::FEATURE_NONE
        | Amazfish::Feature::FEATURE_HRM
        //| Amazfish::Feature::FEATURE_ACTIVITY
        | Amazfish::Feature::FEATURE_STEPS
        // | Amazfish::Feature::FEATURE_ALARMS
        | Amazfish::Feature::FEATURE_ALERT
        // | Amazfish::Feature::FEATURE_EVENT_REMINDER
        // | Amazfish::Feature::FEATURE_MUSIC_CONTROL
        // | Amazfish::Feature::FEATURE_BUTTON_ACTION
        // | Amazfish::Feature::FEATURE_SCREENSHOT
        // | Amazfish::Feature::FEATURE_FILE_INSTALL
        | Amazfish::Feature::FEATURE_SPO2
        ;
}


Amazfish::DataTypes Vivosmart5Device::supportedDataTypes() const
{
    return Amazfish::DataType::TYPE_SPO2
            | Amazfish::DataType::TYPE_HRV
            | Amazfish::DataType::TYPE_HEART_RATE
            | Amazfish::DataType::TYPE_ACTIVITY
            ;

}

QString Vivosmart5Device::deviceType() const
{
    return "Garmin Vivosmart 5";
}

