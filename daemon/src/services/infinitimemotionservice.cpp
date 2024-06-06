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

void InfiniTimeMotionService::refreshSteps()
{
    readAsync(UUID_CHARACTERISTIC_MOTION_STEPS);
}

void InfiniTimeMotionService::refreshMotion()
{
    readAsync(UUID_CHARACTERISTIC_MOTION_MOTION);
}

int InfiniTimeMotionService::steps() const
{
    return m_steps;
}

void InfiniTimeMotionService::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << characteristic << value.toHex();

    if (characteristic == UUID_CHARACTERISTIC_MOTION_STEPS) {
        if (value.length() == 4) {
            m_steps = TypeConversion::toUint32(value[0], value[1], value[2], value[3]);
            emit informationChanged(AbstractDevice::INFO_STEPS, QString::number(m_steps));
        }
    } else if (characteristic == UUID_CHARACTERISTIC_MOTION_MOTION) {
        if (value.length() == 6) {
            double x = (double) TypeConversion::toInt16(value[0], value[1]) * 0.01;
            double y = (double) TypeConversion::toInt16(value[2], value[3]) * 0.01;
            double z = (double) TypeConversion::toInt16(value[4], value[5]) * 0.01;
            emit motionChanged(x, y, z);
            qDebug() << "...Got realtime motion: x = " << x << " y = " << y << " z = " << z;
        }
    }
}
