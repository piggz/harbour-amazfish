#include "garminhrmmessage.h"

#include <QDateTime>
#include <QDebug>


GarminHrmMessage::GarminHrmMessage(CommunicatorV2* parent) : mCommunicator(parent)
{

}
void GarminHrmMessage::onConnect() {};
void GarminHrmMessage::onClose() {}
void GarminHrmMessage::onMessage(const QByteArray& data) {
    quint8 spo2 = data[0];
    quint32 ts = le32(data.constData()+1);

    ts=ts + 631065600; // Unix timestamp in seconds
    QDateTime Timestamp;

    qDebug() << Q_FUNC_INFO << "Garmin: realtime Spo2 Callback: " << spo2 << ", Timestamp " << QDateTime::fromTime_t(ts).toString();
    mCommunicator->setSpo2(spo2);

}
