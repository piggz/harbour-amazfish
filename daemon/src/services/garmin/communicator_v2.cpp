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
static inline quint64 readLe64(const QByteArray& b, int off) {
    if (off + 7 >= b.size()) return 0;
    return quint64(quint8(b[off]) |
             (quint64(quint8(b[off+1])) << 8) |
             (quint64(quint8(b[off+2])) << 16) |
             (quint64(quint8(b[off+3])) << 24) |
             (quint64(quint8(b[off+4])) << 32) |
             (quint64(quint8(b[off+5])) << 40) |
             (quint64(quint8(b[off+6])) << 48) |
             (quint64(quint8(b[off+7])) << 56));

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
    : QBLEService(UUID_SERVICE_GARMIN_ML_GFDI, path, parent), mState(CommunicatorState::create()), m_Path(path), m_device(parent)
{
    qDebug() << "Garmin: Service created for " << path;
    connect(this, &QBLEService::characteristicRead, this, &CommunicatorV2::characteristicRead);
    mMessageCallback =QSharedPointer<GfdiMessageCallback>::create();
    //mAsyncMessageCallback = QSharedPointer<AsyncGfdiMessageCallback>::create(new AsyncGfdiMessageCallback());
    connect(this,&CommunicatorV2::pairingComplete,this,&CommunicatorV2::registerServices);
    initializeDevice();

}

void CommunicatorV2::setMessageCallback(QSharedPointer<GfdiMessageCallback> cb) {
    //QMutexLocker lock(&m_mutex);
    mMessageCallback = std::move(cb);
}

void CommunicatorV2::setAsyncMessageCallback(QPointer<AsyncGfdiMessageCallback> cb) {
    //QMutexLocker lock(&m_mutex);
    mAsyncMessageCallback = cb;
}

void CommunicatorV2::registerServiceCallback(Service service, QSharedPointer<ServiceCallback> cb) {
    //QMutexLocker lock(&m_mutex);
    mState->serviceCallbacks.insert(service, std::move(cb));
}

QSharedPointer<ServiceCallback> CommunicatorV2::unregisterServiceCallback(Service service) {
    //QMutexLocker lock(&m_mutex);
    return mState->serviceCallbacks.take(service);
}

std::optional<QString> CommunicatorV2::getReceiveCharacteristicUuid() const {
    //QMutexLocker lock(&m_mutex);
    if (!mState->characteristicReceive) return std::nullopt;
    return mState->characteristicReceive->value();
}

void CommunicatorV2::onMtuChanged(int mtu) {
    //QMutexLocker lock(&m_mutex);
    mState->maxWriteSize = qMax(0, mtu - 3); // Rust: saturating_sub(3)
    for (auto it = mState->mlrCommunicators.begin(); it != mState->mlrCommunicators.end(); ++it) {
        it.value()->setMaxPacketSize(mState->maxWriteSize);
    }
}

void CommunicatorV2::onDeviceMaxPacketSize(quint16 deviceMaxPacketSize) {
    // Update the maximum packet size based on device-reported maximum
    //
    // The device reports its maximum supported packet size in the DeviceInformation message.
    // This method updates the maxWPaketSize to be the minimum of the current MTU-based size
    // and the device-reported maximum, ensuring we respect the device's limitations.

    //QMutexLocker lock(&m_mutex);
    const int dev = int(deviceMaxPacketSize);
    if (dev < mState->maxWriteSize) {
        mState->maxWriteSize = dev;
        for (auto it = mState->mlrCommunicators.begin(); it != mState->mlrCommunicators.end(); ++it) {
            it.value()->setMaxPacketSize(mState->maxWriteSize);
        }
    }
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
        cb = mAsyncMessageCallback;
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
    qDebug() <<Q_FUNC_INFO << "Garmin: initalizing device";
    QMutexLocker lock(&m_mutex);
    QMap<QString, QSharedPointer<QBLECharacteristic>> characteristicMap;


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
            characteristicMap[charInterface.property("UUID").toString()] = QSharedPointer<QBLECharacteristic>(new QBLECharacteristic(path, this));
        }
    }

    qDebug() << Q_FUNC_INFO << "Garmin: characteristics: " << characteristicMap.keys();

    for (int i = 2810; i <= 2814; i++) {
        QString serviceRec = QString(CommunicatorV2::BASE_UUID).arg(i);
        QString serviceSnd = QString(CommunicatorV2::BASE_UUID).arg(i+10);

        if (characteristicMap.contains(serviceRec) && characteristicMap.contains(serviceSnd))
        {
            mState->characteristicReceive = characteristicMap.value(serviceRec);
            mState->receivePath = serviceRec;
            mState->characteristicSend = characteristicMap.value(serviceSnd);
            mState->sendPath = serviceSnd;
            qDebug() << Q_FUNC_INFO << "Garmin: ML Characteristic found. Send " << serviceSnd << ", Receive " << serviceRec;
        }


        if ((mState->characteristicSend != NULL) && (mState->characteristicReceive  != NULL)) {
            lock.unlock();
            enableNotification(serviceRec);
            connect(this,&QBLEService::characteristicChanged,this,&CommunicatorV2::onCharacteristicChanged);
            connect(mState->characteristicReceive.data(),&QBLECharacteristic::characteristicRead,this,&CommunicatorV2::onCharacteristicChanged);

            const QByteArray closeAll = createCloseAllServicesMessage();
            QString errorMsg;
            mState->characteristicSend->writeValue(closeAll,&errorMsg);
            if (!errorMsg.isEmpty())
                return Result<bool>::isOk(false);


            return Result<bool>::isOk(true);        }
    }

    qDebug() << Q_FUNC_INFO << "Garmin: Failed to find any known Garmin ML characteristics";


     return Result<bool>::isOk(false);
}

