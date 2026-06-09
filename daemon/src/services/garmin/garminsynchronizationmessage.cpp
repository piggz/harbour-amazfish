#include "garminsynchronizationmessage.h"

void GarminSynchronizationMessage::parse(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO << "Garmin: parsing synchronization";
    if (data.size() < 2) {
        return; // Result<GfdiMessage>::err(GarminError(GarminError::Code::InvalidMessage, "Synchronization message too short"));
    }
    const quint8 syncType = quint8(data[0]);
    const int size = quint8(data[1]);

    if (data.size() < 2 + size) {
        return;// Result<GfdiMessage>::err(GarminError(GarminError::Code::InvalidMessage, "Synchronization message truncated"));
    }

    quint64 bitmask = 0;
    if (size == 8) {
        bitmask = u64le(data, 2);
    } else if (size == 4) {
        bitmask = quint64(u32le(data, 2));
    } else {
        return;
    }

    SynchronizationMessage msg;
    msg.synchronizationType = syncType;
    msg.fileTypeBitmask = bitmask;
    //TODO: handle the message in Commuicator!
    if ((bitmask == FileType::WORKOUTS) || (bitmask == FileType::ACTIVITY_SUMMARY) || (bitmask == FileType::ACTIVITIES) || (bitmask == FileType::SLEEP)) {
       if (mCommunicator) mCommunicator->onSynchronizationReceived(msg);
    }
}
