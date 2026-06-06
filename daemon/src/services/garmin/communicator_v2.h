#ifndef COMMUNICATORV2__H
#define COMMUNICATORV2__H

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
#include "garmin/garmindevice.h"
#include "garmintypes.h"
#include "cobscodec.h"
#include "garminmlr.h"
#include "garminmessages.h"
#include "garminnotification.h"
#include <qbledevice.h>
#include <qbleservice.h>
#include <KDbConnection.h>

#include <optional>


// =============================================================================
// Constants
// =============================================================================
static constexpr const char* BASE_UUID_FORMAT = "6A4E%04X-667B-11E3-949A-0800200C9A66";
static constexpr quint64 AMAZFISH_CLIENT_ID = 2; // use 3 to not clash with Gadgetbridge or Garmin Connect


struct deviceInfo {
    QString softwareRevision;
    QString serialNumber;
    QString systemId;
    QString deviceName;
    QString deviceModel;
    QString deviceManufacturer;
};

// Writer for sending messages to a service ->needed? Or use QBLECharacteristic?
class ServiceWriter {
public:
    virtual ~ServiceWriter() = default;
    void write(const QString& taskName, const QByteArray& data);
};




// =============================================================================
// Internal state (Rust: struct CommunicatorState)
// =============================================================================

//Rust characteristicHandle = QBLECharacteristic

struct CommunicatorState {
    QSharedPointer<QBLECharacteristic> characteristicSend;
    QSharedPointer<QBLECharacteristic> characteristicReceive;

    QMap<quint8, Service> serviceByHandle;
    QMap<Service, quint8> handleByService;

    QMap<Service, QSharedPointer<ServiceCallback>> serviceCallbacks;
    QMap<quint8, QSharedPointer<MlrCommunicator>> mlrCommunicators; // key: mlr_handle

    int maxWriteSize {20};
    CobsCoDec cobsCodec;
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
    static QSharedPointer<CommunicatorV2> create(const QString &path, QObject* parent=nullptr) {
        return QSharedPointer<CommunicatorV2>(new CommunicatorV2(path,parent));
    }
    explicit CommunicatorV2(const QString &path, QObject* parent=nullptr);

    static const char* BASE_UUID;
    static const char* UUID_SERVICE_GARMIN_ML_GFDI;

    static QString baseUuid(quint16 shortId); // helper for BASE_UUID_FORMAT

    // set_message_callback
    void setMessageCallback(QSharedPointer<GfdiMessageCallback> cb);


    // register_service_callback / unregister_service_callback
    void registerServiceCallback(Service service, QSharedPointer<ServiceCallback> cb);
    QSharedPointer<ServiceCallback> unregisterServiceCallback(Service service);

    //  get_receive_characteristic_uuid
    std::optional<QString> getReceiveCharacteristicUuid() const;

    //  on_mtu_changed / on_device_max_packet_size
    void onMtuChanged(int mtu);
    void onDeviceMaxPacketSize(quint16 deviceMaxPacketSize);

    //  initialize_device
    bool initializeDevice();

    //  send_message
    bool sendMessage(const QString& taskName, const QByteArray& message);

    //  handle_decoded_message_async
    Result<std::optional<QByteArray>> handleDecodedMessageAsync(const QByteArray& decodedWithHandle);

    //  register_service / close_service
    void registerService(Service service, bool reliable);
    void closeService(Service service);

    //  register_handle / dispose
    void registerHandle(Service service, quint8 handle);
    void dispose();

    // Complete pairing
    bool completePairing();


    // Rust: connection state helpers
    void onConnectionStateChange(bool connected);
    void pauseMlr();
    void resumeMlr();
    void clearAndPauseMlr();

    // return data
    quint32 steps() {return mSteps;};
    quint32 stepsGoal() {return mStepsGoal;};
    quint32 heartRate() {return mHeartRate;};
    quint16 HRV() {return mHRV;};
    quint8 spo2() {return mSpo2;};
    struct deviceInfo deviceInfo() {return mDeviceInfo;};
    quint8 batteryLevel() {return mBatteryLevel;};

signals:
    void logDebug(const QString& msg);
    void logInfo(const QString& msg);
    void logWarn(const QString& msg);
    void logError(const QString& msg);
    void mlrConnected();
    void pairingComplete();
    void stepsChanged(quint32 &steps);
    void heartRateChanged(quint8 &hr);
    void informationChanged(Amazfish::Info infoKey, const QString& infoValue);

    void gfdiMessageReceived(const QByteArray& gfdiMessage);

public slots:
    void characteristicRead(const QString &c, const QByteArray &value);

