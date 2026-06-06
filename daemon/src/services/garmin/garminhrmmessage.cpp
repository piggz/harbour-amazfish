#include "garminhrmmessage.h"

#include <QDateTime>
#include <QDebug>


GarminHrmMessage::GarminHrmMessage(CommunicatorV2* parent) : mCommunicator(parent)
{

}
void GarminHrmMessage::onConnect() {}
void GarminHrmMessage::onClose() {}
void GarminHrmMessage::onMessage(const QByteArray& data) {
    quint8 type = data[0]; // 0/2/3? 3 == realtime?
    quint8 hr = data[1] & 0xff;
    quint8 resting = data[2] & 0xff;
    qDebug() << "Garmin: Got realtime HR type: " << type << ", hr=" << hr << ", resting=" << resting;
    mCommunicator->setHeartRate(hr);

}
