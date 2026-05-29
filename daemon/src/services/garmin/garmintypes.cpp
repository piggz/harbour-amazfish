#include "garmintypes.h"

/*
 * GarminError
 */



// =============================================================================
// GarminError implementations [1](https://computacenter-my.sharepoint.com/personal/thomas_michel_computacenter_com/Documents/Microsoft%20Copilot%20Chat%20Files/types.rs)
// =============================================================================

GarminError GarminError::gfdiHandleNotSet() {
    return GarminError(Code::GfdiHandleNotSet, QStringLiteral("GFDI service handle not set"));
}

GarminError GarminError::invalidHandle(quint8 expected, quint8 got) {
    GarminError e(Code::InvalidHandle,
                  QStringLiteral("Invalid handle: expected %1, got %2").arg(expected).arg(got));
    e.m_expectedU8 = expected;
    e.m_gotU8 = got;
    return e;
}

GarminError GarminError::packetTooShort(int bytes) {
    GarminError e(Code::PacketTooShort,
                  QStringLiteral("Packet too short: %1 bytes").arg(bytes));
    e.m_sizeBytes = bytes;
    return e;
}

GarminError GarminError::notMlrPacket() {
    return GarminError(Code::NotMlrPacket, QStringLiteral("Not an MLR packet"));
}

GarminError GarminError::outOfSequence(quint8 expected, quint8 got) {
    GarminError e(Code::OutOfSequence,
                  QStringLiteral("Out of sequence packet: expected %1, got %2").arg(expected).arg(got));
    e.m_expectedU8 = expected;
    e.m_gotU8 = got;
    return e;
}

GarminError GarminError::fragmentNotFound(quint8 index) {
    GarminError e(Code::FragmentNotFound,
                  QStringLiteral("Fragment not found at index %1").arg(index));
    e.m_gotU8 = index;
    return e;
}

GarminError GarminError::emptyMessage() {
    return GarminError(Code::EmptyMessage, QStringLiteral("Message is empty"));
}

GarminError GarminError::bluetoothError(const QString& msg) {
    return GarminError(Code::BluetoothError, QStringLiteral("Bluetooth error: %1").arg(msg));
}

GarminError GarminError::encodingError(const QString& msg) {
    return GarminError(Code::EncodingError, QStringLiteral("Encoding error: %1").arg(msg));
}

GarminError GarminError::decodingError(const QString& msg) {
    return GarminError(Code::DecodingError, QStringLiteral("Decoding error: %1").arg(msg));
}

GarminError GarminError::ioError(const QString& msg) {
    return GarminError(Code::Io, QStringLiteral("IO error: %1").arg(msg));
}

GarminError GarminError::unknownRequestType(quint8 v) {
    GarminError e(Code::UnknownRequestType,
                  QStringLiteral("Unknown request type: %1").arg(v));
    e.m_gotU8 = v;
    return e;
}

GarminError GarminError::unknownServiceCode(quint16 v) {
    GarminError e(Code::UnknownServiceCode,
                  QStringLiteral("Unknown service code: %1").arg(v));
    e.m_gotU16 = v;
    return e;
}

GarminError GarminError::invalidMessage(const QString& msg) {
    return GarminError(Code::InvalidMessage, QStringLiteral("Invalid message: %1").arg(msg));
}

// =============================================================================
// RequestType conversions + Display [1](https://computacenter-my.sharepoint.com/personal/thomas_michel_computacenter_com/Documents/Microsoft%20Copilot%20Chat%20Files/types.rs)
// =============================================================================

Result<RequestType> requestTypeFromU8(quint8 v) {
    switch (v) {
    case 0: return Result<RequestType>::isOk(RequestType::RegisterMlReq);
    case 1: return Result<RequestType>::isOk(RequestType::RegisterMlResp);
    case 2: return Result<RequestType>::isOk(RequestType::CloseHandleReq);
    case 3: return Result<RequestType>::isOk(RequestType::CloseHandleResp);
    case 4: return Result<RequestType>::isOk(RequestType::UnkHandle);
    case 5: return Result<RequestType>::isOk(RequestType::CloseAllReq);
    case 6: return Result<RequestType>::isOk(RequestType::CloseAllResp);
    case 7: return Result<RequestType>::isOk(RequestType::UnkReq);
    case 8: return Result<RequestType>::isOk(RequestType::UnkResp);
    default:
        return Result<RequestType>::err(GarminError::unknownRequestType(v));
    }
}

