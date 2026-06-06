#include "garminspo2message.h"

#include <QDateTime>
#include <QDebug>


GarminSpo2Message::GarminSpo2Message(CommunicatorV2* parent) : mCommunicator(parent)
{

}
void GarminSpo2Message::onConnect() {};
void GarminSpo2Message::onClose() {}
void GarminSpo2Message::onMessage(const QByteArray& data) {
    quint8 spo2 = data[0];
    quint32 ts = le32(data.constData()+1);

    ts=ts + 631065600; // Unix timestamp in seconds
    QDateTime Timestamp;

    qDebug() << Q_FUNC_INFO << "Garmin: realtime Spo2 Callback: " << spo2 << ", Timestamp " << QDateTime::fromTime_t(ts).toString();
    mCommunicator->setSpo2(spo2);

}
