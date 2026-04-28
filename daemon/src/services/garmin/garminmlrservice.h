#ifndef GARMINMLRSERVICE__H
#define GARMINMLRSERVICE__H

#include "qble/qbleservice.h"
#include <qlist.h>
#include <qtimer.h>

class GarminMlrService : public QObject
{
    Q_OBJECT
public:

    static const uint8_t MLR_FLAG_MASK = 0x80;
    static const uint8_t HANDLE_MASK = 0x70;
    static const uint8_t HANDLE_SHIFT = 4;
    static const uint8_t REQ_NUM_MASK = 0x0F;
    static const uint8_t SEQ_NUM_MASK = 0x3F;




    GarminMlrService( int handle,
                      int maxPacketSize,
                      void (*sendPacket)(QString taskName, QByteArray packet),
                      void (*onDataReceived)( QByteArray data)
                     );
    void setMaxPacketSize( int maxPacketSize);
    void sendMessage(QString taskName, QByteArray message);
    void onPacketReceived(QByteArray packet);
    void close();
    void onConnectionStateChange();

private:
    static const int MAX_SEQ_NUM = 0x3F;
    static const int INITIAL_MAX_UNACKED_SEND = 0x20;
    static const int MAX_RETRANSMISSION_TIMEOUT = 20000;
    static const int INITIAL_RETRANSMISSION_TIMEOUT = 1000;
    static const int ACK_TIMEOUT = 250;
    static const int ACK_TRIGGER_THRESHOLD = 5;

    int mHandle;
    int mMaxPacketSize;

    int mLastSendAck = 0x00;
    int mNextSendSeq = 0x00;
    int mNextRcvSeq = 0x00;
    int mLastRcvAck = 0x00;
    int mMaxNumUnackedSend = INITIAL_MAX_UNACKED_SEND;
    int mRetransmissionTimeout = INITIAL_RETRANSMISSION_TIMEOUT;

    struct Fragment {
        QString taskName;   // task name
        int num;                // numeric identifier
        QByteArray data; // byte array data
        int reqNum;             // request number

        // Constructor matching the record component order
        Fragment(const QString& taskName,
                 int num,
                 const QByteArray data,
                 int reqNum)
            : taskName(taskName),
              num(num),
              data(data),
              reqNum(reqNum) {}
    };

    QList<Fragment> mFragmentQueue;
    QList<Fragment> mSentFragments;
    QTimer *mRetransmissionTimer;
    QTimer *mAckTimer;

    //Handler timeoutHandler = new Handler(Looper.getMainLooper());
    //Runnable ackRunnable = this::sendAckPacket;
    //Runnable retransmissionRunnable = this::onRetransmissionTimeout;

    void  (*mSendPacket)(QString,QByteArray);
    void  (*mOnDataReceived)(QByteArray data);

    void processAck(const int reqNum);
    void scheduleAckIfNeeded();
    void sendAckPacket();
    void runProtocol();
    QByteArray createPacket(const int reqNum, const int seqNum, const QByteArray data);
    void startRetransmissionTimer();


 public slots:

    void onRetransmissionTimeout();



};

#endif // GARMINMLRSERVICE__H
