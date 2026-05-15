#ifndef GARMINMLR__H
#define GARMINMLR__H

#include <QObject>
#include <QByteArray>
#include <QQueue>
#include <QVector>
#include <QMutex>
#include <QTimer>
#include <QElapsedTimer>
#include <QString>
#include <QSharedPointer>
#include <QPointer>

#include "garmintypes.h"
#include <qbleservice.h>
#include "cobscodec.h"



// =============================================================================
// Constants (copied 1:1 from mlr.rs) [1](https://computacenter-my.sharepoint.com/personal/thomas_michel_computacenter_com/Documents/Microsoft%20Copilot%20Chat%20Files/mlr.rs)
// =============================================================================
static constexpr quint8  MLR_FLAG_MASK   = 0x80;
static constexpr quint8  HANDLE_MASK     = 0x70;
static constexpr quint8  HANDLE_SHIFT    = 4;
static constexpr quint8  REQ_NUM_MASK    = 0x0F;
static constexpr quint8  SEQ_NUM_MASK    = 0x3F;
static constexpr quint8  MAX_SEQ_NUM     = 0x3F;

static constexpr int     INITIAL_MAX_UNACKED_SEND            = 0x20;
static constexpr int     MAX_RETRANSMISSION_TIMEOUT_MS       = 20000;
static constexpr int     INITIAL_RETRANSMISSION_TIMEOUT_MS   = 1000;
static constexpr int     ACK_TIMEOUT_MS                      = 250;
static constexpr int     ACK_TRIGGER_THRESHOLD               = 5;

// =============================================================================
// Fragment (Rust: struct Fragment) [1](https://computacenter-my.sharepoint.com/personal/thomas_michel_computacenter_com/Documents/Microsoft%20Copilot%20Chat%20Files/mlr.rs)
// =============================================================================
struct Fragment {
    QString taskName;
    int     num {0};
    QByteArray data;
};

// =============================================================================
// MessageSender / MessageReceiver (Rust traits) [1](https://computacenter-my.sharepoint.com/personal/thomas_michel_computacenter_com/Documents/Microsoft%20Copilot%20Chat%20Files/mlr.rs)
// =============================================================================

// Sender is async-like via signals (to model Rust .await).
class MlrMessageSender : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    MlrMessageSender(QBLECharacteristic *sendChar,
                 QObject* parent=nullptr);
    virtual ~MlrMessageSender() = default;

public slots:
    Result<void> sendPacket(const QString& taskName, const QByteArray& packet);

signals:
    void sendSucceeded(const QString& taskName);
    void sendFailed(const QString& taskName, const QString& error);

private:
    Result<void> awaitBleWrite(const QString& taskName, const QByteArray& bytes);

    QBLECharacteristic *m_sendChar;
};

// =============================================================================
// Callbacks (Rust: GfdiMessageCallback / AsyncGfdiMessageCallback / ServiceWriter / ServiceCallback)
// =============================================================================

class GfdiMessageCallback {
public:
    virtual ~GfdiMessageCallback() = default;
    virtual Result<void> onMessage(const QByteArray& message) = 0;
};

// Async callback uses signals to deliver reply; we will “await” it via QEventLoop.
class AsyncGfdiMessageCallback : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    virtual ~AsyncGfdiMessageCallback() = default;

public slots:
    virtual void onMessage(const QByteArray& message, quint64 cookie) = 0;

signals:
    void replyReady(quint64 cookie, const QByteArray& reply);
    void noReply(quint64 cookie);
    void failed(quint64 cookie, const QString& error);
};

// Receiver is async-like via queued slot.
class MlrMessageReceiver : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    MlrMessageReceiver(QSharedPointer<GfdiMessageCallback> syncCb,
                    QPointer<AsyncGfdiMessageCallback> asyncCb,
                    QObject* parent=nullptr);


    virtual ~MlrMessageReceiver() = default;

public slots:
    Result<void> onDataReceived(const QByteArray& data);

signals:
    void gfdiDecoded(const QByteArray& gfdiMessage);
    void receiverError(const QString& error);

