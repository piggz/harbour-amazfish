#include "garminstepsmessage.h"

#include <QDateTime>
#include <QDebug>


GarminStepsMessage::GarminStepsMessage(CommunicatorV2* parent) : mCommunicator(parent)
{

}
void GarminStepsMessage::onConnect() {}
void GarminStepsMessage::onClose() {}
void GarminStepsMessage::onMessage(const QByteArray& data) {
    if (data.size() >= 8)
    {
        quint32 steps = le32(data.constData());
        quint32 stepsGoal = le32(data.constData()+4);
        qDebug() << Q_FUNC_INFO << "Garmin: Realtime  Steps :  " << steps << ", goal = " << stepsGoal;

        mCommunicator->setStepsGoal(stepsGoal);
        mCommunicator->setSteps(steps);
    }
}
