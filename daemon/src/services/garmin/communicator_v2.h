#pragma once

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QPointer>
#include <QtCore/QSharedPointer>
#include <QtCore/QMutex>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>
#include <optional>
#include <functional>

// Supporting classes assumed to exist (per your instruction)
#include "garmintypes.h"
#include "cobscodec.h"
#include "garminmlr.h"
#include "garminmessages.h"
#include "communicator_v2.h"
#include <qbledevice.h>
#include <qbleservice.h>

// =============================================================================
// Constants (Rust: BASE_UUID_FORMAT, GADGETBRIDGE_CLIENT_ID)
// =============================================================================
static constexpr const char* BASE_UUID_FORMAT = "6A4E%04X-667B-11E3-949A-0800200C9A66";
static constexpr quint64 GADGETBRIDGE_CLIENT_ID = 2;

// =============================================================================
// BLE abstraction (Rust: trait BleSupport + trait Transaction)
// =============================================================================


class Transaction {
public:
    virtual ~Transaction() = default;
    virtual void write(const QBLECharacteristic& handle, const QByteArray& data) = 0;
    virtual void notify(const QBLECharacteristic& handle, bool enable) = 0;
    virtual Result<void> queue() = 0;
};

// BLE support uses signals/slots so we can “await” like Rust async.
class BleSupport : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;

    virtual ~BleSupport() = default;

    // Rust: fn get_characteristic(&self, uuid: &str) -> Option<QBLECharacteristic>
    virtual QBLECharacteristic getCharacteristic(const QString& uuid) = 0;

    // Rust: fn enable_notifications(&self, handle: &QBLECharacteristic) -> Result<()>
    virtual Result<void> enableNotifications(const QBLECharacteristic& handle) = 0;

    // Rust: async fn write_characteristic(...)
public slots:
    virtual void writeCharacteristic(const QBLECharacteristic& handle,
                                     const QByteArray& data,
                                     const QString& taskName) = 0;

signals:
    void writeCompleted(const QString& taskName);
    void writeFailed(const QString& taskName, const QString& error);

public:
    // Rust: fn create_transaction(&self, name: &str) -> Box<dyn Transaction>
    virtual QSharedPointer<Transaction> createTransaction(const QString& name) = 0;
};



class ServiceWriter {
public:
    virtual ~ServiceWriter() = default;
    virtual Result<void> write(const QString& taskName, const QByteArray& data) = 0;
};

class ServiceCallback {
public:
    virtual ~ServiceCallback() = default;

    virtual Result<void> onConnect(QSharedPointer<ServiceWriter> writer) {
        Q_UNUSED(writer);
        return Result<void>::isOk();
    }
    virtual Result<void> onClose() { return Result<void>::isOk(); }
    virtual Result<void> onMessage(const QByteArray& data) = 0;
};

// =============================================================================
// Internal state (Rust: struct CommunicatorState)
// =============================================================================

struct CommunicatorState {
    QBLECharacteristic *characteristicSend = NULL;
    QBLECharacteristic *characteristicReceive = NULL;
    QString sendPath;
    QString receivePath;

    QMap<quint8, Service> serviceByHandle;
    QMap<Service, quint8> handleByService;

    QMap<Service, QSharedPointer<ServiceCallback>> serviceCallbacks;
    QMap<quint8, QSharedPointer<MlrCommunicator>> mlrCommunicators; // key: mlr_handle

    int maxWriteSize {20}; // Rust default
    CobsCoDec cobsCodec;   // Rust: CobsCoDec::new()
    static QSharedPointer<CommunicatorState> create() {
        return QSharedPointer<CommunicatorState>(new CommunicatorState());
    }
};




// =============================================================================
// CommunicatorV2 (Rust: struct CommunicatorV2)
// =============================================================================

class CommunicatorV2 : public QBLEService {
    Q_OBJECT
public:
    explicit CommunicatorV2(const QString &path, QObject* parent=nullptr);

    static const char* BASE_UUID;
    static const char* UUID_SERVICE_GARMIN_ML_GFDI;

    static QString baseUuid(quint16 shortId); // helper for BASE_UUID_FORMAT

    // Rust: set_message_callback / set_async_message_callback
    void setMessageCallback(QSharedPointer<GfdiMessageCallback> cb);
    void setAsyncMessageCallback(QPointer<AsyncGfdiMessageCallback> cb);

    // Rust: register_service_callback / unregister_service_callback
    void registerServiceCallback(Service service, QSharedPointer<ServiceCallback> cb);
    QSharedPointer<ServiceCallback> unregisterServiceCallback(Service service);

    // Rust: get_receive_characteristic_uuid
    std::optional<QString> getReceiveCharacteristicUuid() const;

    // Rust: on_mtu_changed / on_device_max_packet_size
    void onMtuChanged(int mtu);
    void onDeviceMaxPacketSize(quint16 deviceMaxPacketSize);

    // Rust: initialize_device / initialize_device_with_transaction
    Result<bool> initializeDevice();
    //Result<bool> initializeDeviceWithTransaction(const QString& transactionName);

