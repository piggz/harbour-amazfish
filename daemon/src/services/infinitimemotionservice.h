#ifndef INFINITIMEMOTIONSERVICE_H
#define INFINITIMEMOTIONSERVICE_H

#include <QObject>
#include "qble/qbleservice.h"
#include "devices/abstractdevice.h"

/** 00030000-78fc-48fe-8e23-433b3a1942d0 --Motion Service
 * "00030001-78fc-48fe-8e23-433b3a1942d0", --Step Count
 * "00030002-78fc-48fe-8e23-433b3a1942d0", --Motion
 */
class InfiniTimeMotionService : public QBLEService
{
    Q_OBJECT
public:
    InfiniTimeMotionService(const QString &path, QObject *parent);

    static const char *UUID_SERVICE_MOTION;
    static const char *UUID_CHARACTERISTIC_MOTION_STEPS;
    static const char *UUID_CHARACTERISTIC_MOTION_MOTION;

    Q_INVOKABLE void refreshMotion();
    Q_INVOKABLE void refreshSteps();

    Q_SIGNAL void informationChanged(AbstractDevice::Info key, const QString &val);
    Q_SIGNAL void motionChanged(double x, double y, double z);

    int steps() const;

private:
    void characteristicChanged(const QString &c, const QByteArray &value);

    int m_steps = 0;

};

#endif // INFINITIMENAVSERVICE_H
