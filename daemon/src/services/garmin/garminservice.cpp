#include "garminservice.h"
#include "devices/garmin/garmindevice.h"
#include "garmintypes.h"
#include "cobscodec.h"      // CobsCoDec
#include "garminmlr.h"   // MlrCommunicator (Qt)
#include "garmintypes.h"
#include "garminmessages.h"

#include <QDomDocument>
#include <QByteArray>
#include <QBuffer>
#include <QDataStream>
#include <QObject>

//const char* GarminService::BASE_UUID                   = "6a4e%04X-667b-11e3-949a-0800200c9a66";
const char* GarminService::BASE_UUID                   = "6a4e%1-667b-11e3-949a-0800200c9a66";
const char* GarminService::UUID_SERVICE_GARMIN_ML_GFDI = "6a4e2800-667b-11e3-949a-0800200c9a66";



static inline void pushU16le(QByteArray& out, quint16 v) {
    out.append(char(v & 0xFF));
    out.append(char((v >> 8) & 0xFF));
}
static inline void pushU64le(QByteArray& out, quint64 v) {
    for (int i=0;i<8;i++) out.append(char((v >> (8*i)) & 0xFF));
}

/*

// -------------------- MlrBleSender --------------------
void MlrBleSender::sendPacket(const QString& taskName, const QByteArray& packet) {
    Q_UNUSED(taskName);
    mSendCh.writeAsync(packet);
}

// -------------------- MlrGfdiReceiver --------------------
MlrGfdiReceiver::MlrGfdiReceiver(QObject* parent)
    : QObject(parent),
      mCodec(new CobsCoDec(this))
{
}

*/

void MlrGfdiReceiver::onDataReceived(const QByteArray& data) {
    // data is COBS encoded (MLR stripped header already)
    mCodec->receiveBytes(data);
    auto msg = mCodec->retrieveMessage();
    if (msg.has_value() && !msg->isEmpty()) {
        emit gfdiDecoded(*msg);
    }
}


GarminService::GarminService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_GARMIN_ML_GFDI, path, parent), mCobs(new CobsCoDec(this))
{
    qDebug() << "Garmin Service created for " << path;
    mPath = path;
    mDevice = parent;

    connect(this, &QBLEService::characteristicRead, this, &GarminService::characteristicRead);
    //enableNotification();

    /*
    connect(ble_.data(), &IBleSupport::writeCompleted,
            this, &CommunicatorV2::onBleWriteCompleted);

    // default write routing: Communicator emits writeToBle -> IBleSupport performs actual write
    connect(this, &GarminService::writeToBle, this,
            [this](const QString& task, const QBLECharacteristic& ch, const QByteArray& data) {
                this->writeAsync(task, data);
            });
*/

    initializeDevice();
}

// -------------------- Helpers --------------------
QString GarminService::uuidFor(quint16 shortId) {
    return QString("6A4E%1-667B-11E3-949A-0800200C9A66")
        .arg(shortId, 4, 16, QLatin1Char('0'))
        .toUpper();
}

quint16 GarminService::u16le(const QByteArray& b, int off) {
    return quint16(quint8(b[off])) | (quint16(quint8(b[off+1])) << 8);
}

quint64 GarminService::u64le(const QByteArray& b, int off) {
    quint64 v = 0;
    for (int i = 0; i < 8; ++i)
        v |= (quint64(quint8(b[off + i])) << (8 * i));
    return v;
}