    // Rust: send_message / send_message_with_transaction
    Result<void> sendMessage(const QString& taskName, const QByteArray& message);
    Result<void> sendMessageWithTransaction(const QString& taskName, const QByteArray& message);


    // Rust: handle_decoded_message_async
    Result<std::optional<QByteArray>> handleDecodedMessageAsync(const QByteArray& decodedWithHandle);

    // Rust: register_service / close_service
    Result<void> registerService(Service service, bool reliable);
    Result<void> closeService(Service service);

    // Rust: register_handle / dispose
    void registerHandle(Service service, quint8 handle);
    Result<void> dispose();

    // Rust: connection state helpers
    void onConnectionStateChange(bool connected);
    void pauseMlr();
    void resumeMlr();
    void clearAndPauseMlr();

signals:
    void logDebug(const QString& msg);
    void logInfo(const QString& msg);
    void logWarn(const QString& msg);
    void logError(const QString& msg);

    void gfdiMessageReceived(const QByteArray& gfdiMessage);

public slots:
    void characteristicRead(const QString &c, const QByteArray &value);
    // Rust: on_characteristic_changed
    void onCharacteristicChanged(const QString &characteristic, const QByteArray& data);


private:
    Result<void> awaitBleWrite(const QBLECharacteristic& handle,
                               const QByteArray& bytes,
                               const QString& taskName);

    Result<std::optional<QByteArray>> awaitAsyncCallback(const QByteArray& message);

    // Rust: process_handle_management + process_*_resp
    Result<void> processHandleManagement(const QByteArray& message);
    Result<void> processRegisterMlResp(const QByteArray& payload);
    Result<void> processCloseHandleResp(const QByteArray& payload);
    Result<void> processCloseAllResp();

    // Rust: handle_decoded_message
    Result<void> handleDecodedMessage(const QByteArray& decodedWithHandle);

    // Rust: create_* messages
    QByteArray createCloseAllServicesMessage() const;
    QByteArray createRegisterServiceMessage(Service service, bool reliable) const;
    QByteArray createCloseServiceMessage(Service service, quint8 handle) const;

    quint64 nextCookie();

private:
    mutable QMutex m_mutex;
    QSharedPointer<CommunicatorState> m_state;
    QSharedPointer<BleSupport> m_ble;

    QSharedPointer<GfdiMessageCallback> m_syncCb;
    QPointer<AsyncGfdiMessageCallback> m_asyncCb;

    quint64 m_cookieCounter {1};
    QString m_Path;
    QObject *m_device;
    bool isFirstConncet=true;
};

/*
// =============================================================================
// MlrServiceWriter (Rust: struct MlrServiceWriter implements ServiceWriter)
// =============================================================================

class MlrServiceWriter : public ServiceWriter {
public:
    MlrServiceWriter(quint8 handle,
                     QSharedPointer<BleSupport> ble,
                     QString sendPath);

    Result<void> write(const QString& taskName, const QByteArray& data) override;

private:
    Result<void> awaitBleWrite(const QString& taskName, const QByteArray& bytes);

    quint8 m_handle;
    QSharedPointer<BleSupport> m_ble;
    QBLECharacteristic m_sendChar;
};

// =============================================================================
// Example service callbacks (Rust: GfdiServiceCallback, Realtime*, FileTransfer*)
// =============================================================================

class GfdiServiceCallback : public ServiceCallback {
public:
    explicit GfdiServiceCallback(QSharedPointer<GfdiMessageCallback> cb);

    Result<void> onConnect(QSharedPointer<ServiceWriter> writer) override;
    Result<void> onClose() override;
    Result<void> onMessage(const QByteArray& data) override;

private:
    QSharedPointer<GfdiMessageCallback> m_cb;
    CobsCoDec m_codec;
};

class RealtimeHeartRateCallback : public ServiceCallback {
public:
    using Handler = std::function<Result<void>(quint8)>;
    explicit RealtimeHeartRateCallback(Handler h);

    Result<void> onConnect(QSharedPointer<ServiceWriter> writer) override;
    Result<void> onClose() override;
    Result<void> onMessage(const QByteArray& data) override;

private:
    Handler m_handler;
};

class RealtimeStepsCallback : public ServiceCallback {
public:
    using Handler = std::function<Result<void>(quint32)>;
    explicit RealtimeStepsCallback(Handler h);

    Result<void> onConnect(QSharedPointer<ServiceWriter> writer) override;
    Result<void> onClose() override;
    Result<void> onMessage(const QByteArray& data) override;

private:
    Handler m_handler;
};

class FileTransferCallback : public ServiceCallback {
public:
    using Handler = std::function<Result<void>(QByteArray)>;
    explicit FileTransferCallback(Handler onComplete);

    Result<void> onConnect(QSharedPointer<ServiceWriter> writer) override;
    Result<void> onClose() override;
    Result<void> onMessage(const QByteArray& data) override;

private:
    QByteArray m_data;
    Handler m_onComplete;
};

*/