/*
Result<bool> CommunicatorV2::mStateizeDeviceWithTransaction(const QString& transactionName) {
    //QMutexLocker lock(&m_mutex);

    auto tx = m_ble->createTransaction(transactionName);
    if (!tx) return Result<bool>::err(GarminError::bluetoothError(QStringLiteral("Transaction not available")));

    for (quint16 i = 0x2810; i <= 0x2814; ++i) {
        const QString recvUuid = fmtUuid(i);
        const QString sendUuid = fmtUuid(i + 0x10);

        auto recv = m_ble->getCharacteristic(recvUuid);
        auto send = m_ble->getCharacteristic(sendUuid);

        if (recv && send) {
            mState->characteristicReceive = recv;
            mState->characteristicSend = send;

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
    // Send a message to the device via the GFDI service
    //
    // Messages are COBS-encoded and sent via MLR protocol or directly depending on setup.

    qDebug() <<Q_FUNC_INFO << "Garmin: Send Message " << taskName << " Content " << message;
    if (message.isEmpty()) return Result<void>::err(GarminError::emptyMessage());

    // Acquire lock
    QMutexLocker lock(&m_mutex);

    QSharedPointer<MlrCommunicator> mlr;
    //QBLECharacteristic *sendChar;
    quint8 gfdiHandle = 0;
    int maxWriteSize = 20;

    if (!mState->handleByService.contains(Service::GFDI)) {
        qDebug() << Q_FUNC_INFO << "Garmin: No GFDI handle found";
        return Result<void>::err(GarminError::gfdiHandleNotSet());
    }

    gfdiHandle = mState->handleByService.value(Service::GFDI);

    qDebug() << Q_FUNC_INFO << "Sending message " << taskName << "viea GFDI handle " << gfdiHandle;

    // Extract and log message type with sequence number and response details


    if (message.size() >= 4) {
        int offset = 2;
        // Read message ID (2 bytes, little-endian)
        quint16 rawMsgId = le16(message.constData() + offset);

        // Check for sequence number (bit 15 set)
        // If bit 15 is set, the message ID is encoded with a sequence number
        // Format: [bit 15: 1] [bits 14-8: sequence] [bits 7-0: message_id - 5000]
        // We need to decode it: actual_id = (raw_id & 0xFF) + 5000
        // Sequence number: bit 15 set
        std::optional<quint16> sequenceNumber;
        if ((rawMsgId & 0x8000) != 0) {
            const quint16 seq = (rawMsgId >> 8) & 0x7F;
            rawMsgId = quint16((rawMsgId & 0xFF) + 5000);
            sequenceNumber = seq;
        }

        const quint16 msgId = rawMsgId;

        const quint16 msgType = msgId;
        //let msg_type = crate::messages::MessageId::from_u16(msg_id);

        // Build message type string
        QString typeString = messageIdToString(msgType).value();

        qDebug() << Q_FUNC_INFO << "Garmin: SENDING MESSAGE to watch:" << taskName;
        qDebug() << Q_FUNC_INFO << "Garmin: Message type: " << typeString;

        if (sequenceNumber) {
            qDebug() << Q_FUNC_INFO << "Garmin: Sequence number " << sequenceNumber.value();
        }

        // For Response messages, show what we're responding to and status
        if (msgId == 5000) {
            if (message.size() >= 9) {
                const quint16 origMsgId = le16(message.constData() + 4);
                const quint8 statusByte = quint8(message[6]);

                qDebug() << Q_FUNC_INFO << "Outgoing Response decoding: orig_msg_id " << origMsgId << ", status_byte=" << statusByte;

            } else {
                qDebug() << Q_FUNC_INFO << "Garmin: Response message too short: " << message.size() <<" bytes (need at least 9)";
            }
        }

        qDebug() << Q_FUNC_INFO << "Garmin: Message size: " << message.size() << " bytes";
    } else {
        qDebug() << Q_FUNC_INFO << "Garmin: NOT SENDING MESSAGE to watch:" << taskName <<", Massage too short " << message.size() << "bytes.";
    }

    const QByteArray payload = CobsCoDec::encode(message);



    //sendChar = mState->characteristicSend;
    const quint8 mlrHandle = (gfdiHandle & 0x0F);
    mlr = mState->mlrCommunicators.value(mlrHandle);


    if (mlr) {
        qDebug() << Q_FUNC_INFO << "Garmin: Sending message via MLR";
        return mlr->sendMessage(taskName, payload);
    }

    // No MLR => fragment if needed and write directly

    maxWriteSize = mState->maxWriteSize;
    if (!mState->characteristicSend) {
        qDebug() << Q_FUNC_INFO << "Garmin: No send characteristic found.";
        return Result<void>::err(GarminError::bluetoothError(QStringLiteral("Send characteristic not set")));
    }

    qDebug() << Q_FUNC_INFO << "Garmin: No Mlr found, sending directly";
    int remaining = payload.size();
    int position = 0;

    if (remaining > maxWriteSize - 1) {
        while (remaining > 0) {
            const int chunk = qMin(remaining, maxWriteSize - 1);
            QByteArray fragment;
            fragment.reserve(chunk + 1);
            fragment.append(char(gfdiHandle));
            fragment.append(payload.mid(position, chunk));

            //auto wr = awaitBleWrite(*sendChar, fragment, taskName);
            QString errorMsg;
            mState->characteristicSend->writeValue(fragment,&errorMsg);
            if (!errorMsg.isEmpty())
                return Result<void>::isOk();


            position += chunk;
            remaining -= chunk;
        }
    } else {
        QByteArray packet;
        packet.reserve(payload.size() + 1);
        packet.append(char(gfdiHandle));
        packet.append(payload);


        QString errorMsg;
        mState->characteristicSend->writeValue(packet,&errorMsg);
        if (!errorMsg.isEmpty())
            return Result<void>::isOk();

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
        if (!mState->handleByService.contains(Service::GFDI)) {
            return Result<void>::err(GarminError::gfdiHandleNotSet());
        }
        gfdiHandle = mState->handleByService.value(Service::GFDI);
        maxWriteSize = mState->maxWriteSize;
        if (!mState->characteristicSend) {
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

    //handles incoming messages for a characteristic

    if (data.isEmpty()) return ;

    QSharedPointer<MlrCommunicator> mlr;
    {
        //QMutexLocker lock(&m_mutex);
        // MLR packet detection: bit7 set
        if (data.size() >= 2 && (quint8(data[0]) & 0x80) != 0) {
            const quint8 handle = (quint8(data[0]) & 0x70) >> 4;
            mlr = mState->mlrCommunicators.value(handle);
        }
    }

    if (mlr) {
        // Pass data to mlr, which will emit gfdiDecoded with the decoded data.
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
        // Test: Seems non GFDI don't need cobscodec??
        /*
        QMutexLocker lock(&m_mutex);
        mState->cobsCodec.receiveBytes(data);
        auto decoded = mState->cobsCodec.retrieveMessage();
        if (decoded.has_value()) {
            lock.unlock();
            handleDecodedMessage(*decoded);
            return;
        }
        */
        handleDecodedMessage(data);
    }
    return;
}

