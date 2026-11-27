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
static constexpr int     ACK_TIMEOUT_MS                      = 20;
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
// MlrCommunicator
// =============================================================================
class MlrCommunicator : public QObject {
    Q_OBJECT
public:
    explicit MlrCommunicator(quint8 handle,
                             int maxPacketSize,
                             QSharedPointer<QBLECharacteristic> sendChar,
                             QObject* parent=nullptr);

    void setMaxPacketSize(int maxPacketSize);
    void sendMessage(const QString& taskName, const QByteArray& message);
    void onPacketReceived(const QByteArray& packet);
    ~MlrCommunicator() override;

signals:
    void errorOccurred(const QString& msg);
    void gfdiDecoded(const QByteArray& gfdiMessage);

private slots:
    void onRetransmissionTimeout();
    void onAckTimeout();
    void onMessageDecoded(const QByteArray &decoded);

private:
    QByteArray processPacket(int reqNum, int seqNum, const QByteArray&  data) ;
    void processAck(int reqNum);
    void scheduleAck();
    void sendAckPacket();
    void runProtocolOnce();
    void startRetransmissionTimer();


    QTimer* mRetransmissionTimer;
    QTimer* mAckTimer;
    QByteArray createPacket(int reqNum, int seqNum, const QByteArray& packet);
    QSharedPointer<QBLECharacteristic> mSendChar;
    CobsCoDec mCobsCodec;

    int mHandle {0};
    int mMaxPacketSize {20};
    int mLastSendAck {0};
    int mNextSendSeq {0};
    int mNextRcvSeq {0};
    int mLastRcvAck {0};
    int mMaxNumUnackedSend {INITIAL_MAX_UNACKED_SEND};
    int mRetransmissionTimeoutMs {INITIAL_RETRANSMISSION_TIMEOUT_MS};
    QQueue<Fragment> mFragmentQueue;
    QVector<std::optional<Fragment>> mSentFragments;
    bool mPaused {false};
    QMutex mLocker;
};


#endif // GARMINMLR__H
