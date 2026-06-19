#ifndef GARMINSYNCHRONIZATIONMESSAGE_H
#define GARMINSYNCHRONIZATIONMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"

#include <QObject>

enum FileType {
    unk_0,
    unk_1,
    unk_2,
    WORKOUTS,
    unk_4,
    ACTIVITIES,
    unk_6,
    unk_7,
    SOFTWARE_UPDATE,
    unk_9,
    unk_10,
    unk_11,
    unk_12,
    unk_13,
    unk_14,
    unk_15,
    unk_16,
    unk_17,
    unk_18,
    unk_19,
    unk_20,
    ACTIVITY_SUMMARY,
    unk_22,
    unk_23,
    unk_24,
    unk_25,
    SLEEP,
    unk_27,
    unk_28,
    unk_29,
    unk_30,
    unk_31,
    unk_32,
    unk_33,
    unk_34,
    unk_35,
    unk_36,

};


class GarminSynchronizationMessage : public GarminGfdiMessage
{
public:
    GarminSynchronizationMessage(CommunicatorV2 *parent) {
        mCommunicator = parent;
    }
    void parse(const QByteArray& data);
    QByteArray generateResponse(const QByteArray& data);
};

#endif // GARMINSYNCHRONIZATIONMESSAGE_H