QString requestTypeToString(RequestType t) {
    switch (t) {
    case RequestType::RegisterMlReq:   return QStringLiteral("REGISTER_ML_REQ");
    case RequestType::RegisterMlResp:  return QStringLiteral("REGISTER_ML_RESP");
    case RequestType::CloseHandleReq:  return QStringLiteral("CLOSE_HANDLE_REQ");
    case RequestType::CloseHandleResp: return QStringLiteral("CLOSE_HANDLE_RESP");
    case RequestType::UnkHandle:       return QStringLiteral("UNK_HANDLE");
    case RequestType::CloseAllReq:     return QStringLiteral("CLOSE_ALL_REQ");
    case RequestType::CloseAllResp:    return QStringLiteral("CLOSE_ALL_RESP");
    case RequestType::UnkReq:          return QStringLiteral("UNK_REQ");
    case RequestType::UnkResp:         return QStringLiteral("UNK_RESP");
    }
    return QStringLiteral("UNKNOWN");
}

// =============================================================================
// Service conversions + Display [1](https://computacenter-my.sharepoint.com/personal/thomas_michel_computacenter_com/Documents/Microsoft%20Copilot%20Chat%20Files/types.rs)
// =============================================================================

Result<Service> serviceFromCode(quint16 code) {
    switch (code) {
    case 1:      return Result<Service>::isOk(Service::GFDI);
    case 4:      return Result<Service>::isOk(Service::Registration);
    case 6:      return Result<Service>::isOk(Service::RealtimeHr);
    case 7:      return Result<Service>::isOk(Service::RealtimeSteps);
    case 8:      return Result<Service>::isOk(Service::RealtimeCalories);
    case 10:     return Result<Service>::isOk(Service::RealtimeIntensity);
    case 12:     return Result<Service>::isOk(Service::RealtimeHrv);
    case 13:     return Result<Service>::isOk(Service::RealtimeStress);
    case 16:     return Result<Service>::isOk(Service::RealtimeAccelerometer);
    case 19:     return Result<Service>::isOk(Service::RealtimeSpo2);
    case 20:     return Result<Service>::isOk(Service::RealtimeBodyBattery);
    case 21:     return Result<Service>::isOk(Service::RealtimeRespiration);
    case 0x2018: return Result<Service>::isOk(Service::FileTransfer2);
    case 0x4018: return Result<Service>::isOk(Service::FileTransfer4);
    case 0x6018: return Result<Service>::isOk(Service::FileTransfer6);
    case 0xA018: return Result<Service>::isOk(Service::FileTransferA);
    case 0xC018: return Result<Service>::isOk(Service::FileTransferC);
    case 0xE018: return Result<Service>::isOk(Service::FileTransferE);
    default:
        return Result<Service>::err(GarminError::unknownServiceCode(code));
    }
}

QString serviceToString(Service s) {
    switch (s) {
    case Service::GFDI:                 return QStringLiteral("GFDI");
    case Service::Registration:         return QStringLiteral("Registration");
    case Service::RealtimeHr:           return QStringLiteral("RealtimeHR");
    case Service::RealtimeSteps:        return QStringLiteral("RealtimeSteps");
    case Service::RealtimeCalories:     return QStringLiteral("RealtimeCalories");
    case Service::RealtimeIntensity:    return QStringLiteral("RealtimeIntensity");
    case Service::RealtimeHrv:          return QStringLiteral("RealtimeHRV");
    case Service::RealtimeStress:       return QStringLiteral("RealtimeStress");
    case Service::RealtimeAccelerometer:return QStringLiteral("RealtimeAccelerometer");
    case Service::RealtimeSpo2:         return QStringLiteral("RealtimeSPO2");
    case Service::RealtimeBodyBattery:  return QStringLiteral("RealtimeBodyBattery");
    case Service::RealtimeRespiration:  return QStringLiteral("RealtimeRespiration");
    case Service::FileTransfer2:        return QStringLiteral("FileTransfer2");
    case Service::FileTransfer4:        return QStringLiteral("FileTransfer4");
    case Service::FileTransfer6:        return QStringLiteral("FileTransfer6");
    case Service::FileTransferA:        return QStringLiteral("FileTransferA");
    case Service::FileTransferC:        return QStringLiteral("FileTransferC");
    case Service::FileTransferE:        return QStringLiteral("FileTransferE");
    }
    return QStringLiteral("Unknown");
}