    void onCharacteristicChanged(const QString &characteristic, const QByteArray& data);

    void onDeviceInformationReceived(DeviceInformationMessage &message);
    void onConfigurationReceived(const ConfigurationMessage& msg);
    void onCurrentTimeRequestReceived();
    void onNotificationControlReceived(const NotificationControlMessage& msg);
    void onNotificationSubscriptionReceived(const NotificationSubscriptionMessage& msg);
    void onSynchronizationReceived(const SynchronizationMessage& msg);
    void onFilterStatusReceived(const FilterStatusMessage& msg);
    void onWeatherRequestReceived(const WeatherRequestMessage& msg);
    void onUnknownMessageReceived(const UnknownMessage &msg);


    void setSteps(quint32 val);
    void setStepsGoal(quint32);
    void setHeartRate(quint8 val);
    void setHRV(quint16 val);
    void setSpo2(quint8 val);

    // Register services
    void registerServices();


private:
    //  process_handle_management
    void processHandleManagement(const QByteArray& message);
    void processRegisterMlResp(const QByteArray& payload);
    void processCloseHandleResp(const QByteArray& payload);
    void processCloseAllResp();

    //  handle_decoded_message
    void handleDecodedMessage(const QByteArray& decodedWithHandle);

    // create messages
    QByteArray createCloseAllServicesMessage() const;
    QByteArray createRegisterServiceMessage(Service service, bool reliable) const;
    QByteArray createCloseServiceMessage(Service service, quint8 handle) const;

    quint64 nextCookie();

    void saveHRVRecord();
    void saveSpo2Record();

    quint32 mSteps=0;
    quint32 mStepsGoal=10000;
    quint8 mHeartRate=0;
    quint8 mHRV=0;
    quint8 mSpo2=0;
    quint8 mBatteryLevel;

    struct deviceInfo mDeviceInfo;

    mutable QMutex m_mutex;
    QSharedPointer<CommunicatorState> mState;

    QSharedPointer<GfdiMessageCallback> mMessageCallback;
    QPointer<AsyncGfdiMessageCallback> mAsyncMessageCallback;

    quint64 m_cookieCounter {1};
    QString m_Path;
    QObject *m_device = nullptr;
    bool isFirstConncet=true;
    bool isPairing=false;
};


// =============================================================================
// MlrServiceWriter
// =============================================================================

class MlrServiceWriter : public ServiceWriter {
public:
    MlrServiceWriter(quint8 handle,
                     QSharedPointer<QBLECharacteristic> sendChar);


    void write(const QString& taskName, const QByteArray& data);

private:

    quint8 m_handle;
    QSharedPointer<QBLECharacteristic> m_sendChar;
};


// =============================================================================
// Example service callbacks
// =============================================================================

class GfdiServiceCallback : public ServiceCallback {
public:
    explicit GfdiServiceCallback(QSharedPointer<GfdiMessageCallback> cb, CommunicatorV2* parent);

    void onConnect(QSharedPointer<ServiceWriter> writer);
    void onClose() override;
    void onMessage(const QByteArray& data) override;

private:
    QSharedPointer<GfdiMessageCallback> m_cb;
    CobsCoDec m_codec;
    CommunicatorV2 *mCommunicator;
};

class RealtimeHeartRateCallback : public ServiceCallback {
public:
    explicit RealtimeHeartRateCallback(CommunicatorV2* parent);

    void onConnect(QSharedPointer<ServiceWriter> writer);
    void onClose() override;
    void onMessage(const QByteArray& data) override;

private:
    CommunicatorV2 *mCommunicator;

};

class RealtimeSpo2Callback : public ServiceCallback {
public:
    explicit RealtimeSpo2Callback(CommunicatorV2* parent);

    void onConnect(QSharedPointer<ServiceWriter> writer);
    void onClose() override;
    void onMessage(const QByteArray& data) override;

private:
    CommunicatorV2 *mCommunicator;

};

class RealtimeHRVCallback : public ServiceCallback {
public:
    explicit RealtimeHRVCallback(CommunicatorV2* parent);

    void onConnect(QSharedPointer<ServiceWriter> writer);
    void onClose() override;
    void onMessage(const QByteArray& data) override;

private:
    CommunicatorV2 *mCommunicator;

};


class RealtimeStepsCallback : public ServiceCallback {
public:
    explicit RealtimeStepsCallback(CommunicatorV2* parent);

    void onConnect(QSharedPointer<ServiceWriter> writer);
    void onClose() override;
    void onMessage(const QByteArray& data) override;

private:
    CommunicatorV2 *mCommunicator;

};



#endif //_COMMUNICATOR__H
