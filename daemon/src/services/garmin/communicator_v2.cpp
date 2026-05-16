#include "communicator_v2.h"
#include "devices/garmin/garmindevice.h"
#include "garmintypes.h"
#include "cobscodec.h"      // CobsCoDec
#include "garminmlr.h"   // MlrCommunicator (Qt)
#include "garmintypes.h"
#include "garminmessages.h"


#include <QtCore/QStringList>
#include <QtCore/QMetaObject>
#include <QDomDocument>
#include <QByteArray>
#include <QBuffer>
#include <QDataStream>
#include <QObject>

const char* CommunicatorV2::BASE_UUID                   = "6a4e%1-667b-11e3-949a-0800200c9a66";
const char* CommunicatorV2::UUID_SERVICE_GARMIN_ML_GFDI = "6a4e2800-667b-11e3-949a-0800200c9a66";

static inline QString fmtUuid(quint16 shortId) {
    return QStringLiteral("6A4E%1-667B-11E3-949A-0800200C9A66")
        .arg(shortId, 4, 16, QLatin1Char('0'))
        .toUpper();
}

static inline void appendLe16(QByteArray& out, quint16 v) {
    out.append(char(v & 0xFF));
    out.append(char((v >> 8) & 0xFF));
}
static inline void appendLe64(QByteArray& out, quint64 v) {
    for (int i=0;i<8;++i) out.append(char((v >> (8*i)) & 0xFF));
}
static inline quint16 readLe16(const QByteArray& b, int off) {
    if (off + 1 >= b.size()) return 0;
    return quint16(quint8(b[off]) | (quint16(quint8(b[off+1])) << 8));
}
static inline quint32 readLe32(const QByteArray& b, int off) {
    if (off + 3 >= b.size()) return 0;
    return quint32(quint8(b[off]) |
                   (quint32(quint8(b[off+1])) << 8) |
                   (quint32(quint8(b[off+2])) << 16) |
                   (quint32(quint8(b[off+3])) << 24));
}
static inline QString hexDump(const QByteArray& b, int max=32) {
    const int n = qMin(max, b.size());
    QStringList parts;
    parts.reserve(n);
    for (int i=0;i<n;++i) {
        parts << QStringLiteral("%1").arg(quint8(b[i]), 2, 16, QLatin1Char('0')).toUpper();
    }
    return parts.join(QStringLiteral(" "));
}



// =============================================================================

// =============================================================================
// CommunicatorV2
// =============================================================================

QString CommunicatorV2::baseUuid(quint16 shortId) { return fmtUuid(shortId); }

CommunicatorV2::CommunicatorV2(const QString &path, QObject* parent)
    : QBLEService(UUID_SERVICE_GARMIN_ML_GFDI, path, parent), m_state(CommunicatorState::create()), m_Path(path), m_device(parent)
{
    qDebug() << "Garmin Service created for " << path;
    connect(this, &QBLEService::characteristicRead, this, &CommunicatorV2::characteristicRead);
    QPointer<GfdiMessageCallback> m_syncCb(new GfdiMessageCallback());
    //m_asyncCb = QSharedPointer<AsyncGfdiMessageCallback>::create(new AsyncGfdiMessageCallback());
    initializeDevice();

}

void CommunicatorV2::setMessageCallback(QSharedPointer<GfdiMessageCallback> cb) {
    //QMutexLocker lock(&m_mutex);
    m_syncCb = std::move(cb);
}

void CommunicatorV2::setAsyncMessageCallback(QPointer<AsyncGfdiMessageCallback> cb) {
    //QMutexLocker lock(&m_mutex);
    m_asyncCb = cb;
}

void CommunicatorV2::registerServiceCallback(Service service, QSharedPointer<ServiceCallback> cb) {
    //QMutexLocker lock(&m_mutex);
    m_state->serviceCallbacks.insert(service, std::move(cb));
}

QSharedPointer<ServiceCallback> CommunicatorV2::unregisterServiceCallback(Service service) {
    //QMutexLocker lock(&m_mutex);
    return m_state->serviceCallbacks.take(service);
}

std::optional<QString> CommunicatorV2::getReceiveCharacteristicUuid() const {
    //QMutexLocker lock(&m_mutex);
    if (!m_state->characteristicReceive) return std::nullopt;
    return m_state->characteristicReceive->value();
}

void CommunicatorV2::onMtuChanged(int mtu) {
    //QMutexLocker lock(&m_mutex);
    m_state->maxWriteSize = qMax(0, mtu - 3); // Rust: saturating_sub(3)
    for (auto it = m_state->mlrCommunicators.begin(); it != m_state->mlrCommunicators.end(); ++it) {
        it.value()->setMaxPacketSize(m_state->maxWriteSize);
    }
}

