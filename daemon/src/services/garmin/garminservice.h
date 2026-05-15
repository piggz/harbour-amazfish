#ifndef GARMINSERVICE__H
#define GARMINSERVICE__H

//#include "abstractoperationservice.h"
#include "garminmlr.h"
#include "cobscodec.h"
#include "garmintypes.h"

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
#include <qmap.h>
#include <qbledevice.h>
#include <qbleservice.h>

static constexpr quint64 GADGETBRIDGE_CLIENT_ID = 2;


// -------------------- MLR adapters --------------------
// These glue the communicator to your MlrCommunicator and your BLE write primitive.
/*
class MlrBleSender : public QObject {
    Q_OBJECT
public:
    //MlrBleSender(QSharedPointer<IBleSupport> ble,
    MlrBleSender(QSharedPointer<QBLEService> ble,
                 QBLECharacteristic sendCh,
                 QObject* parent=nullptr)
        : QObject(parent), mBle(std::move(ble)), mSendCh(std::move(sendCh)) {}

public slots:
    void sendPacket(const QString& taskName, const QByteArray& packet);

private:
    QSharedPointer<QBLEService> mBle;
    QBLECharacteristic mSendCh;
};

*/

class MlrGfdiReceiver : public QObject {
    Q_OBJECT
public:
    explicit MlrGfdiReceiver(QObject* parent=nullptr);

public slots:
    void onDataReceived(const QByteArray& data);

signals:
    // decoded == raw GFDI message: [size:2][msg_id:2]...[crc:2] (NO handle byte)
    void gfdiDecoded(const QByteArray& decoded);


private:
    CobsCoDec* mCodec; // persistent to support multi-packet messages
};


// -------------------- Service Writer / Callback (Qt) --------------------
class IServiceWriter : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
public slots:
    virtual void write(const QString& taskName, const QByteArray& data) = 0;
};

class IServiceCallback : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
public slots:
    virtual void onConnect(IServiceWriter* writer) { Q_UNUSED(writer); }
    virtual void onClose() {}
    virtual void onMessage(const QByteArray& data) = 0;
};


class GarminService : public QBLEService
{
    Q_OBJECT
/*
    class Service {
    public:
        enum Value {
            GFDI = 1,
            REGISTRATION = 4,
            REALTIME_HR = 6,
            REALTIME_STEPS = 7,
            REALTIME_CALORIES = 8,
            REALTIME_INTENSITY = 10,
            REALTIME_HRV = 12,
            REALTIME_STRESS = 13,
            REALTIME_ACCELEROMETER = 16,
            REALTIME_SPO2 = 19,
            REALTIME_BODY_BATTERY = 20,
            REALTIME_RESPIRATION = 21,
            FILE_TRANSFER_2 = 0x2018,
            FILE_TRANSFER_4 = 0x4018,
            FILE_TRANSFER_6 = 0x6018,
            FILE_TRANSFER_A = 0xa018,
            FILE_TRANSFER_C = 0xc018,
            FILE_TRANSFER_E = 0xe018
        };

    private:
        short mCode;

    public:
        explicit Service(int code) : mCode(static_cast<short>(code)) {}

        short getCode() const {
            return mCode;
        }

        static std::optional<Service> fromCode(int code) {
            static const std::vector<Service> services = {
                Service(GFDI),
                Service(REGISTRATION),
                Service(REALTIME_HR),
                Service(REALTIME_STEPS),
                Service(REALTIME_CALORIES),
                Service(REALTIME_INTENSITY),
                Service(REALTIME_HRV),
                Service(REALTIME_STRESS),
                Service(REALTIME_ACCELEROMETER),
                Service(REALTIME_SPO2),
                Service(REALTIME_BODY_BATTERY),
                Service(REALTIME_RESPIRATION),
                Service(FILE_TRANSFER_2),
                Service(FILE_TRANSFER_4),
                Service(FILE_TRANSFER_6),
                Service(FILE_TRANSFER_A),
                Service(FILE_TRANSFER_C),
                Service(FILE_TRANSFER_E)
            };

            for (const auto& service : services) {
                if (service.mCode == code) {
                    return service;
                }
            }
            return std::nullopt;
        }
    };
    */
/*
    enum RequestType {
            REGISTER_ML_REQ = 0,
            REGISTER_ML_RESP = 1,
            CLOSE_HANDLE_REQ = 2,
            CLOSE_HANDLE_RESP = 3,
            UNK_HANDLE = 4,
            CLOSE_ALL_REQ = 5,
            CLOSE_ALL_RES = 6,
            UNK_REQ = 7,
            UNK_RESP = 8
     };
*/


public:
    explicit GarminService(const QString &path, QObject *parent);
    static const char* BASE_UUID;
    static const char* UUID_SERVICE_GARMIN_ML_GFDI;
    int mMaxWriteSize = 20;

