#include "zepposbatteryservice.h"
#include "huami/zepposdevice.h"

ZeppOsBatteryService::ZeppOsBatteryService(ZeppOSDevice *device) : AbstractZeppOsService(device, false)
{
    m_endpoint = 0x0029;
}

QString ZeppOsBatteryService::name() const
{
    return "Battery";
}

// TODO extract this to its own class
struct HuamiBatteryInfo{
    QByteArray data;
};

void ZeppOsBatteryService::handlePayload(const QByteArray &payload)
{
    if (payload[0] != BATTERY_REPLY) {
        qWarning() << "Unexpected battery payload byte " << payload[0];
    }

    if (payload.size() != 21) {
        qWarning() << "Unexpected battery payload length:" << payload.size();
    }

    qDebug() << "payload:" << payload.toHex();

    const auto batteryInfo = HuamiBatteryInfo{payload.mid(1)};

    // Assuming the second byte, see HuamiBatteryInfo#getLevelInPercent()
    int batteryLevel = batteryInfo.data[1];

    // Example for 18%:
    // 040f1200e90704190f24240ce90704190f24240c64

    m_device->informationChanged(Amazfish::Info::INFO_BATTERY, QString::number(batteryLevel));
}