void CommunicatorV2::onDeviceMaxPacketSize(quint16 deviceMaxPacketSize) {
    //QMutexLocker lock(&m_mutex);
    const int dev = int(deviceMaxPacketSize);
    if (dev < m_state->maxWriteSize) {
        m_state->maxWriteSize = dev;
        for (auto it = m_state->mlrCommunicators.begin(); it != m_state->mlrCommunicators.end(); ++it) {
            it.value()->setMaxPacketSize(m_state->maxWriteSize);
        }
    }
}

Result<void> CommunicatorV2::awaitBleWrite(const QBLECharacteristic& handle,
                                          const QByteArray& bytes,
                                          const QString& taskName) {
    qDebug() << "Garmin: Communicator awaiting write for task " <<taskName << "data " <<bytes;
    Result<void> outcome = Result<void>::isOk();
    /*
    QEventLoop loop;


    QMetaObject::Connection c1 = QObject::connect(
        &handle, &QBLECharacteristic::characteristicWritten,
        &loop, [&](const QString& tn){
            qDebug() << "Garmin: Communicator awaiting wrote successfull, taskname " << taskName << " returned tn " << tn;
            if (tn == taskName) loop.quit();
        });

    QMetaObject::Connection c2 = QObject::connect(
        &handle, &QBLECharacteristic::characteristicWriteFailed,
        &loop, [&](const QString& tn, const QString& err){

            if (tn == taskName) {
                qDebug() << "Garmin: Communicator awaiting wrote failed, taskname " << taskName << " returned tn " << tn;
                outcome = Result<void>::err(GarminError::bluetoothError(err));
                loop.quit();
            }
    });

    handle.writeAsync(bytes,&ErrorMsg);
    //m_ble->writeCharacteristic(handle, bytes, taskName);
    loop.exec();

    QObject::disconnect(c1);
    QObject::disconnect(c2);
    */
    QString ErrorMsg;
    handle.writeAsync(bytes);

    qDebug() << "Garmin: Comminicator awaiting write finished for " << taskName << "Error Message " << ErrorMsg;
    return outcome;
}

quint64 CommunicatorV2::nextCookie() {
    //QMutexLocker lock(&m_mutex);
    return m_cookieCounter++;
}

Result<std::optional<QByteArray>> CommunicatorV2::awaitAsyncCallback(const QByteArray& message) {
    QPointer<AsyncGfdiMessageCallback> cb;
    quint64 cookie = 0;
    {
        //QMutexLocker lock(&m_mutex);
        cb = m_asyncCb;
        cookie = nextCookie();
    }
    if (!cb) return Result<std::optional<QByteArray>>::isOk(std::nullopt);

    QEventLoop loop;
    Result<std::optional<QByteArray>> outcome = Result<std::optional<QByteArray>>::isOk(std::nullopt);

    QMetaObject::Connection c1 = QObject::connect(
        cb, &AsyncGfdiMessageCallback::replyReady,
        &loop, [&](quint64 c, const QByteArray& reply){

            if (c == cookie) { outcome = Result<std::optional<QByteArray>>::isOk(reply); loop.quit(); }
        });

    QMetaObject::Connection c2 = QObject::connect(
        cb, &AsyncGfdiMessageCallback::noReply,
        &loop, [&](quint64 c){
            if (c == cookie) { outcome = Result<std::optional<QByteArray>>::isOk(std::nullopt); loop.quit(); }
        });

    QMetaObject::Connection c3 = QObject::connect(
        cb, &AsyncGfdiMessageCallback::failed,
        &loop, [&](quint64 c, const QString& err){

            if (c == cookie) { outcome = Result<std::optional<QByteArray>>::err(GarminError::invalidMessage(err)); loop.quit(); }
        });

    QMetaObject::invokeMethod(cb, "onMessage", Qt::QueuedConnection,
                              Q_ARG(QByteArray, message),
                              Q_ARG(quint64, cookie));

    loop.exec();

    QObject::disconnect(c1);
    QObject::disconnect(c2);
    QObject::disconnect(c3);
    return outcome;
}