void CommunicatorV2::onDeviceInformationReceived(DeviceInformationMessage &message)
{
    //received device information
    qDebug() << Q_FUNC_INFO << "Device friendly name is " << message.bluetoothFriendlyName;
    qDebug() << Q_FUNC_INFO << "Device software version is " << message.softwareVersion;
    qDebug() << Q_FUNC_INFO << "Device max packet size is " << message.maxPacketSize;

    //set max packet size
    onDeviceMaxPacketSize(message.maxPacketSize);

    //response to the information received
    Result<QByteArray> response =  GfdiMessageGenerator::deviceInformationResponse(message);
    if (response.ok)
    {
        auto wr = sendMessage("Device Information Response",response.value);
        if (!wr.ok) qDebug() << "Garmin: Writing Response to Device Information message failed";
    }
}



Result<void> CommunicatorV2::handleDecodedMessage(const QByteArray& decodedWithHandle) {
    qDebug() << Q_FUNC_INFO << "Garmin: handleDecodedMessage: " << decodedWithHandle.toHex();
    if (decodedWithHandle.isEmpty())
    {
        qDebug() << Q_FUNC_INFO << "Garmin: decoded with handle is empty!";
        return Result<void>::isOk();
    }

    const quint8 handle = quint8(decodedWithHandle[0]);
    const QByteArray payload = decodedWithHandle.mid(1);

    Service service;

    QSharedPointer<ServiceCallback> svcCb;
    //QSharedPointer<GfdiMessageCallback> syncCb;
    //QPointer<AsyncGfdiMessageCallback> asyncCb;

    qDebug() << Q_FUNC_INFO << "Garmin: handle decoded message: Found handle " << handle;

    {
        //QMutexLocker lock(&m_mutex);

        service = mState->serviceByHandle.value(handle, Service::GFDI);
        qDebug() << Q_FUNC_INFO << "Garmin handle decoded message: Found Service " << serviceToString(service);
        //svcCb = mState->serviceCallbacks.value(service);
    }
/*
    if (svcCb) {
        qDebug() << "Garmin: Communicator handles service";
        auto r = svcCb->onMessage(payload);
        if (!r.ok) return r;
    }
*/
    qDebug() << Q_FUNC_INFO << "Garmin: Communicator handling decoded message for service " << serviceToString(service);

    switch (service) {

    case Service::GFDI:
        qDebug() << Q_FUNC_INFO << "Garmin: handle GFDI";
        /*
        GfdiMessageParser::parse(payload);
        if (mMessageCallback) {
            return mMessageCallback->onMessage(payload);
        }
        if (mAsyncMessageCallback) {
            auto r = awaitAsyncCallback(payload);
            if (!r.ok) return Result<void>::err(r.error);
        }
        emit gfdiMessageReceived(payload);
        */
        if (mState->serviceCallbacks.contains(Service::GFDI))
        {
            mState->cobsCodec.receiveBytes(decodedWithHandle);
            auto decoded = mState->cobsCodec.retrieveMessage();
            if (decoded.has_value()) {
                return Result<void>::isOk();
            }
            mState->serviceCallbacks.value(Service::GFDI)->onMessage(payload);
        }
        return Result<void>::isOk();

    case Service::RealtimeSpo2:
        qDebug() << Q_FUNC_INFO << "Garmin: handle Realtime Spo2";
        if (mState->serviceCallbacks.contains(Service::RealtimeSpo2))
        {
            qDebug() << Q_FUNC_INFO << "Garmin: calling Dpo2 callback";

            mState->serviceCallbacks.value(Service::RealtimeSpo2)->onMessage(payload);
        }
        return Result<void>::isOk();
    case Service::RealtimeHr:
        qDebug() << Q_FUNC_INFO  << "Garmin: handle Realtime HeartRate";
        if (mState->serviceCallbacks.contains(Service::RealtimeHr))
        {
            mState->serviceCallbacks.value(Service::RealtimeHr)->onMessage(payload);
        }
        return Result<void>::isOk();
    default:
        qDebug() << Q_FUNC_INFO << "Garmin: decoded message is not handled: " << serviceToString(service);
        return Result<void>::isOk();
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
        service = mState->serviceByHandle.value(handle, Service::GFDI);
    }

    if (service == Service::GFDI) {
        return awaitAsyncCallback(payload);
    }
    return Result<std::optional<QByteArray>>::isOk(std::nullopt);
}

