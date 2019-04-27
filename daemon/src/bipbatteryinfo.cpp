#include "bipbatteryinfo.h"
#include "typeconversion.h"

BipBatteryInfo::BipBatteryInfo()
{
}

void BipBatteryInfo::setData(const QByteArray &data)
{
    m_data = data;
}

int BipBatteryInfo::state() const
{
    if (m_data.length() >= 3) {
        int value = m_data[2];
        switch (value) {
        case DEVICE_BATTERY_NORMAL:
            return DEVICE_BATTERY_NORMAL;
        case DEVICE_BATTERY_CHARGING:
            return DEVICE_BATTERY_CHARGING;
            //                case DEVICE_BATTERY_CHARGING:
            //                    return BatteryState.BATTERY_CHARGING;
            //                case DEVICE_BATTERY_CHARGING_FULL:
            //                    return BatteryState.BATTERY_CHARGING_FULL;
            //                case DEVICE_BATTERY_CHARGE_OFF:
            //                    return BatteryState.BATTERY_NOT_CHARGING_FULL;
        }
    }
    return -1;
}


int BipBatteryInfo::currentChargeLevelPercent() const
{
    if (m_data.length() >= 2) {
        return m_data[1];
    }
    return 50; // actually unknown
}

int BipBatteryInfo::lastChargeLevelPercent() const
{
    if (m_data.length() >= 20) {
        return m_data[19];
    }
    return 50; // actually unknown
}

int BipBatteryInfo::numCharges() const
{
    return -1;
}

QDateTime BipBatteryInfo::lastChargeTime() const
{
    QDateTime lastCharge;

    if (m_data.length() >= 18) {
        lastCharge = TypeConversion::rawBytesToDateTime(m_data.mid(10, 8), true);
    }

    return lastCharge;
}