Result<bool> CommunicatorV2::initializeDevice() {
    QMutexLocker lock(&m_mutex);
    QMap<QString, QBLECharacteristic*> characteristicMap;


    QDBusInterface miIntro("org.bluez", m_Path, "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus(), 0);
    QDBusReply<QString> xml = miIntro.call("Introspect");
    QDomDocument doc;

    doc.setContent(xml.value());

    QDomNodeList nodes = doc.elementsByTagName("node");

    for (int x = 0; x < nodes.count(); x++)
    {
        QDomElement node = nodes.at(x).toElement();
        QString nodeName = node.attribute("name");

        if (nodeName.startsWith("char")) {
            QString path = m_Path + "/" + nodeName;
            QDBusInterface charInterface("org.bluez", path, "org.bluez.GattCharacteristic1", QDBusConnection::systemBus(), 0);
            characteristicMap[charInterface.property("UUID").toString()] = new QBLECharacteristic(path, this);
        }
    }

    qDebug() << "Garmin characteristics" << characteristicMap.keys();

    for (int i = 2810; i <= 2814; i++) {
        QString serviceRec = QString(CommunicatorV2::BASE_UUID).arg(i);
        QString serviceSnd = QString(CommunicatorV2::BASE_UUID).arg(i+10);
        qDebug() << "Checking for Garmin    UUID " << serviceRec <<" and " << serviceSnd;

        if (characteristicMap.contains(serviceRec) && characteristicMap.contains(serviceSnd))
        {
            m_state->characteristicReceive = characteristicMap.value(serviceRec);
            m_state->receivePath = serviceRec;
            m_state->characteristicSend = characteristicMap.value(serviceSnd);
            m_state->sendPath = serviceSnd;
            qDebug() << " Garmin ML Characteristic found";
        }


        if ((m_state->characteristicSend != NULL) && (m_state->characteristicReceive  != NULL)) {
            lock.unlock();
            enableNotification(serviceRec);
            connect(this,&QBLEService::characteristicChanged,this,&CommunicatorV2::onCharacteristicChanged);
            /* TODO
            auto en = m_ble->enableNotifications(m_state->characteristicReceive );
            if (!en.ok) return Result<bool>::err(en.error);
    */

            const QByteArray closeAll = createCloseAllServicesMessage();
            auto wr = awaitBleWrite(*m_state->characteristicSend, closeAll, QStringLiteral("close_all"));
            if (!wr.ok) return Result<bool>::err(wr.error);

/*
            auto msg = GfdiMessageGenerator::protobufBatteryStatusRequest(0);
            if (!msg.ok) return Result<bool>::isOk(false);
            qDebug() << "Garmin: Request Battery Updatesy";
            wr = sendMessage(msg.value,"Battery Updates");
            if (!wr.ok) return Result<bool>::err(wr.error);
*/

            return Result<bool>::isOk(true);        }
    }

    qDebug() << "Failed to find any known Garmin ML characteristics";


     return Result<bool>::isOk(false);
}

/*
Result<bool> CommunicatorV2::initializeDeviceWithTransaction(const QString& transactionName) {
    //QMutexLocker lock(&m_mutex);

    auto tx = m_ble->createTransaction(transactionName);
    if (!tx) return Result<bool>::err(GarminError::bluetoothError(QStringLiteral("Transaction not available")));

    for (quint16 i = 0x2810; i <= 0x2814; ++i) {
        const QString recvUuid = fmtUuid(i);
        const QString sendUuid = fmtUuid(i + 0x10);

        auto recv = m_ble->getCharacteristic(recvUuid);
        auto send = m_ble->getCharacteristic(sendUuid);

        if (recv && send) {
            m_state->characteristicReceive = recv;
            m_state->characteristicSend = send;

            tx->notify(*recv, true);
            tx->write(*send, createCloseAllServicesMessage());

            lock.unlock();
            auto q = tx->queue();
            if (!q.ok) return Result<bool>::err(q.error);
            return Result<bool>::isOk(true);
        }
    }

    return Result<bool>::isOk(false);
}
*/
Result<void> CommunicatorV2::sendMessage(const QString& taskName, const QByteArray& message) {
    qDebug() << "Garmin: Send Message " << taskName << " Content " << message;
    if (message.isEmpty()) return Result<void>::err(GarminError::emptyMessage());

    QSharedPointer<MlrCommunicator> mlr;
    QBLECharacteristic *sendChar;
    quint8 gfdiHandle = 0;
    int maxWriteSize = 20;

    QMutexLocker lock(&m_mutex);
    if (!m_state->handleByService.contains(Service::GFDI)) {
        qDebug() << "Garmin: No GFDI handle found";
        return Result<void>::err(GarminError::gfdiHandleNotSet());
    }
    gfdiHandle = m_state->handleByService.value(Service::GFDI);
    maxWriteSize = m_state->maxWriteSize;
    if (!m_state->characteristicSend) {
        qDebug() << "Garmin: No send characteristic found.";
        return Result<void>::err(GarminError::bluetoothError(QStringLiteral("Send characteristic not set")));
    }
    sendChar = m_state->characteristicSend;
    const quint8 mlrHandle = (gfdiHandle & 0x0F);
    mlr = m_state->mlrCommunicators.value(mlrHandle);

    const QByteArray payload = CobsCoDec::encode(message);

    if (mlr) {
        qDebug() << "Garmin: Sending message via MLR";
        return mlr->sendMessage(taskName, payload);
    }

    // No MLR => fragment and write directly (same as Rust)
    qDebug() << "Garmin: No Mlr found, sending directly";
    int remaining = payload.size();
    int position = 0;

    if (remaining > maxWriteSize - 1) {
        while (remaining > 0) {
            const int chunk = qMin(remaining, maxWriteSize - 1);
            QByteArray fragment;
            fragment.reserve(chunk + 1);
            fragment.append(char(gfdiHandle));
            fragment.append(payload.mid(position, chunk));

            auto wr = awaitBleWrite(*sendChar, fragment, taskName);
            if (!wr.ok) return wr;

            position += chunk;
            remaining -= chunk;
        }
    } else {
        QByteArray packet;
        packet.reserve(payload.size() + 1);
        packet.append(char(gfdiHandle));
        packet.append(payload);

        auto wr = awaitBleWrite(*sendChar, packet, taskName);
        if (!wr.ok) return wr;
    }


    return Result<void>::isOk();
}

Result<void> CommunicatorV2::sendMessageWithTransaction(const QString& taskName, const QByteArray& message) {
    if (message.isEmpty()) return Result<void>::err(GarminError::emptyMessage());
/*
    QSharedPointer<Transaction> tx = m_ble->createTransaction(taskName);
    if (!tx) return Result<void>::err(GarminError::bluetoothError(QStringLiteral("Transaction not available")));
*/
    quint8 gfdiHandle = 0;
    int maxWriteSize = 20;

    {
        //QMutexLocker lock(&m_mutex);
        if (!m_state->handleByService.contains(Service::GFDI)) {
            return Result<void>::err(GarminError::gfdiHandleNotSet());
        }
        gfdiHandle = m_state->handleByService.value(Service::GFDI);
        maxWriteSize = m_state->maxWriteSize;
        if (!m_state->characteristicSend) {
            return Result<void>::err(GarminError::bluetoothError(QStringLiteral("Send characteristic not set")));
        }
    }

    const QByteArray payload = CobsCoDec::encode(message);

    int remaining = payload.size();
    int position = 0;
    if (remaining > maxWriteSize - 1) {
        while (remaining > 0) {
            const int chunk = qMin(remaining, maxWriteSize - 1);
            QByteArray fragment;
            fragment.reserve(chunk + 1);
            fragment.append(char(gfdiHandle));
            fragment.append(payload.mid(position, chunk));
            sendMessageWithTransaction(taskName,fragment);
            //tx->write(*sendChar, fragment);
            position += chunk;
            remaining -= chunk;
        }
    } else {
        QByteArray packet;
        packet.reserve(payload.size() + 1);
        packet.append(char(gfdiHandle));
        packet.append(payload);
        sendMessageWithTransaction(taskName,packet);
        //tx->write(*sendChar, packet);
    }

    return Result<void>::isOk();

    //return tx->queue();
}

void CommunicatorV2::onCharacteristicChanged(const QString &characteristic, const QByteArray& data) {
    if (data.isEmpty()) return ;

    QSharedPointer<MlrCommunicator> mlr;
    {
        //QMutexLocker lock(&m_mutex);
        // MLR packet detection: bit7 set
        if (data.size() >= 2 && (quint8(data[0]) & 0x80) != 0) {
            const quint8 handle = (quint8(data[0]) & 0x70) >> 4;
            mlr = m_state->mlrCommunicators.value(handle);
        }
    }

    if (mlr) {
        mlr->onPacketReceived(data);
        return;
    }

    // handle 0 => management
    const quint8 handle = quint8(data[0]);
    const QByteArray payload = data.mid(1);
    if (handle == 0) {
        processHandleManagement(payload);
        return;
    }

    // Non-MLR COBS processing (faithful: Rust passes full data into codec)
    {
        //QMutexLocker lock(&m_mutex);
        m_state->cobsCodec.receiveBytes(data);
        auto decoded = m_state->cobsCodec.retrieveMessage();
        if (decoded.has_value()) {
            //lock.unlock();
            handleDecodedMessage(*decoded);
            return;
        }
    }
    /*
    if (isFirstConncet) {

        auto msg = GfdiMessageGenerator::systemEvent(8,0);
        //if (!msg.ok) return Result<bool>::isOk(false);
        qDebug() << "Garmin: Sync Ready";
        auto wr = sendMessage(msg.value,"SYNC_READY");
        //if (!wr.ok) return Result<bool>::err(wr.error);
        msg = GfdiMessageGenerator::systemEvent(4,0);
        //if (!msg.ok) return Result<bool>::isOk(false);
        qDebug() << "Garmin: Sending Pair Complete";
        wr = sendMessage(msg.value,"PAIR_COMPLETE");
        //if (!wr.ok) return Result<bool>::err(wr.error);
        msg = GfdiMessageGenerator::systemEvent(0,0);
        //if (!msg.ok) return Result<bool>::isOk(false);
        qDebug() << "Garmin: Sending Sync Complete";
        wr = sendMessage(msg.value,"SYNC_COMPLETE");
        //if (!wr.ok) return Result<bool>::err(wr.error);
        msg = GfdiMessageGenerator::systemEvent(14,0);
        //if (!msg.ok) return Result<bool>::isOk(false);
        qDebug() << "Garmin: Sending SetupWizard Complete";
        wr = sendMessage(msg.value,"SETUP_WIZARD_COMPLETE,");
        //if (!wr.ok) return Result<bool>::err(wr.error);
        if (!wr.ok) {
            isFirstConncet=true;
        }
    }
    */

    return;
}

Result<void> CommunicatorV2::handleDecodedMessage(const QByteArray& decodedWithHandle) {
    if (decodedWithHandle.isEmpty()) return Result<void>::isOk();

    const quint8 handle = quint8(decodedWithHandle[0]);
    const QByteArray payload = decodedWithHandle.mid(1);

    Service service;
    QSharedPointer<ServiceCallback> svcCb;
    QSharedPointer<GfdiMessageCallback> syncCb;
    QPointer<AsyncGfdiMessageCallback> asyncCb;

    {
        //QMutexLocker lock(&m_mutex);
        service = m_state->serviceByHandle.value(handle, Service::GFDI);
        svcCb = m_state->serviceCallbacks.value(service);
        syncCb = m_syncCb;
        asyncCb = m_asyncCb;
    }

    if (svcCb) {
        auto r = svcCb->onMessage(payload);
        if (!r.ok) return r;
    }

    if (service == Service::GFDI) {
        if (syncCb) {
            return syncCb->onMessage(payload);
        }
        if (asyncCb) {
            auto r = awaitAsyncCallback(payload);
            if (!r.ok) return Result<void>::err(r.error);
        }
        emit gfdiMessageReceived(payload);
    }

    return Result<void>::isOk();
}

Result<std::optional<QByteArray>> CommunicatorV2::handleDecodedMessageAsync(const QByteArray& decodedWithHandle) {
    if (decodedWithHandle.isEmpty()) return Result<std::optional<QByteArray>>::isOk(std::nullopt);
    const quint8 handle = quint8(decodedWithHandle[0]);
    const QByteArray payload = decodedWithHandle.mid(1);

    Service service;
    {
        //QMutexLocker lock(&m_mutex);
        service = m_state->serviceByHandle.value(handle, Service::GFDI);
    }

    if (service == Service::GFDI) {
        return awaitAsyncCallback(payload);
    }
    return Result<std::optional<QByteArray>>::isOk(std::nullopt);
}

Result<void> CommunicatorV2::processHandleManagement(const QByteArray& message) {
    qDebug() << "Garmin: processHandlemanagement called";
    if (message.isEmpty()) {
        return Result<void>::err(GarminError::invalidMessage(QStringLiteral("Empty handle management message")));
    }

    auto rt = requestTypeFromU8(quint8(message[0]));
    if (!rt.ok) return Result<void>::err(rt.error);

    if (message.size() < 9) {
        return Result<void>::err(GarminError::invalidMessage(QStringLiteral("Handle management message too short")));
    }

    const quint64 clientId = readLe16(message, 1) | (quint64(readLe16(message, 3)) << 16) |
                             (quint64(readLe16(message, 5)) << 32) | (quint64(readLe16(message, 7)) << 48);

    if (clientId != GADGETBRIDGE_CLIENT_ID) {
        return Result<void>::isOk();
    }

    const QByteArray payload = message.mid(9);

    switch (rt.value) {
    case RequestType::RegisterMlResp:
        qDebug() << "Garmin: handle RegisterMlResp";
        return processRegisterMlResp(payload);
    case RequestType::CloseHandleResp:
        qDebug() << "Garmin: handle CloseHendleResp";
        return processCloseHandleResp(payload);
    case RequestType::CloseAllResp:
        qDebug() << "Garmin: handle CloseAllResp";
        return processCloseAllResp();
    default:
        return Result<void>::isOk();
    }
}

Result<void> CommunicatorV2::processRegisterMlResp(const QByteArray& payload) {
    qDebug() << "Garmin: processing RegisterMlResp";
    if (payload.size() < 5) {
        qDebug() << "Garmin: MLR Registration Responsepayload too short: " << payload.size();
        return Result<void>::err(GarminError::invalidMessage(QStringLiteral("RegisterMlResp payload too short")));
    }

    const quint16 serviceCodeLE = readLe16(payload, 0);
    const quint8 status = quint8(payload[2]);
    const quint8 handle = quint8(payload[3]);
    const quint8 reliable = quint8(payload[4]);

    auto svc = serviceFromCode(serviceCodeLE);
    if (!svc.ok) {
        qDebug() << "Garmin: Got register response status " << status << " for unknown service " << serviceCodeLE;
        return Result<void>::isOk();
    }
    if (status != 0)  {
        qDebug() << "Failed to register" << serviceCodeLE << ", status = " << status;
        return Result<void>::isOk();
    }

    const Service service = svc.value;
    const quint8 mlrHandle = (reliable != 0) ? (handle & 0x0F) : handle;

    QSharedPointer<MlrCommunicator> createdMlr;

    //QMutexLocker lock(&m_mutex);
    qDebug() << "Garmin: Inserting Handle " << handle;
    m_state->serviceByHandle.insert(handle, service);
    m_state->handleByService.insert(service, handle);

    // Create MLR communicator if reliable
    if (reliable != 0 && !m_state->mlrCommunicators.contains(mlrHandle)) {
        if (!m_state->characteristicSend) {
            qDebug() << "Garmin: No send characteristic found!";
            return Result<void>::isOk();
        }

        qDebug() << "Garmin: creating reliable MLR communicator";
        auto sender = QSharedPointer<MlrMessageSender>::create(m_state->characteristicSend, this);
        auto receiver = QSharedPointer<MlrMessageReceiver>::create(m_syncCb, m_asyncCb, this);
        auto mlr = QSharedPointer<MlrCommunicator>::create(mlrHandle, 20, sender, receiver);
        mlr->start();
        qDebug() << "Garmin: Reliable MLR communicator created";
        m_state->mlrCommunicators.insert(mlrHandle, mlr);
        QObject::connect(receiver.data(), &MlrMessageReceiver::gfdiDecoded,
            this, &CommunicatorV2::gfdiMessageReceived);
        createdMlr = mlr;
     }

    // Default callback creation for GFDI (faithful)
    {
        QMutexLocker lock(&m_mutex);

        if (service == Service::GFDI && !m_state->serviceCallbacks.contains(Service::GFDI) && m_syncCb) {
            qDebug() << "Garmin: Creating GFDI callback handle";
            m_state->serviceCallbacks.insert(Service::GFDI, QSharedPointer<ServiceCallback>(new GfdiServiceCallback(m_syncCb)));
        }
    }
    // Call onConnect if callback exists
    QSharedPointer<ServiceCallback> cb;
    QBLECharacteristic *sendChar = NULL;
    {
       //QMutexLocker lock(&m_mutex);
       cb = m_state->serviceCallbacks.value(service);
       if (m_state->characteristicSend) sendChar = m_state->characteristicSend;
       if (cb && sendChar) {
           QSharedPointer<ServiceWriter> writer(new MlrServiceWriter(mlrHandle, m_ble, m_state->sendPath));
           (void)cb->onConnect(writer);
       }
    }

    qDebug() << "Garmin: Checking for first connection";
    if (isFirstConncet) {

        auto msg = GfdiMessageGenerator::systemEvent(8,0);
        //if (!msg.ok) return Result<bool>::isOk(false);
        qDebug() << "Garmin: Sync Ready";
        auto wr = sendMessage("SYNC_READY",msg.value);
        if (!wr.ok) qDebug() << "Garmin: Sync Ready failed";
        //if (!wr.ok) return Result<bool>::err(wr.error);
        msg = GfdiMessageGenerator::systemEvent(4,0);
        //if (!msg.ok) return Result<bool>::isOk(false);
        qDebug() << "Garmin: Sending Pair Complete";
        wr = sendMessage("PAIR_COMPLETE",msg.value);
        if (!wr.ok) qDebug() << "Garmin: Pair Complet failed";
        //if (!wr.ok) return Result<bool>::err(wr.error);
        msg = GfdiMessageGenerator::systemEvent(0,0);
        //if (!msg.ok) return Result<bool>::isOk(false);
        qDebug() << "Garmin: Sending Sync Complete";
        wr = sendMessage("SYNC_COMPLETE",msg.value);
        if (!wr.ok) qDebug() << "Garmin: Sync Complete failed";
        //if (!wr.ok) return Result<bool>::err(wr.error);
        msg = GfdiMessageGenerator::systemEvent(14,0);
        //if (!msg.ok) return Result<bool>::isOk(false);
        qDebug() << "Garmin: Sending SetupWizard Complete";
        wr = sendMessage("SETUP_WIZARD_COMPLETE",msg.value);
        if (!wr.ok) qDebug() << "Garmin: Setup Wizard complete failed";
        //if (!wr.ok) return Result<bool>::err(wr.error);
        if (!wr.ok) {
            isFirstConncet=true;
        }
    }
    return Result<void>::isOk();
}

Result<void> CommunicatorV2::processCloseHandleResp(const QByteArray& payload) {
    if (payload.size() < 4) {
        return Result<void>::err(GarminError::invalidMessage(QStringLiteral("CloseHandleResp payload too short")));
    }

    const quint16 serviceCodeLE = readLe16(payload, 0);
    const quint8 handle = quint8(payload[2]);

    auto svc = serviceFromCode(serviceCodeLE);
    if (!svc.ok) return Result<void>::isOk();
    const Service service = svc.value;

    QSharedPointer<ServiceCallback> cb;
    {
        QMutexLocker lock(&m_mutex);
        cb = m_state->serviceCallbacks.take(service);
        m_state->handleByService.remove(service);
        m_state->serviceByHandle.remove(handle);
        m_state->mlrCommunicators.remove(handle);
    }
    if (cb) (void)cb->onClose();
    return Result<void>::isOk();
}

Result<void> CommunicatorV2::processCloseAllResp() {
    QBLECharacteristic *sendChar=NULL;
    QList<QSharedPointer<ServiceCallback>> callbacks;

    {
        QMutexLocker lock(&m_mutex);
        m_state->serviceByHandle.clear();
        m_state->handleByService.clear();

        for (auto it = m_state->serviceCallbacks.begin(); it != m_state->serviceCallbacks.end(); ++it) {
            callbacks.push_back(it.value());
        }
        m_state->serviceCallbacks.clear();
        m_state->mlrCommunicators.clear();

        sendChar = m_state->characteristicSend;
    }

    for (auto& cb : callbacks) {
        if (cb) (void)cb->onClose();
    }

    if (sendChar) {
        const QByteArray reg = createRegisterServiceMessage(Service::GFDI, true);
        (void)awaitBleWrite(*sendChar, reg, QStringLiteral("register_gfdi"));
    }



    return Result<void>::isOk();
}

Result<void> CommunicatorV2::registerService(Service service, bool reliable) {
    const QByteArray msg = createRegisterServiceMessage(service, reliable);
    return sendMessage(QStringLiteral("register_%1").arg(serviceToString(service)), msg);
}

Result<void> CommunicatorV2::closeService(Service service) {
    quint8 handle = 0;
    {
        //QMutexLocker lock(&m_mutex);
        if (!m_state->handleByService.contains(service)) {
            return Result<void>::err(GarminError::bluetoothError(QStringLiteral("Service not registered")));
        }
        handle = m_state->handleByService.value(service);
    }
    const QByteArray msg = createCloseServiceMessage(service, handle);
    return sendMessage(QStringLiteral("close_%1").arg(serviceToString(service)), msg);
}

void CommunicatorV2::registerHandle(Service service, quint8 handle) {
    //QMutexLocker lock(&m_mutex);
    m_state->serviceByHandle.insert(handle, service);
    m_state->handleByService.insert(service, handle);
}

Result<void> CommunicatorV2::dispose() {
    //QMutexLocker lock(&m_mutex);
    m_state->mlrCommunicators.clear();
    return Result<void>::isOk();
}

void CommunicatorV2::onConnectionStateChange(bool connected) {
    if (!connected) clearAndPauseMlr();
}

void CommunicatorV2::pauseMlr() {
    //QMutexLocker lock(&m_mutex);
    for (auto it = m_state->mlrCommunicators.begin(); it != m_state->mlrCommunicators.end(); ++it) {
        it.value()->pause();
    }
}

void CommunicatorV2::resumeMlr() {
    //QMutexLocker lock(&m_mutex);
    for (auto it = m_state->mlrCommunicators.begin(); it != m_state->mlrCommunicators.end(); ++it) {
        it.value()->resume();
    }
}

void CommunicatorV2::clearAndPauseMlr() {
    //QMutexLocker lock(&m_mutex);
    for (auto it = m_state->mlrCommunicators.begin(); it != m_state->mlrCommunicators.end(); ++it) {
        it.value()->clearAndPause();
    }
    m_state->mlrCommunicators.clear();
}

QByteArray CommunicatorV2::createCloseAllServicesMessage() const {
    QByteArray b;
    b.reserve(13);
    b.append(char(0));
    b.append(char(quint8(RequestType::CloseAllReq)));
    appendLe64(b, GADGETBRIDGE_CLIENT_ID);
    appendLe16(b, 0);
    b.append(char(0));
    return b;
}

QByteArray CommunicatorV2::createRegisterServiceMessage(Service service, bool reliable) const {
    QByteArray b;
    b.reserve(13);
    b.append(char(0));
    b.append(char(quint8(RequestType::RegisterMlReq)));
    appendLe64(b, GADGETBRIDGE_CLIENT_ID);
    appendLe16(b, serviceCode(service));
    b.append(char(reliable ? 2 : 0));
    return b;
}

QByteArray CommunicatorV2::createCloseServiceMessage(Service service, quint8 handle) const {
    QByteArray b;
    b.reserve(12);
    b.append(char(0));
    b.append(char(quint8(RequestType::CloseHandleReq)));
    appendLe64(b, GADGETBRIDGE_CLIENT_ID);
    appendLe16(b, serviceCode(service));
    b.append(char(handle));
    return b;
}

void CommunicatorV2::characteristicRead(const QString &c, const QByteArray &value)
{

        qDebug() << Q_FUNC_INFO << c << "=================" << value;
        qDebug() << "Garmin: Charateristics Read called";

}


// =============================================================================
// MlrServiceWriter
// =============================================================================

MlrServiceWriter::MlrServiceWriter(quint8 handle,
                                   QSharedPointer<QBLEService> ble,
                                   QString sendPath)
    : m_handle(handle),m_ble(std::move(ble)), m_sendChar(sendPath)
{}

Result<void> MlrServiceWriter::awaitBleWrite(const QString& taskName, const QByteArray& bytes) {
    qDebug() <<"Garmin: MLR Servicwriter awaitWrite called";
    Result<void> outcome = Result<void>::isOk();
/*
    QEventLoop loop;


    QMetaObject::Connection c1 = QObject::connect(
        &m_sendChar, &QBLECharacteristic::characteristicWritten,
        &loop, [&](const QString& tn){
            qDebug() << "Garmin: ServiceWRiter wrote successfull, taskname " << taskName << " returned tn " << tn;
            if (tn == taskName) loop.quit();
        });

    QMetaObject::Connection c2 = QObject::connect(
        &m_sendChar, &QBLECharacteristic::characteristicWriteFailed,
        &loop, [&](const QString& tn, const QString& err){

            if (tn == taskName) {
                qDebug() << "Garmin: ServiceWRiter wrote failed, taskname " << taskName << " returned tn " << tn;
                outcome = Result<void>::err(GarminError::bluetoothError(err));
                loop.quit();
            }
        });

   m_sendChar.writeValue(bytes);
    //m_ble->writeCharacteristic(m_sendChar, bytes, taskName);

    loop.exec();

    QObject::disconnect(c1);
    QObject::disconnect(c2);
*/
    QString errorMsg;
    m_sendChar.writeValue(bytes,&errorMsg);
    qDebug() << "Garmin: Mlr Service writer wrote, error returned: " << errorMsg;
    return outcome;
}

Result<void> MlrServiceWriter::write(const QString& taskName, const QByteArray& data) {
    QByteArray payload;
    payload.reserve(data.size() + 1);
    payload.append(char(m_handle));
    payload.append(data);
    return awaitBleWrite(taskName, payload);
}

// =============================================================================
// Example callbacks
// =============================================================================


GfdiServiceCallback::GfdiServiceCallback(QSharedPointer<GfdiMessageCallback> cb)
    : m_cb(std::move(cb))
{
    qDebug() << "Garmin: GFDI Callback called";
}

Result<void> GfdiServiceCallback::onConnect(QSharedPointer<ServiceWriter> writer) {
    Q_UNUSED(writer);
    return Result<void>::isOk();
}
Result<void> GfdiServiceCallback::onClose() {
    return Result<void>::isOk();
}
Result<void> GfdiServiceCallback::onMessage(const QByteArray& data) {
    qDebug() << "Garmin: GfdiServicecallback called";
    m_codec.receiveBytes(data);
    auto decoded = m_codec.retrieveMessage();
    if (decoded.has_value() && m_cb) {
        return m_cb->onMessage(*decoded);
    }
    return Result<void>::isOk();
}
/*
RealtimeHeartRateCallback::RealtimeHeartRateCallback(Handler h) : m_handler(std::move(h)) {}
Result<void> RealtimeHeartRateCallback::onConnect(QSharedPointer<ServiceWriter> writer) { Q_UNUSED(writer); return Result<void>::isOk(); }
Result<void> RealtimeHeartRateCallback::onClose() { return Result<void>::isOk(); }
Result<void> RealtimeHeartRateCallback::onMessage(const QByteArray& data) {
    if (data.size() >= 1 && m_handler) return m_handler(quint8(data[0]));
    return Result<void>::isOk();
}

RealtimeStepsCallback::RealtimeStepsCallback(Handler h) : m_handler(std::move(h)) {}
Result<void> RealtimeStepsCallback::onConnect(QSharedPointer<ServiceWriter> writer) { Q_UNUSED(writer); return Result<void>::isOk(); }
Result<void> RealtimeStepsCallback::onClose() { return Result<void>::isOk(); }
Result<void> RealtimeStepsCallback::onMessage(const QByteArray& data) {
    if (data.size() >= 4 && m_handler) return m_handler(readLe32(data, 0));
    return Result<void>::isOk();
}

FileTransferCallback::FileTransferCallback(Handler onComplete) : m_onComplete(std::move(onComplete)) {}
Result<void> FileTransferCallback::onConnect(QSharedPointer<ServiceWriter> writer) { Q_UNUSED(writer); return Result<void>::isOk(); }
Result<void> FileTransferCallback::onClose() {
    if (!m_data.isEmpty() && m_onComplete) return m_onComplete(m_data);
    return Result<void>::isOk();
}
Result<void> FileTransferCallback::onMessage(const QByteArray& data) {
    m_data.append(data);
    return Result<void>::isOk();
}

*/