Result<void> CommunicatorV2::processHandleManagement(const QByteArray& message) {
    qDebug() << Q_FUNC_INFO << "Garmin: processHandlemanagement called";
    if (message.isEmpty()) {
        return Result<void>::err(GarminError::invalidMessage(QStringLiteral("Empty handle management message")));
    }

    auto requestType = requestTypeFromU8(quint8(message[0]));
    if (!requestType.ok) return Result<void>::err(requestType.error);


    if (message.size() < 9) {
        return Result<void>::err(GarminError::invalidMessage(QStringLiteral("Handle management message too short")));
    }

    //const quint64 clientId = readLe16(message, 1) | (quint64(readLe16(message, 3)) << 16) |
    //                         (quint64(readLe16(message, 5)) << 32) | (quint64(readLe16(message, 7)) << 48);
    const quint64 clientId = readLe64(message, 1);
    if (clientId != GADGETBRIDGE_CLIENT_ID) {
        return Result<void>::isOk();
    }

    qDebug() << Q_FUNC_INFO << "Garmin: Processing handle management:" << requestTypeToString(requestType.value);
    const QByteArray payload = message.mid(9);

    switch (requestType.value) {
    case RequestType::RegisterMlResp:
        qDebug() << "Garmin: handle RegisterMlResp";
        return processRegisterMlResp(payload);
    case RequestType::CloseHandleResp:
        qDebug() << "Garmin: handle CloseHendleResp";
        return processCloseHandleResp(payload);
    case RequestType::CloseAllResp:
        qDebug() << "Garmin: handle CloseAllResp";
        return processCloseAllResp();
    case RequestType::UnkResp:
        qDebug() << "Garmin: Received unkown response message:" << message;
        return Result<void>::isOk();

    default:
        qDebug() << Q_FUNC_INFO << "Garmin:Received unkown request type: " << requestTypeToString(requestType.value);
        return Result<void>::isOk();
    }
}

