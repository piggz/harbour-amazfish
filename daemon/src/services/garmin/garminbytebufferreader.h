#ifndef GARMINBYTEBUFFERREADER_H
#define GARMINBYTEBUFFERREADER_H

#include <qbuffer.h>
#include <qsysinfo.h>
#include "byteorder.h"

class GarminByteBufferReader
{
public:
    GarminByteBufferReader(QBuffer byteBuffer);

     int remaining();
     QBuffer asReadOnlyBuffer();
     void setByteOrder(QSysInfo::Endian   byteOrder);
     int readByte();
     int getPosition();
     void setPosition(const int position);
     int getLimit();
     int readShort();
     int readInt();
     long readLong();
     float readFloat32();
     double readFloat64();
     QString readString();
     QString readNullTerminatedString();
     QByteArray readBytes(int size);
};

#endif // GARMINBYTEBUFFERREADER_H
