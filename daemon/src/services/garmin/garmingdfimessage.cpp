#include "garmingdfimessage.h"
#include "checksumcalculator.h"

#include <qdebug.h>

MessageReader::MessageReader(QByteArray data) : GarminByteBufferReader(data)
{
    setByteOrder(QSysInfo::LittleEndian);
    payloadSize = readShort(); //includes CRC
    checkSize();
    checkCRC();
    //mByteBuffer.limit(payloadSize - 2); //remove CRC
}

void MessageReader::skip(int offset) {
    //if (remaining() < offset) throw new IllegalStateException();
    if (remaining() < offset)
    {
        qDebug() << "Unhandled Exeption skippping Buffer";
        return;
    }
    setPosition(getPosition()+offset);
}

void MessageReader::checkSize() {
    if (payloadSize != getLimit()) {
        qDebug() << "Received GFDI packet with invalid length: " << payloadSize << " vs " << getLimit();
    }
}

void MessageReader::checkCRC() {
    int pos = getPosition();
    setPosition(payloadSize - 2);
    int crc = readShort();
    setPosition(pos);
    QByteArray data = asReadOnlyBuffer();
    ChecksumCalculator calc;
    int correctCrc = calc.computeCrc(data,0,payloadSize-2);
    if (crc != correctCrc) {
        qDebug() << "Received GFDI packet with invalid CRC:" << crc << " vs " << correctCrc;
        //throw new IllegalArgumentException("Received GFDI packet with invalid CRC");
    }
}


GarminGDFIMessage::GarminGDFIMessage()
{

}
