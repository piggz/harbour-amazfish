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

#include "garmin/garmindevice.h"
#include "garmintypes.h"
#include "cobscodec.h"
#include "garminmlr.h"
#include "weather/currentweather.h"
#include "garminprotobufmessage.h"

#include <qbledevice.h>
#include <qbleservice.h>
#include <KDbConnection.h>

#include <optional>

//forward declarations
class ProtobufHandler;

// =============================================================================
// Constants
// =============================================================================
static constexpr const char* BASE_UUID_FORMAT = "6A4E%04X-667B-11E3-949A-0800200C9A66";
static constexpr quint64 AMAZFISH_CLIENT_ID = 2;
struct deviceInfo {

    QString softwareRevision;
    QString serialNumber;
    QString systemId;
    QString deviceName;
    QString deviceModel;
    QString deviceManufacturer;
};

struct DeviceInformationMessage {
    quint16 protocolVersion{};
    quint16 productNumber{};
    quint32 unitNumber{};
    quint16 softwareVersion{};
    quint16 maxPacketSize{};
    QString bluetoothFriendlyName;
    QString deviceName;
    QString deviceModel;
};





// =============================================================================
// Internal state (Rust: struct CommunicatorState)
// =============================================================================

//Rust characteristicHandle = QBLECharacteristic

struct CommunicatorState {
    QSharedPointer<QBLECharacteristic> characteristicSend;
    QSharedPointer<QBLECharacteristic> characteristicReceive;

    QString UUIDSend;
    QString UUIDReceive;

    QMap<quint8, Service> serviceByHandle;
    QMap<Service, quint8> handleByService;

    QMap<Service, QSharedPointer<ServiceCallback>> serviceCallbacks;
    QMap<quint8, QSharedPointer<MlrCommunicator>> mlrCommunicators; // key: mlr_handle

    int maxWriteSize {20};
    CobsCoDec *cobsCodec=nullptr;
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


    static const char* BASE_UUID;//                   = "6a4e%1-667b-11e3-949a-0800200c9a66";
    static const char* UUID_SERVICE_GARMIN_ML_GFDI;//  = "6a4e2800-667b-11e3-949a-0800200c9a66";
    static const char* UUID_SERVICE_GARMIN_V0_SEND;// = "df334c80-e6a7-d082-274d-78fc66f85e16";
    static const char* UUID_SERVICE_GARMIN_V0_RECV;// = "4acbcd28-7425-868e-f447-915c8f00d0cb";
    static const char* UUID_SERVICE_GARMIN_V1_SEND;// = "6a4e4c80-667b-11e3-949a-0800200c9a66";
    static const char* UUID_SERVICE_GARMIN_V1_RECV;// = "6a4ecd28-667b-11e3-949a-0800200c9a66";

    static QString baseUuid(quint16 shortId); // helper for BASE_UUID_FORMAT

    void setStatus(const QString &status);
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
    void sendRawBytes(const QString &label, const QByteArray &bytes);
    void processSendQueue();

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


    void onConnectionStateChange(bool connected);


    //hadle incoming GFDI  messages
    void onDeviceInformationReceived(DeviceInformationMessage &message);
    void onConfigurationReceived();
    void onNotificationControlReceived(const NotificationControlMessage& msg);
    void onSynchronizationReceived(const SynchronizationMessage& msg);
    void onFilterStatusReceived(const FilterStatusMessage& msg);
    void onWeatherRequestReceived(const WeatherRequestMessage& msg);
    void onProtobufMessageReceived(const QByteArray& data);
    void onProtobufStatusMessageReceived(const QByteArray& data);



    // return data
    quint32 steps() {return mSteps;};
    quint32 stepsGoal() {return mStepsGoal;};
    quint32 heartRate() {return mHeartRate;};
    quint16 HRV() {return mHRV;};
    quint8 spo2() {return mSpo2;};
    struct deviceInfo deviceInfo() {return mDeviceInfo;};
    quint8 batteryLevel() {return mBatteryLevel;};
    bool isConnected() {return mConnected;};
    bool handshakeComplete() {return !isFirstConnect;};

signals:
    void statusChanged();
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

    void setSteps(quint32 val);
    void setStepsGoal(quint32);
    void setHeartRate(quint8 val);
    void setHRV(quint16 val);
    void setSpo2(quint8 val);
    void setBatteryLevel(quint8 val);

    // Register services
    void registerServices();
    void getBatteryLevel();


private:
    //  process_handle_management
    void processHandleManagement(const QByteArray& message);
    void processRegisterMlResp(const QByteArray& payload);
    void processCloseHandleResp(const QByteArray& payload);
    void processCloseAllResp();

    //handle non Mlr Messages
    void handleNonMlrMessage(const QByteArray& data);

    //handle non reliable GFDI Messages
    void handleIncomingGfdiMessage(const QByteArray& data);
    //  handle MLR decoded_message
    void handleDecodedMessage(const QByteArray& decodedWithHandle);

    // create messages
    QByteArray createCloseAllServicesMessage() const;
    QByteArray createRegisterServiceMessage(Service service, bool reliable) const;
    QByteArray createCloseServiceMessage(Service service, quint8 handle) const;

    // cleanup
    void cleanup();

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
    //QPointer<AsyncGfdiMessageCallback> mAsyncMessageCallback;
    QSharedPointer<ProtobufHandler> mProtobufHandler;


    quint64 m_cookieCounter {1};
    QString m_Path;
    QObject *m_device = nullptr;
    bool isFirstConnect=true;
    bool mConnected=false;
    bool mServicesResolved = false;
    bool mIsMlProtocol = false;
    bool mIsReliable = false;
    QString mStatus;
    QTimer* mBatteryTimer;
    QQueue<QPair<QByteArray, QString>> mSendQueue;
    bool  mSendInProgress = false;
};





#endif //_COMMUNICATOR__H