Result<void> CommunicatorV2::processRegisterMlResp(const QByteArray& payload) {
    qDebug() << Q_FUNC_INFO << "Garmin: processing RegisterMlResp to add handle " << payload.toHex();
    if (payload.size() < 5) {
        qDebug() << Q_FUNC_INFO << "Garmin: MLR Registration Response payload too short: " << payload.size();
        return Result<void>::err(GarminError::invalidMessage(QStringLiteral("RegisterMlResp payload too short")));
    }

    const quint16 serviceCodeLE = readLe16(payload, 0);
    const quint8 status = quint8(payload[2]);
    const quint8 handle = quint8(payload[3]);
    const quint8 reliable = quint8(payload[4]);

    auto svc = serviceFromCode(serviceCodeLE);

    if (!svc.ok) {
        qDebug() << Q_FUNC_INFO << "Garmin: Got register response status " << status << " for unknown service " << serviceCodeLE;
        return Result<void>::isOk();
    }

    qDebug() << Q_FUNC_INFO << "Garmin: Got register response status " << status << " for service " << serviceToString(svc.value);

    if (status != 0)  {
        qDebug() << Q_FUNC_INFO << "Garmin: Failed to register" << serviceCodeLE << ", status = " << status;
        return Result<void>::isOk();
    }

    // The service handle has the MLR bit encoded in it
    // For example: handle=134 (0x86) means MLR handle 6 with reliable bit set
    // Extract the actual MLR handle from bits 0-3 of the service handle

    const quint8 mlrHandle = (reliable != 0) ? (handle & 0x0F) : handle;

    const Service service = svc.value;

    QSharedPointer<MlrCommunicator> createdMlr;

    //QMutexLocker lock(&m_mutex);
    qDebug() << Q_FUNC_INFO <<"Garmin: Inserting handle " << handle <<" for service " << serviceToString(service);
    mState->serviceByHandle.insert(handle, service);
    mState->handleByService.insert(service, handle);

    // Create MLR communicator if reliable is enabled
    if ((reliable != 0) && !mState->mlrCommunicators.contains(mlrHandle)) {
        QSharedPointer<QBLECharacteristic> sendChar = mState->characteristicSend;
        if (!sendChar) {
            qDebug() << Q_FUNC_INFO << "Garmin: No send characteristic found!";
            return Result<void>::isOk();
        }

        qDebug() << Q_FUNC_INFO << "Garmin: Inserting reliable MLR communicator for " << serviceToString(service) << ", Handle " << mlrHandle;
        auto sender = QSharedPointer<MlrMessageSender>::create(sendChar, this);
        auto receiver = QSharedPointer<MlrMessageReceiver>::create(mMessageCallback, mAsyncMessageCallback, this);
        auto mlr = QSharedPointer<MlrCommunicator>::create(mlrHandle, 20, sender, receiver);
        mlr->start();
        qDebug() << Q_FUNC_INFO << "Garmin: Reliable MLR communicator created";

        mState->mlrCommunicators.insert(mlrHandle, mlr);



        // Connect mlr gfdiDecoded to Message handling
        connect(receiver.data(),&MlrMessageReceiver::gfdiDecoded,this,&CommunicatorV2::handleDecodedMessage);

        //connect(receiver.data(),&MlrMessageReceiver::gfdiDecoded,this,&CommunicatorV2::onCharacteristicChanged);
        createdMlr = mlr;

     }

    // If no callback is registered for this service, create a default one for known services
    if (!mState->serviceCallbacks.contains(service))
    {
        QMutexLocker lock(&m_mutex);
        switch (service) {
        case Service::GFDI:
            qDebug() << Q_FUNC_INFO <<  "Garmin: Inserting GFDI callback handle";
            mState->serviceCallbacks.insert(Service::GFDI, QSharedPointer<ServiceCallback>(new GfdiServiceCallback(mMessageCallback,this)));
            break;
        case Service::RealtimeSpo2:
            qDebug() << Q_FUNC_INFO << "Garmin: Inserting Spo2 callback handle";
            mState->serviceCallbacks.insert(service, QSharedPointer<ServiceCallback>(new RealtimeSpo2Callback(this)));
            break;
        case Service::RealtimeHr:
           qDebug() << Q_FUNC_INFO << "Garmin: Inserting realtime HR callback handle";
            mState->serviceCallbacks.insert(service, QSharedPointer<ServiceCallback>(new RealtimeHeartRateCallback(this)));
            break;
        case Service::RealtimeSteps:
           qDebug() << Q_FUNC_INFO << "Garmin: Inserting realtime Steps callback handle";
            mState->serviceCallbacks.insert(service, QSharedPointer<ServiceCallback>(new RealtimeStepsCallback(this)));
            break;
        default:
            // Create Default Callback
            //mState->serviceCallbacks.insert(service, QSharedPointer<ServiceCallback>(new DefaultCallback()));
            break;
        }
    }


    // Call onConnect for the service callback if registered
    QSharedPointer<ServiceCallback> cb;
    QSharedPointer<QBLECharacteristic> sendChar;
    {
       //QMutexLocker lock(&m_mutex);
       cb = mState->serviceCallbacks.value(service);
       if (mState->characteristicSend) sendChar = mState->characteristicSend;
       if (cb && sendChar) {
           QSharedPointer<ServiceWriter> writer(new MlrServiceWriter(mlrHandle,sendChar));
           (void)cb->onConnect(writer);
       }
    }

    // complete the pairing - technically only needed once, not on every connect. Should be modified accordingly

    completePairing();
    emit mlrConnected();
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
        cb = mState->serviceCallbacks.take(service);
        mState->handleByService.remove(service);
        mState->serviceByHandle.remove(handle);
        mState->mlrCommunicators.remove(handle);
    }
    if (cb) (void)cb->onClose();
    return Result<void>::isOk();
}