quint16 computeCrc16(const QByteArray& data)
{
    static const quint16 C[16] = {
        0x0000,0xCC01,0xD801,0x1400,0xF001,0x3C00,0x2800,0xE401,
        0xA001,0x6C00,0x7800,0xB401,0x5000,0x9C01,0x8801,0x4400
    };

    quint16 crc = 0;

    for (quint8 b : data) {
        crc = (((crc >> 4) & 0x0FFF) ^ C[crc & 0x0F]) ^ C[b & 0x0F];
        crc = (((crc >> 4) & 0x0FFF) ^ C[crc & 0x0F]) ^ C[(b >> 4) & 0x0F];
    }

    return crc;
}

QByteArray wrapInGfdiEnvelope(quint16 messageId, const QByteArray& payload)
{
    QByteArray msg;

    quint16 size = static_cast<quint16>(2 + 2 + payload.size() + 2);

    msg.append(char(size & 0xFF));
    msg.append(char((size >> 8) & 0xFF));

    msg.append(char(messageId & 0xFF));
    msg.append(char((messageId >> 8) & 0xFF));

    msg.append(payload);

    quint16 crc = computeCrc16(msg);

    msg.append(char(crc & 0xFF));
    msg.append(char((crc >> 8) & 0xFF));

    return msg;
}

quint16 le16(const char* p) {
    return quint16(quint8(p[0]) | (quint16(quint8(p[1]))<<8));
}

quint32 le32(const char* p) {
    return quint32(quint8(p[0]) | (quint32(quint8(p[1]))<<8) | (quint32(quint8(p[2]))<<16) | (quint32(quint8(p[3]))<<24));
}

quint64 le64(const char* p) {
    quint64 v=0;
    for (int i=0;i<8;++i) v |= (quint64(quint8(p[i])) << (8*i));
    return v;
}

quint8 u8le(const QByteArray& b, int off) {
    if (off >= b.size()) return 0;
    return quint16(quint8(b[off]));
}

quint16 u16le(const QByteArray& b, int off) {
    if (off + 1 >= b.size()) return 0;
    return quint16(quint8(b[off])) | (quint16(quint8(b[off+1])) << 8);
}

quint32 u32le(const QByteArray& b, int off) {
    if (off + 3 >= b.size()) return 0;
    return quint32(quint8(b[off])) |
           (quint32(quint8(b[off+1])) << 8) |
           (quint32(quint8(b[off+2])) << 16) |
           (quint32(quint8(b[off+3])) << 24);
}

quint64 u64le(const QByteArray& b, int off) {
    if (off + 7 >= b.size()) return 0;
    quint64 v = 0;
    for (int i=0;i<8;i++) v |= (quint64(quint8(b[off+i])) << (8*i));
    return v;
}

qint32 i32le(const QByteArray& b, int off) {
    return qint32(u32le(b, off));
}

void writeU16le(QByteArray& out, quint16 v) {
    out.append(char(v & 0xFF));
    out.append(char((v >> 8) & 0xFF));
}
void writeU32le(QByteArray& out, quint32 v) {
    out.append(char(v & 0xFF));
    out.append(char((v >> 8) & 0xFF));
    out.append(char((v >> 16) & 0xFF));
    out.append(char((v >> 24) & 0xFF));
}
void writeU64le(QByteArray& out, quint64 v) {
    for (int i=0;i<8;++i) out.append(char((v >> (8*i)) & 0xFF));
}
