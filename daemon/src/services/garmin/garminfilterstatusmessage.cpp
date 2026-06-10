#include "garminfilterstatusmessage.h"

void GarminFilterStatusMessage::parse(const QByteArray& data) {
    qDebug() << Q_FUNC_INFO << "Garmin: parsing filter status";

    if (data.size() < 3) {
        return; // Result<GfdiMessage>::err(GarminError(GarminError::Code::InvalidMessage, "Filter status message too short"));
    }

    const quint8 statusByte = quint8(data[2]);
    Status st;
    switch (statusByte) {
    case 0: st = Status::Ack; break;
    case 1: st = Status::Nack; break;
    case 2: st = Status::Unsupported; break;
    default:
        return;// Result<GfdiMessage>::err(GarminError(GarminError::Code::InvalidMessage,QString("Unknown status: %1").arg(statusByte)));
    }

    FilterStatusMessage msg;
    msg.status = st;
    msg.filterType = (data.size() > 3) ? quint8(data[3]) : 0;
    if (mCommunicator) mCommunicator->onFilterStatusReceived(msg);
}
