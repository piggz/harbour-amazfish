#include "garmindevicestatusmessage.h"
#include "garmintypes.h"

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

QByteArray GarminDeviceStatusMessage::generateOutgoing(const QByteArray& data) {

}


QByteArray GarminDeviceStatusMessage::generateBatteryStatusRequest(quint16 requestId)
{

    // Generate a ProtobufRequest for battery status updates
    //
    // This sends a protobuf request asking the watch to provide battery status updates.
    // Sent during initialization (completeInitialization in Java).
    //
    // The protobuf structure is:
    // ```proto
    // Smart {
    //   device_status_service = 8 {
    //     remote_device_battery_status_request = 2 {
    //       // empty message
    //     }
    //   }
    // }
    // ```
    //
    // # Arguments
    // * `requestId` - The protobuf request ID (incrementing counter)

    // Build the protobuf payload manually
    // Smart.device_status_service (field 8, type length-delimited)
    // DeviceStatusService.remote_device_battery_status_request (field 2, type length-delimited)

    // Inner message: RemoteDeviceBatteryStatusRequest (empty)
    QByteArray inner; // empty

    // DeviceStatusService with field 2 (remote_device_battery_status_request)
    QByteArray deviceStatusService;
    deviceStatusService.append(char((2 << 3) | 2)); // Field 2, wire type 2 (length-delimited)
    //deviceStatusService.append(char(inner.size()));// Length of inner message (0)
    //deviceStatusService.append(inner);
    deviceStatusService.append(char(0));

    // Smart with field 8 (device_status_service)
    QByteArray smartProto;
    smartProto.append(char((8 << 3) | 2));// Field 8, wire type 2 (length-delimited)
    smartProto.append(char(deviceStatusService.size()));// Length
    smartProto.append(deviceStatusService);

    // Now build the ProtobufRequest message
    QByteArray m;
    // Request ID
    writeU16le(m, requestId);
    // Data offset (0 for non-chunked)
    writeU32le(m, 0);
    // Total protobuf length
    writeU32le(m, quint32(smartProto.size()));
    // Protobuf data length (same as total for non-chunked)
    writeU32le(m, quint32(smartProto.size()));
    // Protobuf payload
    m.append(smartProto);
    // Message ID: PROTOBUF_REQUEST (5043)
    return wrapInGfdiEnvelope(5043,m);
}
