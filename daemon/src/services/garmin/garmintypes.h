#ifndef GARMINTYPES__H
#define GARMINTYPES__H

#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <QtCore/QtGlobal>

#include <optional>
#include <utility>
#include <variant>


// This defines some selper classes and functions used by the Garmin communication

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
// Result<T>
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
// RequestType
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
// Service
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

/// Status codes for response messages
///
enum class Status : quint8 {
    Ack = 0,
    Nack = 1,
    Unsupported = 2,
    DecodeError = 3,
    CrcError = 4,
    LengthError = 5,
};

inline std::optional<Status> statusFromU8(quint8 code) {
    switch (code) {
    case 0: return Status::Ack;
    case 1: return Status::Nack;
    case 2: return Status::Unsupported;
    case 3: return Status::DecodeError;
    case 4: return Status::CrcError;
    case 5: return Status::LengthError;
    default: return std::nullopt;
    }
}

inline QString statusName(Status s) {
    switch (s) {
    case Status::Ack: return "ACK";
    case Status::Nack: return "NACK";
    case Status::Unsupported: return "UNSUPPORTED";
    case Status::DecodeError: return "DECODE_ERROR";
    case Status::CrcError: return "CRC_ERROR";
    case Status::LengthError: return "LENGTH_ERROR";
    }
    return "UNKNOWN";
}



// Filter Status Message (response to Filter message)

struct FilterStatusMessage {
    Status status{};
    quint8 filterType{};
};

// Weather Request Message (5014)

struct WeatherRequestMessage {
    quint8 format{};
    qint32 latitude{};
    qint32 longitude{};
    quint8 hoursOfForecast{};
};



struct UnknownMessage {
    quint16 messageId {0};
    QByteArray data;
};




// -------------------- Parsed message structs --------------------



// Notification Control Message (incoming from watch)

struct NotificationControlMessage {
    qint32 notificationId{};
    quint8 command{};
    QVector<QPair<quint8, quint16>> attributes; // (attribute_id, max_length)
    std::optional<quint8> actionId;
    std::optional<QString> actionString;
};

// Notification Subscription Message (incoming from watch)

struct NotificationSubscriptionMessage {
    bool enable{};
    quint8 unk{};
};

// Synchronization Message (incoming from watch)

struct SynchronizationMessage {
    quint8 synchronizationType{};
    quint64 fileTypeBitmask{};

    bool shouldProceed() const {
        constexpr quint64 WORKOUTS = 1ull << 3;
        constexpr quint64 ACTIVITIES = 1ull << 5;
        constexpr quint64 ACTIVITY_SUMMARY = 1ull << 21;
        constexpr quint64 SLEEP = 1ull << 26;
        return (fileTypeBitmask & (WORKOUTS | ACTIVITIES | ACTIVITY_SUMMARY | SLEEP)) != 0;
    }
};

using GfdiMessage = std::variant<
    NotificationControlMessage,
    NotificationSubscriptionMessage,
    SynchronizationMessage,
    FilterStatusMessage,
    WeatherRequestMessage,
    UnknownMessage
>;

// Helper functions
quint16 computeCrc16(const QByteArray& data);
QByteArray wrapInGfdiEnvelope(quint16 messageId, const QByteArray& payload);
quint16 le16(const char* p);
quint32 le32(const char* p);
quint64 le64(const char* p);
quint8  u8le(const QByteArray& b, int off);
quint16 u16le(const QByteArray& b, int off);
quint32 u32le(const QByteArray& b, int off);
quint64 u64le(const QByteArray& b, int off);
qint32  i32le(const QByteArray& b, int off);
void writeU16le(QByteArray& out, quint16 v);
void writeU32le(QByteArray& out, quint32 v);
void writeU64le(QByteArray& out, quint64 v);


// Callback for service lifecycle events
class ServiceCallback : public QObject{
    Q_OBJECT
public:
    virtual ~ServiceCallback() = default;

    // Called when service is connected and ready to use
    /*
    void onConnect(QSharedPointer<ServiceWriter> writer) {
        Q_UNUSED(writer);
    }
    */
    virtual void onClose() { }
    // Called when a message is received from the service
    virtual void onMessage(const QByteArray& data) = 0;

private:
    //QBLEService *mParent;
};


#endif //GARMINTYPES__H
