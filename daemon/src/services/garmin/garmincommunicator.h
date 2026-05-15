#pragma once

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QHash>
#include <QQueue>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>
#include <QSharedPointer>
#include <QWeakPointer>
#include <optional>
#include <variant>
#include "cobscodec.h"
#include "garminmlr.h"
#include "garminmessages.h"
#include "garmintypes.h"

// -------------------- Constants --------------------
static constexpr const char* BASE_UUID_FORMAT = "6A4E%04X-667B-11E3-949A-0800200C9A66";
static constexpr quint64 GADGETBRIDGE_CLIENT_ID = 2;


template <typename T>
using Result = std::variant<T, GarminError>;

/*
// Void result: empty => success, value => error
using ResultVoid = std::optional<GarminError>;
static inline ResultVoid ok() { return std::nullopt; }
static inline ResultVoid err(GarminError e) { return ResultVoid{std::m  ove(e)}; }
static inline bool isOk(const ResultVoid& r) { return !r.has_value(); }
template <typename T>
static inline bool isOk(const Result<T>& r) { return std::holds_alternative<T>(r); }

*/

// -------------------- BLE handles --------------------
struct CharacteristicHandle {
    QString uuid;
};
inline uint qHash(const CharacteristicHandle& h, uint seed = 0) { return qHash(h.uuid, seed); }

// -------------------- BLE Transaction interface --------------------
class ITransaction {
public:
    virtual ~ITransaction() = default;
    virtual void write(const CharacteristicHandle& handle, const QByteArray& data) = 0;
    virtual void notify(const CharacteristicHandle& handle, bool enable) = 0;
    //virtual ResultVoid queue() = 0;
};


// -------------------- BLE Support abstraction (Qt) --------------------
// Implement this with QLowEnergyService/Controller if you like.
class IBleSupport : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    virtual ~IBleSupport() = default;

    virtual std::optional<CharacteristicHandle> getCharacteristic(const QString& uuid) const = 0;
    virtual ResultVoid enableNotifications(const CharacteristicHandle& handle) = 0;

    // Non-blocking write. Must emit writeCompleted() when done.
    virtual void writeCharacteristic(const QString& taskName,
                                     const CharacteristicHandle& handle,
                                     const QByteArray& data) = 0;

    virtual QSharedPointer<ITransaction> createTransaction(const QString& name) = 0;

signals:
    void writeCompleted(const QString& taskName, bool ok, const QString& error);
};






// -------------------- CommunicatorV2 --------------------
class CommunicatorV2 : public QObject {
    Q_OBJECT
public:
    explicit CommunicatorV2(QSharedPointer<IBleSupport> ble, QObject* parent=nullptr);
    ~CommunicatorV2() override = default;



public slots:
    // device lifecycle
    void initializeDevice();
    void initializeDeviceWithTransaction(const QString& transactionName);


    // service lifecycle
    void registerService(Service service, bool reliable);
    void closeService(Service service);

    // async-response emulation
    void submitGfdiResponse(quint64 requestId, const QByteArray& response);



signals:
    void initialized(bool ok);

    // GFDI delivery
    void gfdiMessageReceived(const QByteArray& gfdiMessage);
    void gfdiMessageReceivedNeedResponse(quint64 requestId, const QByteArray& gfdiMessage);

    // other service delivery
    void serviceMessageReceived(Service service, const QByteArray& data);

    // BLE write primitive (decouple transport)
    void writeToBle(const QString& taskName, const CharacteristicHandle& handle, const QByteArray& data);

    void errorOccurred(const QString& error);

private slots:
    void onBleWriteCompleted(const QString& taskName, bool ok, const QString& error);

private:
    // builders
    QByteArray createCloseAllServicesMessage() const;
    QByteArray createRegisterServiceMessage(Service service, bool reliable) const;
    QByteArray createCloseServiceMessage(Service service, quint8 handle) const;





    // write queue
    void enqueueWrite(const QString& taskName, const QByteArray& data);
    void pumpWriteQueue();


private:
    QSharedPointer<IBleSupport> ble_;

    // ---- State guarded by mutex ----
    QMutex mutex_;
    std::optional<CharacteristicHandle> sendCh_;
    std::optional<CharacteristicHandle> recvCh_;
    QHash<quint8, Service> serviceByHandle_;
    QHash<Service, quint8> handleByService_;
    QHash<Service, QSharedPointer<IServiceCallback>> serviceCallbacks_;

    int maxWriteSize_{20}; // MTU-3 then possibly limited by device max

    // COBS for non-MLR path (streaming)
    QScopedPointer<CobsCoDec> cobs_;

    // Sequential write queue (replaces Rust await writes)
    QQueue<QPair<QString, QByteArray>> pendingWrites_;
    bool writeInFlight_{false};

    // request-id counter for gfdiMessageReceivedNeedResponse
    quint64 requestSeq_{1};
    QHash<quint64, QByteArray> pendingResponses_;
};
