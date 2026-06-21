#include "garmindeviceinformationmessage.h"
#include "communicator_v2.h"




void GarminDeviceInformationMessage::parse(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO << "Garmin: parsing device information" << data;
    if (data.size() < 10) {
        qDebug() << Q_FUNC_INFO <<  "Garmin: DeviceInformation too short";
        return;
        //return Result<GfdiMessage>::err(
        //    GarminError(GarminError::Code::InvalidMessage, "DeviceInformation too short"));
    }
    int off = 0;

    DeviceInformationMessage msg;
    msg.protocolVersion = u16le(data, off); off += 2;
    msg.productNumber   = u16le(data, off); off += 2;
    msg.unitNumber      = u32le(data, off); off += 4;
    msg.softwareVersion = u16le(data, off); off += 2;
    msg.maxPacketSize   = u16le(data, off); off += 2;

    int consumed = 0;

    // Read length-prefixed strings (1 byte length, then N bytes of UTF-8 string)
    auto s1 = readLengthPrefixedString(data.mid(off), consumed);
    msg.bluetoothFriendlyName = s1.value;
    qDebug() << Q_FUNC_INFO << "Garmin: Friendly name is " <<s1.value;

    off += consumed;

    auto s2 = readLengthPrefixedString(data.mid(off), consumed);
    msg.deviceName = s2.value;
    qDebug() << Q_FUNC_INFO << "Garmin: name is " <<s2.value;
    off += consumed;

    auto s3 = readLengthPrefixedString(data.mid(off), consumed);
    msg.deviceModel = s3.value;

    // Generate Response
    QByteArray response = generateOutgoing(msg);
    response = wrapInGfdiEnvelope(5000,response);
    // Now update the device via the communicator
    if (mCommunicator) {
        mCommunicator->sendMessage("DEVICEINFORMATION RESPONSE",response);
        mCommunicator->onDeviceInformationReceived(msg);
    }
}

QByteArray GarminDeviceInformationMessage::generateOutgoing(const DeviceInformationMessage &incoming)
{
    QByteArray r;
    writeU16le(r, 5024);                             // original DEVICE_INFORMATION
    r.append(char(quint8(Status::Ack)));            // status

    writeU16le(r, 150);                              // protocol version 1.50
    writeU16le(r, 0xFFFF);                           // product number (-1 = 0xFFFF for phone)
    writeU32le(r, 0xFFFFFFFFu);                      // our unit number
    writeU16le(r, 7791);                             // software version  (7791 = version 77.91, matching Gadgetbridge)
    writeU16le(r, 0xFFFF);                           // our max packet size( -1 = 0xFFFF means no limit)
    // Bluetooth name (null-terminated)
    r.append("Jolla-Amazfish"); r.append(char(0));
    // Device manufacturer (null-terminated)
    r.append("Jolla");      r.append(char(0));
    // Device model (null-terminated)
    r.append("SailfishOS");             r.append(char(0));

     // Protocol flags (1 for v1.x, 0 for v2.x)
    const quint8 protocolFlags = (incoming.protocolVersion / 100 == 1) ? 1 : 0;
    r.append(char(protocolFlags));

    return r;
}
