#include "garmindevicestatusmessage.h"

void GarminDeviceStatusMessage::parse(const QByteArray& data) {
    // This is a device status response
    const quint8 firstTag = static_cast<quint8>(data[0]);
    const quint8 fieldNumber = firstTag >> 3;
    const quint8 wireType = firstTag & 0x07;
    qDebug() << Q_FUNC_INFO << "Garmin: protobuf field:" << fieldNumber
            << "(wire type:" << wireType << ") Payload: " << data.toHex();
    qDebug() << Q_FUNC_INFO << "Garmin: Status Response payload:" << data.toHex();
    quint8 protobufSize = data[1];
    if (fieldNumber==3 && wireType ==2) {
        //battery reponse
        char status = data[3];
        if (status ==1) {
            //OK
            // According to protobuf documentation, batterylevel should be int32 but seems to be uint8 in byte 5
            quint8 batteryLevel=u16le(data,5);
            qDebug() << Q_FUNC_INFO << "Garmin: Battery Leves is " << batteryLevel << "%";
            if (mCommunicator) mCommunicator->setBatteryLevel(batteryLevel);
        }
    }
}

QByteArray GarminDeviceStatusMessage::generateResponse(const QByteArray& data) {

}

