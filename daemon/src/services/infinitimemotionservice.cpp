#include "infinitimemotionservice.h"
#include "typeconversion.h"

#include <QDebug>

const char* InfiniTimeMotionService::UUID_SERVICE_MOTION = "00030000-78fc-48fe-8e23-433b3a1942d0";
const char* InfiniTimeMotionService::UUID_CHARACTERISTIC_MOTION_STEPS = "00030001-78fc-48fe-8e23-433b3a1942d0";
const char* InfiniTimeMotionService::UUID_CHARACTERISTIC_MOTION_MOTION = "00030002-78fc-48fe-8e23-433b3a1942d0";

InfiniTimeMotionService::InfiniTimeMotionService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_MOTION, path, parent)
{
    qDebug() << Q_FUNC_INFO;
    connect(this, &QBLEService::characteristicChanged, this, &InfiniTimeMotionService::characteristicChanged);

}

int InfiniTimeMotionService::steps() const
{
    return m_steps;
}

void InfiniTimeMotionService::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << "MiBand Changed:" << characteristic << value.toHex();

    if (characteristic == UUID_CHARACTERISTIC_MOTION_STEPS) {
        qDebug() << "...Got realtime steps:" << value.length();
        if (value.length() == 4) {
            m_steps = TypeConversion::toUint32(value[0], value[1], value[2], value[3]);
            emit informationChanged(AbstractDevice::INFO_STEPS, QString::number(m_steps));
        }
    }
}