Result<void> CommunicatorV2::processCloseAllResp() {
    QSharedPointer <QBLECharacteristic> sendChar;
    QList<QSharedPointer<ServiceCallback>> callbacks;

    {
        QMutexLocker lock(&m_mutex);
        mState->serviceByHandle.clear();
        mState->handleByService.clear();

        for (auto it = mState->serviceCallbacks.begin(); it != mState->serviceCallbacks.end(); ++it) {
            callbacks.push_back(it.value());
        }
        mState->serviceCallbacks.clear();
        mState->mlrCommunicators.clear();

        sendChar = mState->characteristicSend;
    }

    for (auto& cb : callbacks) {
        if (cb) (void)cb->onClose();
    }

    if (sendChar) {
        const QByteArray reg = createRegisterServiceMessage(Service::GFDI, true);
        sendChar->writeValue(reg);
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
        if (!mState->handleByService.contains(service)) {
            return Result<void>::err(GarminError::bluetoothError(QStringLiteral("Service not registered")));
        }
        handle = mState->handleByService.value(service);
    }
    const QByteArray msg = createCloseServiceMessage(service, handle);
    return sendMessage(QStringLiteral("close_%1").arg(serviceToString(service)), msg);
}

void CommunicatorV2::registerHandle(Service service, quint8 handle) {
    //QMutexLocker lock(&m_mutex);
    mState->serviceByHandle.insert(handle, service);
    mState->handleByService.insert(service, handle);
}

Result<bool> CommunicatorV2::completePairing() {
    // To complete the initial pairing, some messages must be sent to the device
    qDebug() << "Garmin: Checking for first connection";
    if (isFirstConncet && !isPairing) {
        isPairing = true;

        //QMutexLocker lock(&m_mutex);

        auto msg1 = GfdiMessageGenerator::systemEvent(8,0);
        if (!msg1.ok) return Result<bool>::isOk(false);
        qDebug() << "Garmin: Sync Ready";
        auto wr1 = sendMessage("SYNC_READY",msg1.value);
        if (!wr1.ok) qDebug() << "Garmin: Sync Ready failed";
        if (!wr1.ok) return Result<bool>::err(wr1.error);
        auto msg2 = GfdiMessageGenerator::systemEvent(4,0);
        if (!msg2.ok) return Result<bool>::isOk(false);
        qDebug() << "Garmin: Sending Pair Complete";
        auto wr2 = sendMessage("PAIR_COMPLETE",msg2.value);
        if (!wr2.ok) qDebug() << "Garmin: Pair Complete failed";
        if (!wr2.ok) return Result<bool>::err(wr2.error);
        auto msg3 = GfdiMessageGenerator::systemEvent(0,0);
        if (!msg3.ok) return Result<bool>::isOk(false);
        qDebug() << "Garmin: Sending Sync Complete";
        auto wr3 = sendMessage("SYNC_COMPLETE",msg3.value);
        if (!wr3.ok) qDebug() << "Garmin: Sync Complete failed";
        if (!wr3.ok) return Result<bool>::err(wr3.error);
        auto msg4 = GfdiMessageGenerator::systemEvent(14,0);
        if (!msg4.ok) return Result<bool>::isOk(false);
        qDebug() << "Garmin: Sending SetupWizard Complete";
        auto wr4 = sendMessage("SETUP_WIZARD_COMPLETE",msg4.value);
        if (!wr4.ok) qDebug() << "Garmin: Setup Wizard complete failed";
        isPairing=false;
        if (wr4.ok) {
            isFirstConncet=false;
            emit pairingComplete();
            return Result<bool>::isOk(true);
        }
        if (!wr4.ok) return Result<bool>::err(wr4.error);
    }
    return Result<bool>::isOk(true);
}

