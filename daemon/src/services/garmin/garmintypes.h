#ifndef GARMINTYPES__H
#define GARMINTYPES__H

#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <QtCore/QtGlobal>

#include <optional>
#include <utility>



// =============================================================================
// GarminError (Rust: enum GarminError)
// =============================================================================

class GarminError {
public:
    enum class Code {
        GfdiHandleNotSet,
        InvalidHandle,
        PacketTooShort,
        NotMlrPacket,
        OutOfSequence,
        FragmentNotFound,
        EmptyMessage,
        BluetoothError,
        EncodingError,
        DecodingError,
        Io,
        UnknownRequestType,
        UnknownServiceCode,
        InvalidMessage,
    };

    GarminError() = default;

    Code code() const { return m_code; }
    QString message() const { return m_message; }

    // --- Factory helpers mirroring Rust error variants and messages ---
    static GarminError gfdiHandleNotSet();                                             // "GFDI service handle not set"
    static GarminError invalidHandle(quint8 expected, quint8 got);                      // "Invalid handle: expected {expected}, got {got}"
    static GarminError packetTooShort(int bytes);                                       // "Packet too short: {0} bytes"
    static GarminError notMlrPacket();                                                  // "Not an MLR packet"
    static GarminError outOfSequence(quint8 expected, quint8 got);                      // "Out of sequence packet: expected {expected}, got {got}"
    static GarminError fragmentNotFound(quint8 index);                                  // "Fragment not found at index {0}"
    static GarminError emptyMessage();                                                  // "Message is empty"
    static GarminError bluetoothError(const QString& msg);                              // "Bluetooth error: {0}"
    static GarminError encodingError(const QString& msg);                               // "Encoding error: {0}"
    static GarminError decodingError(const QString& msg);                               // "Decoding error: {0}"
    static GarminError ioError(const QString& msg);                                     // "IO error: {0}"
    static GarminError unknownRequestType(quint8 v);                                    // "Unknown request type: {0}"
    static GarminError unknownServiceCode(quint16 v);                                   // "Unknown service code: {0}"
    static GarminError invalidMessage(const QString& msg);                              // "Invalid message: {0}"

    // Equivalent to Rust Display for the error
    QString toString() const { return m_message; }

    // Useful accessors for parameterised variants
    std::optional<quint8> expectedU8() const { return m_expectedU8; }
    std::optional<quint8> gotU8() const { return m_gotU8; }
    std::optional<quint16> gotU16() const { return m_gotU16; }
    std::optional<int> sizeBytes() const { return m_sizeBytes; }
    GarminError(Code c, QString msg) : m_code(c), m_message(std::move(msg)) {}

private:

    Code m_code {Code::InvalidMessage};
    QString m_message;

    std::optional<quint8>  m_expectedU8;
    std::optional<quint8>  m_gotU8;
    std::optional<quint16> m_gotU16;
    std::optional<int>     m_sizeBytes;
};


// =============================================================================
// Result<T> (Rust: pub type Result<T> = std::result::Result<T, GarminError>)
// =============================================================================

template<typename T>
struct Result {
    bool ok {false};
    T value {};
    GarminError error {};

    static Result<T> isOk(T v) {
        Result<T> r;
        r.ok = true;
        r.value = std::move(v);
        return r;
    }

    static Result<T> err(GarminError e) {
        Result<T> r;
        r.ok = false;
        r.error = std::move(e);
        return r;
    }
};

// Specialisation for void
template<>
struct Result<void> {
    bool ok {false};
    GarminError error {};

    static Result<void> isOk() {
        Result<void> r;
        r.ok = true;
        return r;
    }

    static Result<void> err(GarminError e) {
        Result<void> r;
        r.ok = false;
        r.error = std::move(e);
        return r;
    }
};


// =============================================================================
// RequestType (Rust: enum RequestType repr(u8) + from_u8/to_u8 + Display)
// =============================================================================

enum class RequestType : quint8 {
    RegisterMlReq  = 0,
    RegisterMlResp = 1,
    CloseHandleReq = 2,
    CloseHandleResp= 3,
    UnkHandle      = 4,
    CloseAllReq    = 5,
    CloseAllResp   = 6,
    UnkReq         = 7,
    UnkResp        = 8,
};

Result<RequestType> requestTypeFromU8(quint8 v);
inline quint8 requestTypeToU8(RequestType t) { return quint8(t); }
QString requestTypeToString(RequestType t);

// =============================================================================
// Service (Rust: enum Service repr(u16) + from_code/code + Display)
// =============================================================================

enum class Service : quint16 {
    GFDI = 1,
    Registration = 4,
    RealtimeHr = 6,
    RealtimeSteps = 7,
    RealtimeCalories = 8,
    RealtimeIntensity = 10,
    RealtimeHrv = 12,
    RealtimeStress = 13,
    RealtimeAccelerometer = 16,
    RealtimeSpo2 = 19,
    RealtimeBodyBattery = 20,
    RealtimeRespiration = 21,
    FileTransfer2 = 0x2018,
    FileTransfer4 = 0x4018,
    FileTransfer6 = 0x6018,
    FileTransferA = 0xA018,
    FileTransferC = 0xC018,
    FileTransferE = 0xE018,
};

Result<Service> serviceFromCode(quint16 code);
inline quint16 serviceCode(Service s) { return quint16(s); }
QString serviceToString(Service s);

#endif //GARMINTYPES__H