private:
    Result<std::optional<QByteArray>> awaitAsyncCallback(const QByteArray& message);

    QSharedPointer<GfdiMessageCallback> m_syncCb;
    QPointer<AsyncGfdiMessageCallback> m_asyncCb;

    CobsCoDec m_codec;
};

// =============================================================================
// MlrCommunicator (Rust: struct MlrCommunicator) [1](https://computacenter-my.sharepoint.com/personal/thomas_michel_computacenter_com/Documents/Microsoft%20Copilot%20Chat%20Files/mlr.rs)
// =============================================================================
class MlrCommunicator : public QObject {
    Q_OBJECT
public:
    explicit MlrCommunicator(quint8 handle,
                             int maxPacketSize,
                             QSharedPointer<MlrMessageSender> sender,
                             QSharedPointer<MlrMessageReceiver> receiver,
                             QObject* parent=nullptr);

    // Rust: set_max_packet_size(&self, ...) async
    void setMaxPacketSize(int maxPacketSize);

    // Rust: start(&mut self) -> Result<()>
    Result<void> start();

    // Rust: send_message(&self, ...) async -> Result<()>
    Result<void> sendMessage(const QString& taskName, const QByteArray& message);

    // Rust: on_packet_received(&self, ...) async -> Result<()>
    Result<void> onPacketReceived(const QByteArray& packet);

    // Rust: pause/resume/clear_and_pause
    void pause();
    void resume();
    void clearAndPause();

    // Rust: close(&mut self)
    void close();

    ~MlrCommunicator() override;

signals:
    void debugLog(const QString& msg);
    void warnLog(const QString& msg);
    void errorOccurred(const QString& msg);

private slots:
    void onTick();

private:
    // ---- state (Rust: struct MlrState) ----
    struct State {
        quint8 handle {0};
        int maxPacketSize {20};

        quint8 lastSendAck {0};
        quint8 nextSendSeq {0};
        quint8 nextRcvSeq {0};
        quint8 lastRcvAck {0};

        int maxNumUnackedSend {INITIAL_MAX_UNACKED_SEND};
        int retransmissionTimeoutMs {INITIAL_RETRANSMISSION_TIMEOUT_MS};

        QQueue<Fragment> fragmentQueue;
        QVector<std::optional<Fragment>> sentFragments; // size 64

        std::optional<qint64> lastAckTimeMs;
        std::optional<qint64> lastRetransmitTimeMs;

        bool paused {false};

        State() {
            sentFragments.resize(int(MAX_SEQ_NUM) + 1);
        }
    };

    // helper monotonic clock like Rust Instant
    qint64 nowMs() const;

    // Create packet (Rust: create_packet)
    static QByteArray createPacket(const State& st, quint8 reqNum, quint8 seqNum, const QByteArray& data);

    // seq_diff (Rust: seq_diff)
    static int seqDiff(quint8 a, quint8 b);

    // process_ack (Rust: process_ack)
    static Result<void> processAck(State& st, quint8 reqNum);

    // schedule_ack (Rust: schedule_ack)
    static void scheduleAck(State& st, qint64 nowMs);

    // send_ack_packet (Rust: send_ack_packet)
    Result<void> sendAckPacketLocked(const State& st);

    // run_protocol (Rust: run_protocol)
    Result<void> runProtocolOnce();

    // check_ack_timeout (Rust: check_ack_timeout)
    Result<void> checkAckTimeout();

    // check_retransmit_timeout (Rust: check_retransmit_timeout)
    Result<void> checkRetransmitTimeout();

    // clear_state (Rust: clear_state)
    void clearStateLocked(State& st);

    // await a send using sender signals (models Rust await)
    Result<void> awaitSend(const QString& taskName, const QByteArray& packet);


    mutable QMutex m_mutex;
    State m_state;

    QSharedPointer<MlrMessageSender> m_sender;
    QSharedPointer<MlrMessageReceiver> m_receiver;

    QTimer m_timer;
    QElapsedTimer m_clock;

    bool m_running {false};
};


#endif // GARMINMLR__H
