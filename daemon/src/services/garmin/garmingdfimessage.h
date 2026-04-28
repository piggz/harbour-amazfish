#ifndef GARMINGDFIMESSAGE_H
#define GARMINGDFIMESSAGE_H

#include <QDataStream>
#include <QByteArray>
#include "garminbytebufferreader.h"

class MessageReader : public GarminByteBufferReader
{

public:

    MessageReader(QByteArray data);

    void skip(int offset);
    void warnIfLeftover(int messageType, bool supportedType);

private:

    int payloadSize;

    void checkSize();

    void checkCRC();
};

class GarminGDFIMessage
{
public:
    GarminGDFIMessage();

private:
    QDataStream mDataBuffer;
};


#endif // GARMINGDFIMESSAGE_H