Result<void> CommunicatorV2::registerServices() {
    qDebug() << "Garmin: handle service registraation";
    if (!mState->characteristicSend)
    {
        qDebug() << "Garmin: Can#t register services due to missing send characteristic";
    }
        QString errorMsg;

        //const QByteArray reg = createRegisterServiceMessage(Service::GFDI, true);
/*
        QByteArray reg = createRegisterServiceMessage(Service::GFDI, true);
        //(void)awaitBleWrite(*sendChar, reg, QStringLiteral("register_gfdi"));
        mState->characteristicSend->writeValue(reg,&errorMsg);
        if (!errorMsg.isEmpty())
             return Result<void>::isOk();
*/

        QByteArray reg = createRegisterServiceMessage(Service::RealtimeHr, true);
        mState->characteristicSend->writeValue(reg,&errorMsg);
        if (!errorMsg.isEmpty())
            return Result<void>::isOk();
        reg.clear();
        reg = createRegisterServiceMessage(Service::RealtimeSpo2, true);
        //(void)awaitBleWrite(*sendChar, reg, QStringLiteral("register_Spo2"));
        mState->characteristicSend->writeValue(reg,&errorMsg);
        if (!errorMsg.isEmpty())
            return Result<void>::isOk();
        reg.clear();
        reg = createRegisterServiceMessage(Service::RealtimeSteps, true);
        //(void)awaitBleWrite(*sendChar, reg, QStringLiteral("register_Steps"));
        mState->characteristicSend->writeValue(reg,&errorMsg);
        if (!errorMsg.isEmpty())
            return Result<void>::isOk();


         auto msg = GfdiMessageGenerator::protobufBatteryStatusRequest(0);
         if (!msg.ok) return Result<void>::isOk();
         mState->characteristicSend->writeValue(msg.value,&errorMsg);
         if (!errorMsg.isEmpty())
             return Result<void>::isOk();

        //Test: REgister HRM & Steps
        //registerService(Service::RealtimeHr,true);
        //registerService(Service::RealtimeSpo2,true);
        //registerService(Service::RealtimeSteps,true);

        return Result<void>::isOk();

}

Result<void> CommunicatorV2::dispose() {
    //QMutexLocker lock(&m_mutex);
    mState->mlrCommunicators.clear();
    return Result<void>::isOk();
}

void CommunicatorV2::onConnectionStateChange(bool connected) {
    if (!connected) clearAndPauseMlr();
}

void CommunicatorV2::pauseMlr() {
    //QMutexLocker lock(&m_mutex);
    for (auto it = mState->mlrCommunicators.begin(); it != mState->mlrCommunicators.end(); ++it) {
        it.value()->pause();
    }
}

void CommunicatorV2::resumeMlr() {
    //QMutexLocker lock(&m_mutex);
    for (auto it = mState->mlrCommunicators.begin(); it != mState->mlrCommunicators.end(); ++it) {
        it.value()->resume();
    }
}

void CommunicatorV2::clearAndPauseMlr() {
    //QMutexLocker lock(&m_mutex);
    for (auto it = mState->mlrCommunicators.begin(); it != mState->mlrCommunicators.end(); ++it) {
        it.value()->clearAndPause();
    }
    mState->mlrCommunicators.clear();
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
                                   QSharedPointer<QBLECharacteristic> sendChar)
    : m_handle(handle), m_sendChar(sendChar)
{}


Result<void> MlrServiceWriter::write(const QString& taskName, const QByteArray& data) {
    QByteArray payload;
    payload.reserve(data.size() + 1);
    payload.append(char(m_handle));
    payload.append(data);
    //return awaitBleWrite(taskName, payload);
    QString errorMsg;
    m_sendChar->writeValue(data,&errorMsg);
    qDebug() << "Garmin: Mlr Service writer wrote, error returned: " << errorMsg;
    return Result<void>::isOk();
}

