#include "garmincommunicator.h"

// include your real implementations
#include "cobscodec.h"      // CobsCoDec
#include "garmincommunicator.h"   // MlrCommunicator (Qt)

#include <QDateTime>



// -------------------- CommunicatorV2 --------------------
CommunicatorV2::CommunicatorV2(QSharedPointer<IBleSupport> ble, QObject* parent)
    : QObject(parent), ble_(std::move(ble)), cobs_(new CobsCoDec(this))
{
    connect(ble_.data(), &IBleSupport::writeCompleted,
            this, &CommunicatorV2::onBleWriteCompleted);

    // default write routing: Communicator emits writeToBle -> IBleSupport performs actual write
    connect(this, &CommunicatorV2::writeToBle, this,
            [this](const QString& task, const CharacteristicHandle& ch, const QByteArray& data) {
                ble_->writeCharacteristic(task, ch, data);
            });
}



// -------------------- MTU / device max --------------------



// -------------------- Initialise device --------------------


void CommunicatorV2::initializeDeviceWithTransaction(const QString& transactionName) {
    QMutexLocker lock(&mutex_);
    auto tx = ble_->createTransaction(transactionName);
    if (!tx) { emit initialized(false); return; }

    for (quint16 i = 0x2810; i <= 0x2814; ++i) {
        const QString recvUuid = uuidFor(i);
        const QString sendUuid = uuidFor(i + 0x10);

        auto recv = ble_->getCharacteristic(recvUuid);
        auto send = ble_->getCharacteristic(sendUuid);

        if (recv && send) {
            recvCh_ = *recv;
            sendCh_ = *send;

            tx->notify(*recv, true);
            tx->write(*send, createCloseAllServicesMessage());

            auto r = tx->queue();
            if (r.has_value()) {
                emit errorOccurred(r->message);
                emit initialized(false);
            } else {
                emit initialized(true);
            }
            return;
        }
    }

    emit initialized(false);
}


