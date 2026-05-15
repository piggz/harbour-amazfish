#ifndef GARMINGDFIMESSAGE_H
#define GARMINGDFIMESSAGE_H

#include <QDataStream>
#include <QByteArray>
#include "garminbytebufferreader.h"
#include <typeindex>
#include <QMap>


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
    enum MessageType {
        RESPONSE = 5000,
        DOWNLOAD_REQUEST = 5002,
        UPLOAD_REQUEST = 5003,
        FILE_TRANSFER_DATA = 5004,
        CREATE_FILE = 5005,
        FILTER = 5007,
        SET_FILE_FLAG = 5008,
        FIT_DEFINITION = 5011,
        FIT_DATA = 5012,
        WEATHER_REQUEST = 5014,
        DEVICE_INFORMATION = 5024,
        DEVICE_SETTINGS = 5026,
        SYSTEM_EVENT = 5030,
        SUPPORTED_FILE_TYPES_REQUEST = 5031,
        NOTIFICATION_UPDATE = 5033,
        NOTIFICATION_CONTROL = 5034,
        NOTIFICATION_DATA = 5035,
        NOTIFICATION_SUBSCRIPTION = 5036,
        SYNCHRONIZATION = 5037,
        FIND_MY_PHONE_REQUEST = 5039,
        FIND_MY_PHONE_CANCEL = 5040,
        MUSIC_CONTROL = 5041,
        MUSIC_CONTROL_CAPABILITIES = 5042,
        PROTOBUF_REQUEST = 5043,
        PROTOBUF_RESPONSE = 5044,
        MUSIC_CONTROL_ENTITY_UPDATE = 5049,
        CONFIGURATION = 5050,
        CURRENT_TIME_REQUEST = 5052,
        AUTH_NEGOTIATION = 5101
    };


    enum Status {
        ACK = 0,
        NAK = 1,
        UNSUPPORTED= 2,
        DECODE_ERROR = 3,
        CRC_ERROR =4,
        LENGTH_ERROR =5
    };

    GarminGDFIMessage();
    //GarminGDFIMessage parseIncoming(QByteArray message);
    void parseIncoming(GarminGDFIMessage &message, QByteArray incoming);


private:

    using MessageCreator = std::function<GarminGDFIMessage*()>;

    QMap<int, MessageCreator> mMesssageMap= {
        { MessageType::RESPONSE, []() { return new GarminGDFIMessage; } }
    };

protected:
    QBuffer mResponse;
    //GarminMessage garminMessage;
    //QMap<int,GarminGDFIMessage> GarminMessageMap;
};


#endif // GARMINGDFIMESSAGE_H
