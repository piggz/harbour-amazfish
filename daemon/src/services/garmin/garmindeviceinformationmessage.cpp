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

    // Now update the device via the communicator
    if (mCommunicator) mCommunicator->onDeviceInformationReceived(msg);
}
