#include "garminhrvmessage.h"

#include <QDateTime>
#include <QDebug>


GarminHrvMessage::GarminHrvMessage(CommunicatorV2* parent) : mCommunicator(parent)
{

}
void GarminHrvMessage::onConnect() {}
void GarminHrvMessage::onClose() {}
void GarminHrvMessage::onMessage(const QByteArray& data) {
    quint16 rr = le16(data.constData());
    quint16 unk = le32(data.constData() + 2);
    qDebug() << Q_FUNC_INFO << "Garmin: realtime HRV: " << rr << ", Unknown " << unk;
    mCommunicator->setHRV(rr);
}