bool GarminService::initializeDevice() {
    // Iterate through the known ML characteristics until we find a known pair
    // send characteristic = read characteristic + 0x10 (eg. 2810 / 2820)
    //QString servicePath = (dynamic_cast<QBLEDevice* > (mDevice))->devicePath();
    qDebug() << "Garmin Service path " << mPath;

    QMap<QString, QBLECharacteristic*> characteristicMap;
    QMutexLocker lock(&mMutex);


    QDBusInterface miIntro("org.bluez", mPath, "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus(), 0);
    QDBusReply<QString> xml = miIntro.call("Introspect");
    QDomDocument doc;

    doc.setContent(xml.value());

    QDomNodeList nodes = doc.elementsByTagName("node");

    for (int x = 0; x < nodes.count(); x++)
    {
        QDomElement node = nodes.at(x).toElement();
        QString nodeName = node.attribute("name");

        if (nodeName.startsWith("char")) {
            QString path = mPath + "/" + nodeName;
            QDBusInterface charInterface("org.bluez", path, "org.bluez.GattCharacteristic1", QDBusConnection::systemBus(), 0);
            characteristicMap[charInterface.property("UUID").toString()] = new QBLECharacteristic(path, this);
        }
    }

    qDebug() << "Garmin characteristics" << characteristicMap.keys();

    for (int i = 2810; i <= 2814; i++) {
        QString serviceRec = QString(GarminService::BASE_UUID).arg(i);
        QString serviceSnd = QString(GarminService::BASE_UUID).arg(i+10);
        qDebug() << "Checking for Garmin    UUID " << serviceRec <<" and " << serviceSnd;

        if (characteristicMap.contains(serviceRec) && characteristicMap.contains(serviceSnd))
        {
            mCharacteristicSend = characteristicMap.value(serviceRec);
            mCharacteristicReceive = characteristicMap.value(serviceSnd);
            qDebug() << " Garmin ML Characteristic found";
        }


        if ((mCharacteristicSend != NULL) && (mCharacteristicReceive != NULL)) {

            //builder.notify(characteristicReceive, true);
            mPendingWrites.clear();
            mWriteInFlight = false;
            mCharacteristicSend->writeAsync(createCloseAllServicesMessage());
            lock.unlock();
            pumpWriteQueue();
            return true;
        }
    }

    qDebug() << "Failed to find any known Garmin ML characteristics";

    return false;
}

void GarminService::characteristicRead(const QString &c, const QByteArray &value)
{

        qDebug() << Q_FUNC_INFO << c << "=================" << value;
        qDebug() << "Charateristics Read called";

}


QByteArray GarminService::createCloseAllServicesMessage() const {
    QByteArray b;
    b.reserve(13);
    b.append(char(0));
    b.append(char(quint8(RequestType::CloseAllReq)));
    pushU64le(b, GADGETBRIDGE_CLIENT_ID);
    pushU16le(b, 0);
    b.append(char(0)); // padding
    return b;
}

QByteArray GarminService::createRegisterServiceMessage(Service service, bool reliable) const {
    QByteArray b;
    b.reserve(13);
    b.append(char(0));
    b.append(char(quint8(RequestType::RegisterMlReq)));
    pushU64le(b, GADGETBRIDGE_CLIENT_ID);
    pushU16le(b, serviceCode(service));
    b.append(char(reliable ? 2 : 0));
    return b;
}

QByteArray GarminService::createCloseServiceMessage(Service service, quint8 handle) const {
    QByteArray b;
    b.reserve(12);
    b.append(char(0));
    b.append(char(quint8(RequestType::CloseHandleReq)));
    pushU64le(b, GADGETBRIDGE_CLIENT_ID);
    pushU16le(b, serviceCode(service));
    b.append(char(handle));
    return b;
}


void GarminService::onMtuChanged(int mtu) {
    QMutexLocker lock(&mMutex);
    mMaxWriteSize = qMax(1, mtu - 3);
    for (auto& mlr : mMlrByHandle) {
        mlr->setMaxPacketSize(mMaxWriteSize);
    }
    //mMaxWriteSize = calcMaxWriteChunk(mtu);
}

void GarminService::onDeviceMaxPacketSize(quint16 deviceMaxPacketSize) {
    QMutexLocker lock(&mMutex);
    const int devMax = int(deviceMaxPacketSize);
    if (devMax < mMaxWriteSize) {
        mMaxWriteSize = devMax;
        for (auto& mlr : mMlrByHandle) {
            mlr->setMaxPacketSize(mMaxWriteSize);
        }
    }
}


int GarminService::calcMaxWriteChunk(int mtu) {
    // the minimum MTU is 23 (Bluetooth spec)
    int safeMtu = std::max(23, mtu);

    // GATT_MAX_ATTR_LEN: no larger than 512 (Bluetooth spec)
    // MTU: overhead of simple write must be supported. Some other operations like
    //      ATT_PREPARE_WRITE_REQ have even larger overhead so the max BLE MTU is larger than 512+3
    return std::min(512, safeMtu - 3);
}

void GarminService::enqueueWrite(const QString& taskName, const QByteArray& data) {
    mPendingWrites.enqueue({taskName, data});
}

void GarminService::pumpWriteQueue() {
    QMutexLocker lock(&mMutex);
    if (mWriteInFlight) return;
    if (!mCharacteristicSend) return;
    if (mPendingWrites.isEmpty()) return;

    auto item = mPendingWrites.dequeue();
    mWriteInFlight = true;

    //const auto ch = mCharacteristicSend;
    const QString task = item.first;
    const QByteArray data = item.second;
    lock.unlock();
    mCharacteristicSend->writeAsync(data);
}

void GarminService::onBleWriteCompleted(const QString& taskName, bool okWrite, const QString& error) {
    Q_UNUSED(taskName);
    // TODO if (!okWrite) emit errorOccurred(error);

    {
        QMutexLocker lock(&mMutex);
        mWriteInFlight = false;
    }
    pumpWriteQueue();
}



// -------------------- Connection + MLR management --------------------
void GarminService::onConnectionStateChanged(bool connected) {
    if (!connected) clearAndPauseMlr();
}

void GarminService::pauseMlr() {
    QMutexLocker lock(&mMutex);
    for (auto& mlr : mMlrByHandle) mlr->pause();
}

void GarminService::resumeMlr() {
    QMutexLocker lock(&mMutex);
    for (auto& mlr : mMlrByHandle) mlr->resume();
}

void GarminService::clearAndPauseMlr() {
    QMutexLocker lock(&mMutex);
    for (auto& mlr : mMlrByHandle) mlr->clearAndPause();
    mMlrByHandle.clear();
}


// -------------------- Service ops --------------------
void GarminService::registerService(Service service, bool reliable) {
    sendMessage(QString("register_%1").arg(serviceCode(service)),
                createRegisterServiceMessage(service, reliable));
}

void GarminService::closeService(Service service) {
    quint8 handle = 0;
    {
        QMutexLocker lock(&mMutex);
        if (!mHandleByService.contains(service)) {
            // TODO emit errorOccurred(QString("Service %1 not registered").arg(serviceCode(service)));
            return;
        }
        handle = mHandleByService[service];
    }
    sendMessage(QString("close_%1").arg(serviceCode(service)),
                createCloseServiceMessage(service, handle));
}

// -------------------- Async response emulation --------------------
void GarminService::submitGfdiResponse(quint64 requestId, const QByteArray& response) {
    Q_UNUSED(requestId);
    // simplest behaviour: send immediately
    sendMessage(QString("gfdi_response_%1").arg(requestId), response);
}

// -------------------- Send message --------------------
void GarminService::sendMessage(const QString& taskName, const QByteArray& gfdiMessage) {
    if (gfdiMessage.isEmpty()) {
        // TODO emit errorOccurred(GarminError::emptyMessage().message);
        return;
    }

    quint8 gfdiHandle = 0;
    QBLECharacteristic *sendCh=NULL;
    int maxWrite = 20;
    QSharedPointer<MlrCommunicator> mlr;

    {
        QMutexLocker lock(&mMutex);
        if (!mHandleByService.contains(Service::GFDI)) {
            // TODO emit errorOccurred(GarminError::gfdiHandleNotSet().message);
            return;
        }
        gfdiHandle = mHandleByService[Service::GFDI];
        sendCh = mCharacteristicSend;
        maxWrite = mMaxWriteSize;

        const quint8 mlrHandle = gfdiHandle & 0x0F;
        if (mMlrByHandle.contains(mlrHandle))
            mlr = mMlrByHandle[mlrHandle];
    }

    const QByteArray payload = CobsCoDec::encode(gfdiMessage);

    // If MLR exists, send via MLR
    if (mlr) {
        mlr->sendMessage(taskName, payload);
        return;
    }

    // Else raw fragmentation: [handle][cobs...]
    if (!sendCh) {
        // TODO emit errorOccurred(GarminError::bt("Send characteristic not set").message);
        return;
    }

    const int maxChunk = qMax(1, maxWrite - 1);

    {
        QMutexLocker lock(&mMutex);
        mPendingWrites.clear();
        mWriteInFlight = false;

        for (int pos = 0; pos < payload.size(); pos += maxChunk) {
            QByteArray fragment;
            fragment.reserve(1 + qMin(maxChunk, payload.size() - pos));
            fragment.append(char(gfdiHandle));
            fragment.append(payload.mid(pos, maxChunk));
            enqueueWrite(taskName, fragment);
        }
    }
    pumpWriteQueue();
}

void GarminService::sendMessageWithTransaction(const QString& taskName, const QByteArray& gfdiMessage) {
    if (gfdiMessage.isEmpty()) {
        // TODO emit errorOccurred(GarminError::emptyMessage().message);
        return;
    }

    // TODO
    /*
    auto tx = ble_->createTransaction(taskName);
    if (!tx) { emit errorOccurred("Transaction creation failed"); return; }
*/
    quint8 gfdiHandle = 0;
    QBLECharacteristic *sendCh=NULL;
    int maxWrite = 20;

    {
        QMutexLocker lock(&mMutex);
        if (!mHandleByService.contains(Service::GFDI)) {
            // TODO emit errorOccurred(GarminError::gfdiHandleNotSet().message);
            return;
        }
        gfdiHandle = mHandleByService[Service::GFDI];
        sendCh = mCharacteristicSend;
        maxWrite = mMaxWriteSize;
    }

    if (!sendCh) {
        // TODO emit errorOccurred(GarminError::bt("Send characteristic not set").message);
        return;
    }

    const QByteArray payload = CobsCoDec::encode(gfdiMessage);
    const int maxChunk = qMax(1, maxWrite - 1);

    for (int pos = 0; pos < payload.size(); pos += maxChunk) {
        QByteArray fragment;
        fragment.reserve(1 + qMin(maxChunk, payload.size() - pos));
        fragment.append(char(gfdiHandle));
        fragment.append(payload.mid(pos, maxChunk));
        // TODO tx->write(*sendCh, fragment);
    }
// TODO
    /*
    auto r = tx->queue();
    if (r.has_value()) emit errorOccurred(r->message);
    */
}


// -------------------- Handle management --------------------
void GarminService::processHandleManagement(const QByteArray& message) {
    if (message.isEmpty()) {
        // TODO emit errorOccurred(GarminError::invalid("Empty handle management message").message); return;
    }
    if (message.size() < 9) {
        // TODO emit errorOccurred("Handle management message too short"); return;
    }

    const quint8 requestType = quint8(message[0]);
    const quint64 clientId = u64le(message, 1);
    if (clientId != GADGETBRIDGE_CLIENT_ID) return;

    const QByteArray payload = message.mid(9);

    switch (requestType) {
    case quint8(RequestType::RegisterMlResp):  processRegisterMlResp(payload); break;
    case quint8(RequestType::CloseHandleResp): processCloseHandleResp(payload); break;
    case quint8(RequestType::CloseAllResp):    processCloseAllResp(); break;
    default:
        // Requests or unknown responses: ignore like Rust
        break;
    }
}

void GarminService::processRegisterMlResp(const QByteArray& payload) {
    if (payload.size() < 5) {
        // TODO emit errorOccurred("RegisterMlResp payload too short"); return;
    }

    const quint16 code = u16le(payload, 0);
    const quint8 status = quint8(payload[2]);
    const quint8 serviceHandle = quint8(payload[3]);
    const quint8 reliable = quint8(payload[4]);

    auto svc = serviceFromCode(code);
    if (!svc.isOk(code)) return;
    if (status != 0) return;

    // Extract MLR handle if reliable
    const unsigned char mlrHandle = (reliable != 0) ? (serviceHandle & 0x0F) : serviceHandle;

    {
        QMutexLocker lock(&mMutex);
        mServiceByHandle[serviceHandle] = svc.value();
        mHandleByService[svc.value()] = serviceHandle;
    }

    // If reliable, create MLR communicator
    if (reliable != 0) {
        std::optional<QBLECharacteristic *> sendCh;
        {
            QMutexLocker lock(&mMutex);
            sendCh = mCharacteristicSend;
        }
        if (!sendCh.has_value()) return;

        // Create BLE sender adapter
        //  auto sender = new MlrBleSender(ble_, *sendCh, this);

        // Receiver adapter -> emits gfdiDecoded
        auto receiver = new MlrGfdiReceiver(this);
        connect(receiver, &MlrGfdiReceiver::gfdiDecoded, this, [this](const QByteArray& decodedGfdi) {
            emit gfdiMessageReceived(decodedGfdi);
            const quint64 reqId = mRequestSeq++;
            emit gfdiMessageReceivedNeedResponse(reqId, decodedGfdi);
        });

        // Create MLR communicator (assumed API)
        auto mlr = QSharedPointer<MlrCommunicator>::create(mlrHandle, mMaxWriteSize, this);

        // Wire: MLR wants to send -> sender does BLE write
        connect(mlr.data(), SIGNAL(packetToSend(QString,QByteArray)),
                //sender, SLOT(sendPacket(QString,QByteArray)));
                mCharacteristicSend, SLOT(writeAsync(QByteArray)));

        // Wire: MLR produces decoded bytes (COBS-encoded chunk stream) -> receiver decodes to GFDI
        connect(mlr.data(), SIGNAL(dataReceived(QByteArray)),
                receiver, SLOT(onDataReceived(QByteArray)));


        {
            QMutexLocker lock(&mMutex);
            mMlrByHandle[mlrHandle] = mlr;
        }
        mlr->start();
    }

    // Service callback onConnect: provide a writer (optional; can be expanded)
    QSharedPointer<IServiceCallback> cb;
    {
        QMutexLocker lock(&mMutex);
        cb = mServiceCallbacks.value(svc.value());
    }
    if (cb) {
        cb->onConnect(nullptr);
    }
}

void GarminService::processCloseHandleResp(const QByteArray& payload) {
    if (payload.size() < 4) {
        // TODO emit errorOccurred("CloseHandleResp payload too short"); return;
    }

    const quint16 code = u16le(payload, 0);
    const quint8 handle = quint8(payload[2]);

    auto svc = serviceFromCode(code);

    QSharedPointer<IServiceCallback> cb;
    {
        QMutexLocker lock(&mMutex);
        mServiceByHandle.remove(handle);
        if (svc.isOk()) {
            mHandleByService.remove(svc.value());
            cb = mServiceCallbacks.value(svc.value());
            mServiceCallbacks.remove(svc.value());
        }
        mMlrByHandle.remove(handle); // best-effort
    }

    if (cb) cb->onClose();
}

void GarminService::processCloseAllResp() {
    QHash<Service, QSharedPointer<IServiceCallback>> callbacks;
    std::optional<QBLECharacteristic *> sendCh;

    {
        QMutexLocker lock(&mMutex);
        mServiceByHandle.clear();
        mHandleByService.clear();
        callbacks = mServiceCallbacks;
        mServiceCallbacks.clear();
        mMlrByHandle.clear();
        sendCh = mCharacteristicSend;
    }

    // notify all closes
    for (auto cb : callbacks) if (cb) cb->onClose();

    // re-register GFDI in reliable mode
    if (sendCh.has_value()) {
        enqueueWrite("register_gfdi_after_close_all", createRegisterServiceMessage(Service::GFDI, true));
        pumpWriteQueue();
    }
}


void GarminService::setServiceCallback(Service service, QSharedPointer<IServiceCallback> cb) {
    QMutexLocker lock(&mMutex);
    mServiceCallbacks[service] = std::move(cb);
}

void GarminService::removeServiceCallback(Service service) {
    QMutexLocker lock(&mMutex);
    mServiceCallbacks.remove(service);
}


// -------------------- Receive notifications --------------------
void GarminService::onCharacteristicChanged(const QByteArray& data) {
    if (data.isEmpty()) return;

    // MLR packet? (bit 7 set)
    if (data.size() >= 2 && (quint8(data[0]) & 0x80u) != 0) {
        const quint8 mlrHandle = (quint8(data[0]) & 0x70u) >> 4;

        QSharedPointer<MlrCommunicator> mlr;
        {
            QMutexLocker lock(&mMutex);
            if (mMlrByHandle.contains(mlrHandle))
                mlr = mMlrByHandle[mlrHandle];
        }
        if (mlr) {
            mlr->onPacketReceived(data);
            return;
        }
        // else fall through to raw path
    }

    // handle management (handle 0)
    const quint8 handle = quint8(data[0]);
    if (handle == 0) {
        processHandleManagement(data.mid(1));
        return;
    }

    // Non-MLR stream COBS decode
    mCobs->receiveBytes(data);
    auto msg = mCobs->retrieveMessage();
    if (msg.has_value() && !msg->isEmpty()) {
        handleDecodedFrame(*msg);
    }
}

void GarminService::handleDecodedFrame(const QByteArray& decodedFrame) {
    if (decodedFrame.isEmpty()) return;
    const quint8 handle = quint8(decodedFrame[0]);
    dispatchServiceMessage(handle, decodedFrame.mid(1));
}

void GarminService::dispatchServiceMessage(quint8 handle, const QByteArray& payload) {
    QSharedPointer<IServiceCallback> cb;
    std::optional<Service> svc;

    {
        QMutexLocker lock(&mMutex);
        if (mServiceByHandle.contains(handle)) {
            svc = mServiceByHandle[handle];
            cb = mServiceCallbacks.value(svc.value());
        }
    }

    if (!svc.has_value()) return;

    // GFDI: emit signal
    if (*svc == Service::GFDI) {
        // emit both styles:
        emit gfdiMessageReceived(payload);

        const quint64 reqId = mRequestSeq++;
        emit gfdiMessageReceivedNeedResponse(reqId, payload);
        return;
    }

    // Other services
    if (cb) cb->onMessage(payload);
    emit serviceMessageReceived(*svc, payload);
}





