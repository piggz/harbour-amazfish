#ifndef ASYNCMESSAGEHANDLER_H
#define ASYNCMESSAGEHANDLER_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QQueue>
#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QSemaphore>
#include <QtCore/QTimer>
#include <QtCore/QElapsedTimer>
#include <QtCore/QCoreApplication>

#include <optional>

#include "garmintypes.h"
#include "datatransferhandler.h"
#include "communicator_v2.h"

// Forward declarations
class CommunicatorV2;
class GarminNotificationHandler;
class WatchdogManager;
class DataTransferHandler;

enum class WeatherProviderType : int;   // assumed
class UnifiedWeatherProvider;          // assumed

// Max protobuf data size per PROTOBUF_RESPONSE chunk (tested on Garmin devices)
static constexpr int MAX_PROTOBUF_CHUNK_SIZE = 3072;

// request_id -> pending protobuf chunks to send incrementally
struct PendingProtobufChunk {
    QByteArray completePayload; // Rust: Vec<u8>
    int totalLength = 0;        // Rust: usize
    quint16 messageType = 0;    // Rust: message_type (u16)
    quint16 requestId = 0;      // Rust: request_id (u16)
};

class AsyncMessageHandler : public QObject
{
    Q_OBJECT
public:
    explicit AsyncMessageHandler(QObject* parent = nullptr);
     ~AsyncMessageHandler() override = default;
     void setCommunicator(CommunicatorV2* comm);
     void setNotificationHandler(const QSharedPointer<GarminNotificationHandler>& handler);

     // Rust: set_calendar_manager(&self, manager: Arc<CalendarManager>)
     //void setCalendarManager(const QSharedPointer<CalendarManager>& manager);
     bool sendResponse(const QByteArray& response);
     bool sendProtobufResponse(const QByteArray& message);
     bool sendProtobufChunk(quint16 requestId,
                                    quint32 dataOffset,
                                    const QByteArray& chunkData,
                                    int totalLength);
     bool handleProtobufChunkAck(quint16 requestId, quint32 dataOffset);

     void parse(const UnknownMessage& msg);

 signals:
     void logInfo(const QString& msg);
     void logDebug(const QString& msg);
     void logWarn(const QString& msg);
     void logError(const QString& msg);

 private slots:
     void processQueueTick();

 private:
     static quint16 computeCrc16A001(const QByteArray& data);



 private:
     // Mirrors Rust fields (Arc<Mutex<...>> -> QMutex + value)
     mutable QMutex mMutex;

     CommunicatorV2* mCommunicator;
     bool mInitializationComplete = false;
     QSharedPointer<GarminNotificationHandler> mNotificationHandler;
     bool mPairingDetected = false;

     QQueue<QByteArray> mMessageQueue;                             // VecDeque<Vec<u8>>
     QSemaphore mSendSemaphore;                                    // Semaphore(1)

     QSharedPointer<WatchdogManager> mWatchdog;                    // Option<Arc<WatchdogManager>>
     QSharedPointer<DataTransferHandler> mDataTransferHandler;     // Arc<DataTransferHandler>

     //QSharedPointer<CalendarManager> mCalendarManager;             // Option<Arc<CalendarManager>>

     QHash<quint16, PendingProtobufChunk> mPendingProtobufChunks;  // HashMap<u16, PendingProtobufChunk>

     QSharedPointer<UnifiedWeatherProvider> mWeatherProvider;      // Arc<UnifiedWeatherProvider>

     // Queue processor timer (Tokio loop replacement)
     QTimer m_queueTimer;

     // Release delay timer per send (50ms); implemented with singleShot
};

#endif // ASYNCMESSAGEHANDLER_H
