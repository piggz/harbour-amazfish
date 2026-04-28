#ifndef GARMINBYTEBUFFERREADER_H
#define GARMINBYTEBUFFERREADER_H

#include <QBuffer>
#include <qsysinfo.h>

class GarminByteBufferReader
{
public:
    GarminByteBufferReader(QByteArray byteBuffer);
    ~GarminByteBufferReader();

     int remaining();
     QByteArray asReadOnlyBuffer();
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

private:
     QBuffer mByteBuffer;
     QSysInfo::Endian mOrder=QSysInfo::LittleEndian;
};

#endif // GARMINBYTEBUFFERREADER_H
