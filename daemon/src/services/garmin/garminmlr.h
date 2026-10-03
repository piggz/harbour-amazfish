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
// Constants
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
// Fragment
// =============================================================================
struct Fragment {
    QString taskName;
    int     num {0};
    QByteArray data;
    int reqNum{0};
};

// =============================================================================
// MessageSender / MessageReceiver
// =============================================================================

class MlrMessageSender : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    MlrMessageSender(QSharedPointer<QBLECharacteristic> sendChar,
                 QObject* parent=nullptr);
    virtual ~MlrMessageSender() = default;

public slots:
    Result<void> sendPacket(const QString& taskName, const QByteArray& packet);

signals:
    void sendSucceeded(const QString& taskName);
    void sendFailed(const QString& taskName, const QString& error);

private:

    QSharedPointer<QBLECharacteristic> m_sendChar;
};

// =============================================================================
// Callbacks
// =============================================================================

class GfdiMessageCallback : public QObject{
public:
    GfdiMessageCallback() {};
     ~GfdiMessageCallback() = default;
     void onMessage(const QByteArray& message);
};


// Receiver is async-like via queued slot.
class MlrMessageReceiver : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    MlrMessageReceiver(QSharedPointer<GfdiMessageCallback> syncCb,
                    QObject* parent=nullptr);


    virtual ~MlrMessageReceiver() = default;

public slots:
    void onDataReceived(const QByteArray& data);
    void onDataDecoded(const QByteArray &decoded);



signals:
    void gfdiDecoded(const QByteArray& gfdiMessage);
    void receiverError(const QString& error);

private:

    QSharedPointer<GfdiMessageCallback> m_syncCb;

    CobsCoDec m_codec;
};

// =============================================================================
// MlrCommunicator
// =============================================================================
class MlrCommunicator : public QObject {
    Q_OBJECT
public:
    explicit MlrCommunicator(quint8 handle,
                             int maxPacketSize,
                             QSharedPointer<MlrMessageSender> sender,
                             QSharedPointer<MlrMessageReceiver> receiver,
                             QObject* parent=nullptr);

    void setMaxPacketSize(int maxPacketSize);

    Result<void> start();

    void sendMessage(const QString& taskName, const QByteArray& message);

    void onPacketReceived(const QByteArray& packet);


    void close();

    ~MlrCommunicator() override;

signals:

    void errorOccurred(const QString& msg);

private slots:
    void onRetransmissionTimeout();

private:
    struct State {
        int handle {0};
        int maxPacketSize {20};

        int lastSendAck {0};
        int nextSendSeq {0};
        int nextRcvSeq {0};
        int lastRcvAck {0};

        int maxNumUnackedSend {INITIAL_MAX_UNACKED_SEND};
        int retransmissionTimeoutMs {INITIAL_RETRANSMISSION_TIMEOUT_MS};

        QQueue<Fragment> fragmentQueue;
        //QList<Fragment> sentFragments;

        QVector<std::optional<Fragment>> sentFragments;

        bool paused {false};

        State() {
            //sentFragments.resize(int(MAX_SEQ_NUM) + 1);
        }
    };
    QTimer* m_retransmissionTimer;
    QTimer* m_ackTimer;



    QByteArray createPacket(int reqNum, int seqNum, const QByteArray& data);


    void processAck(int reqNum);

    void scheduleAck();

    void sendAckPacket();

    void runProtocolOnce();

    void startRetransmissionTimer();



    mutable QMutex m_mutex;
    State m_state;

    QSharedPointer<MlrMessageSender> m_sender;
    QSharedPointer<MlrMessageReceiver> m_receiver;


    bool m_running {false};
};


#endif // GARMINMLR__H