// =============================================================================
// Callbacks handling the actual responses based on the handle/service
// =============================================================================


GfdiServiceCallback::GfdiServiceCallback(QSharedPointer<GfdiMessageCallback> cb, CommunicatorV2* parent)
    : m_cb(std::move(cb)), mCommunicator(parent)
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
    GfdiMessageParser parser;
    connect(&parser,&GfdiMessageParser::deviceInformationReceived,mCommunicator,&CommunicatorV2::onDeviceInformationReceived);
    parser.parse(data);

    return Result<void>::isOk();

    qDebug() << Q_FUNC_INFO << "Garmin: GfdiServicecallback called with data " << data;
    m_codec.receiveBytes(data);

    if (data.size() < 6) {
        return Result<void>::err(GarminError::invalidMessage(QStringLiteral("Message too short")));
    }

    int offset = 0;
    offset += 2; // packet size field, skip over

    // Read message ID (2 bytes, little-endian)
    quint16 rawId = le16(data.constData() + offset);
    offset += 2;

    // Check for sequence number (bit 15 set)
    // If bit 15 is set, the message ID is encoded with a sequence number
    // Format: [bit 15: 1] [bits 14-8: sequence] [bits 7-0: message_id - 5000]
    // We need to decode it: actual_id = (raw_id & 0xFF) + 5000
    quint16 msgId = rawId;
    if ((msgId & 0x8000) != 0) {
        msgId = (msgId & 0xFF) + 5000;
    }

    const auto mid = messageIdFromU16(msgId);
    if (!mid.has_value()) {
        qDebug() << "Garmin: Unknown GFDI message: " << msgId;
        return Result<void>::err(GarminError::invalidMessage(QStringLiteral("Unknown ID")));
    }

    return Result<void>::isOk();
}


RealtimeHeartRateCallback::RealtimeHeartRateCallback(CommunicatorV2* parent) : mCommunicator(parent)
{

}
Result<void> RealtimeHeartRateCallback::onConnect(QSharedPointer<ServiceWriter> writer) { Q_UNUSED(writer); return Result<void>::isOk(); }
Result<void> RealtimeHeartRateCallback::onClose() { return Result<void>::isOk(); }
Result<void> RealtimeHeartRateCallback::onMessage(const QByteArray& data) {
    qDebug() << Q_FUNC_INFO << "Garmin: Realtime Heart Rate Servicecallback called with data " << data;
    quint8 type = data[0]; // 0/2/3? 3 == realtime?
    quint8 hr = data[1] & 0xff;
    quint8 resting = data[2] & 0xff;
    qDebug() << "Garmin: Got realtime HR type: " << type << ", hr=" << hr << ", resting=" << resting;
    return Result<void>::isOk();
}

RealtimeSpo2Callback::RealtimeSpo2Callback(CommunicatorV2* parent) : mCommunicator(parent)
{

}
Result<void> RealtimeSpo2Callback::onConnect(QSharedPointer<ServiceWriter> writer) { Q_UNUSED(writer); return Result<void>::isOk(); }
Result<void> RealtimeSpo2Callback::onClose() { return Result<void>::isOk(); }
Result<void> RealtimeSpo2Callback::onMessage(const QByteArray& data) {
    qDebug() << Q_FUNC_INFO << "Garmin: Realtime  Spo2 Servicecallback called with data " << data;
    quint8 spo2 = data[0];
    quint32 ts = le32(data.constData()+1);

    ts=ts + 631065600; // Unix timestamp in seconds
    QDateTime Timestamp;

    qDebug() << Q_FUNC_INFO << "Garmin: realtime Spo2 Callback: " << spo2 << ", Timestamp " << QDateTime::fromTime_t(ts).toString();
    return Result<void>::isOk();
}

RealtimeStepsCallback::RealtimeStepsCallback(CommunicatorV2* parent) : mCommunicator(parent)
{

}
Result<void> RealtimeStepsCallback::onConnect(QSharedPointer<ServiceWriter> writer) { Q_UNUSED(writer); return Result<void>::isOk(); }
Result<void> RealtimeStepsCallback::onClose() { return Result<void>::isOk(); }
Result<void> RealtimeStepsCallback::onMessage(const QByteArray& data) {
    qDebug() << Q_FUNC_INFO << "Garmin: Realtime  Steps Servicecallback called with data " << data;

    if (data.size() >= 8)
    {
        quint32 steps = le32(data.constData());
        quint32 stepsGoal = le32(data.constData()+4);
        qDebug() << Q_FUNC_INFO << "Garmin: Realtime  Steps :  " << steps << ", goal = " << stepsGoal;
        return Result<void>::isOk();
    }
    return Result<void>::isOk();
}


