#include "garminbytebufferreader.h"


GarminByteBufferReader::GarminByteBufferReader(QByteArray data) {
        mByteBuffer.setData(data);
        mByteBuffer.open(QIODevice::ReadOnly);
}

GarminByteBufferReader::~GarminByteBufferReader()
{
    mByteBuffer.close();
}
int GarminByteBufferReader::remaining()
{
        return mByteBuffer.bytesAvailable();
}


QByteArray GarminByteBufferReader::asReadOnlyBuffer()
{
        return mByteBuffer.data();
}


void GarminByteBufferReader::setByteOrder(QSysInfo::Endian   byteOrder)
{
        mOrder=byteOrder;
}

int GarminByteBufferReader::readByte()
{
    char res;
    mByteBuffer.getChar(&res);
    return static_cast<unsigned int>(res);
}

int GarminByteBufferReader::getPosition()
{
        return mByteBuffer.pos();
}

void GarminByteBufferReader::setPosition(int position)
{
        mByteBuffer.seek(position);
}

int GarminByteBufferReader::getLimit()
{
        return mByteBuffer.size();
}

int GarminByteBufferReader::readShort()
{
    QByteArray data;
    data=mByteBuffer.read(2);
    return data.toInt();
}

int GarminByteBufferReader::readInt()
{
    QByteArray data;
    data = mByteBuffer.read(4);
    return data.toInt();
}

long GarminByteBufferReader::readLong()
{
    QByteArray data;
    data = mByteBuffer.read(8);
    return data.toLong();
}

float GarminByteBufferReader::readFloat32()
{
    QByteArray data;
    data = mByteBuffer.read(4);
    return data.toFloat();
}

double GarminByteBufferReader::readFloat64() {
    QByteArray data;
    data = mByteBuffer.read(8);
    return data.toDouble();
}

QString GarminByteBufferReader::readString() {
        int size = readByte();
        QByteArray bytes;
        bytes = mByteBuffer.read(size);
        QString res(bytes);
        return res;
    }

QString GarminByteBufferReader::readNullTerminatedString() {
        int position = mByteBuffer.pos();
        int size = 0;
        while (!mByteBuffer.atEnd()) {
            char c;
            mByteBuffer.read(&c,1);
            if ( c == 0)
                break;
            size++;
        }
        mByteBuffer.seek(position);
        QByteArray bytes;
        bytes = mByteBuffer.read(size);
        mByteBuffer.read(1); // discard null terminator
        QString res(bytes);
        return res;
    }

QByteArray GarminByteBufferReader::readBytes(int size) {
        QByteArray bytes;

        bytes = mByteBuffer.read(size);

        return bytes;
    }

