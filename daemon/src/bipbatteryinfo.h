#ifndef BIPBATTERYINFO_H
#define BIPBATTERYINFO_H

#include <QByteArray>
#include <QDateTime>

class BipBatteryInfo
{
public:
    BipBatteryInfo();
    void setData(const QByteArray &data);
    static const char DEVICE_BATTERY_NORMAL = 0;
    static const char DEVICE_BATTERY_CHARGING = 1;

    int state() const;
    int currentChargeLevelPercent() const;
    int lastChargeLevelPercent() const;
    QDateTime lastChargeTime() const;
    int numCharges() const;

private:
    QByteArray m_data;
};

#endif // BIPBATTERYINFO_H
