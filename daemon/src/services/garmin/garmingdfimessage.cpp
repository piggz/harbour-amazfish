#include "garmingdfimessage.h"
#include "checksumcalculator.h"

#include <qdebug.h>
#include <QMap>


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

void MessageReader::warnIfLeftover(int messageType, bool supportedType) {
            if (!mByteBuffer.atEnd() && (getPosition() < getLimit())) {
                int pos = getPosition();
                QByteArray leftover;
                leftover=mByteBuffer.readAll();
                setPosition(pos);
                if (supportedType){
                    qDebug() << "Leftover bytes when parsing message type " << messageType << ". Bytes: " << leftover << ", complete message: " << mByteBuffer.data();
                } else {
                    qDebug() << "Unknown message type " << messageType <<". Bytes: " <<leftover;;
                }
            }
        }

GarminGDFIMessage::GarminGDFIMessage()
{

}

void GarminGDFIMessage::parseIncoming(GarminGDFIMessage &message, QByteArray incoming)
{
    MessageReader messageReader(message);

    bool supportedType = false;
    int messageType = messageReader.readShort();
    std::unique_ptr<GarminGDFIMessage> tmp(new GarminGDFIMessage);
            /*
        try {
        if ((messageType & 0x8000) != 0) {
            // final int sequenceNumber = (messageType >> 8) & 0x7f;
            messageType = (messageType & 0xff) + 5000;
        }
        if (mMesssageMap.contains(messageType)) {
            mMesssageMap.value(messageType)();
        }
        if (mes != NULL)
        {
            supportedType=true;
            mes->parseIncoming(message);
            return *mes;
        } else
        {
            supportedType=false;
            return new GarminGDFIMessage;
         }
         */
}