    bool initializeDevice();
    void setServiceCallback(Service service, QSharedPointer<IServiceCallback> cb);
    void removeServiceCallback(Service service);

public slots:
    // service lifecycle
    void registerService(Service service, bool reliable);
    void closeService(Service service);

    // send/receive
    void sendMessage(const QString& taskName, const QByteArray& gfdiMessage);
    void sendMessageWithTransaction(const QString& taskName, const QByteArray& gfdiMessage);
    void onCharacteristicChanged(const QByteArray& data);


    // async-response emulation
    void submitGfdiResponse(quint64 requestId, const QByteArray& response);

    // MTU / device max size
    void onMtuChanged(int mtu);
    void onDeviceMaxPacketSize(quint16 deviceMaxPacketSize);

signals:
    void initialized(bool ok);

    // GFDI delivery
    void gfdiMessageReceived(const QByteArray& gfdiMessage);
    void gfdiMessageReceivedNeedResponse(quint64 requestId, const QByteArray& gfdiMessage);

    // other service delivery
    void serviceMessageReceived(Service service, const QByteArray& data);

    // BLE write primitive (decouple transport)
    void writeToBle(const QString& taskName, const QBLECharacteristic& handle, const QByteArray& data);

    void errorOccurred(const QString& error);

private:
    QString mPath;
    QBLECharacteristic *mCharacteristicSend = NULL;
    QBLECharacteristic *mCharacteristicReceive = NULL;
    QObject *mDevice;

    QMap<int, Service> mServiceByHandle;
    QMap<Service, int> mHandleByService;
    //QMap<Service, ServiceCallback> mServiceCallbacks;

    QMap<int, MlrCommunicator> mMlrServices;

    bool mRealtimeHrOneShot = false;
    int mPreviousSteps = -1;

    int calcMaxWriteChunk(int mtu);

    // helpers
    static QString uuidFor(quint16 shortId);
    static quint16 u16le(const QByteArray& b, int off);
    static quint64 u64le(const QByteArray& b, int off);

    // write queue
    void enqueueWrite(const QString& taskName, const QByteArray& data);
    void pumpWriteQueue();



    QByteArray createCloseAllServicesMessage() const;
    QByteArray createRegisterServiceMessage(Service service, bool reliable) const;
    QByteArray createCloseServiceMessage(Service service, quint8 handle) const;

    // connection handling + MLR management
    void onConnectionStateChanged(bool connected);
    void pauseMlr();
    void resumeMlr();
    void clearAndPauseMlr();

    // inbound handling
    void processHandleManagement(const QByteArray& payload);
    void processRegisterMlResp(const QByteArray& payload);
    void processCloseHandleResp(const QByteArray& payload);
    void processCloseAllResp();

    void handleDecodedFrame(const QByteArray& decodedFrame); // decoded frame begins with handle byte
    void dispatchServiceMessage(quint8 handle, const QByteArray& payload);

    QScopedPointer<CobsCoDec> mCobs;
    QMutex mMutex;
    bool mWriteInFlight{false};
    QQueue<QPair<QString, QByteArray>> mPendingWrites;

    // MLR keyed by mlrHandle (bits 0-3)
    QHash<quint8, QSharedPointer<MlrCommunicator>> mMlrByHandle;

    QHash<Service, QSharedPointer<IServiceCallback>> mServiceCallbacks;

    // request-id counter for gfdiMessageReceivedNeedResponse
    quint64 mRequestSeq{1};
    QHash<quint64, QByteArray> mPendingResponses;

    // Sequential write queue (replaces Rust await writes)
    QQueue<QPair<QString, QByteArray>> pendingWrites_;


private slots:
    void characteristicRead(const QString &c, const QByteArray &value);
    void onBleWriteCompleted(const QString& taskName, bool ok, const QString& error);


};




#endif // GARMINSERVICE__H
