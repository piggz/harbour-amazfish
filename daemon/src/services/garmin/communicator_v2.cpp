#include "communicator_v2.h"
#include "devices/garmin/garmindevice.h"
#include "garmintypes.h"
#include "cobscodec.h"      // CobsCoDec
#include "garminmlr.h"   // MlrCommunicator (Qt)
#include "garmintypes.h"
#include "garminmessages.h"
#include "amazfishconfig.h"


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
// CommunicatorV2
// =============================================================================

QString CommunicatorV2::baseUuid(quint16 shortId) { return fmtUuid(shortId); }

CommunicatorV2::CommunicatorV2(const QString &path, QObject* parent)
    : QBLEService(UUID_SERVICE_GARMIN_ML_GFDI, path, parent), mState(CommunicatorState::create()),
          m_Path(path), m_device(parent)
{
    qDebug() << "Garmin: Service created for " << path;
    connect(this, &QBLEService::characteristicRead, this, &CommunicatorV2::characteristicRead);
    mMessageCallback =QSharedPointer<GfdiMessageCallback>::create();
    //mAsyncMessageCallback = QSharedPointer<AsyncGfdiMessageCallback>::create(new AsyncGfdiMessageCallback());
    connect(this,&CommunicatorV2::pairingComplete,this,&CommunicatorV2::registerServices);
    initializeDevice();

}

void CommunicatorV2::setMessageCallback(QSharedPointer<GfdiMessageCallback> cb) {
    mMessageCallback = std::move(cb);
}


void CommunicatorV2::registerServiceCallback(Service service, QSharedPointer<ServiceCallback> cb) {
    mState->serviceCallbacks.insert(service, std::move(cb));
}

QSharedPointer<ServiceCallback> CommunicatorV2::unregisterServiceCallback(Service service) {
    return mState->serviceCallbacks.take(service);
}

std::optional<QString> CommunicatorV2::getReceiveCharacteristicUuid() const {
    if (!mState->characteristicReceive) return std::nullopt;
    return mState->characteristicReceive->value();
}

void CommunicatorV2::onMtuChanged(int mtu) {
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

    const int dev = int(deviceMaxPacketSize);
    if (dev < mState->maxWriteSize) {
        mState->maxWriteSize = dev;
        for (auto it = mState->mlrCommunicators.begin(); it != mState->mlrCommunicators.end(); ++it) {
            it.value()->setMaxPacketSize(mState->maxWriteSize);
        }
    }
}


quint64 CommunicatorV2::nextCookie() {
    return m_cookieCounter++;
}

bool CommunicatorV2::initializeDevice() {
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
            mState->characteristicSend = characteristicMap.value(serviceSnd);
            qDebug() << Q_FUNC_INFO << "Garmin: ML Characteristic found. Send " << serviceSnd << ", Receive " << serviceRec;
        }


        if ((mState->characteristicSend != NULL) && (mState->characteristicReceive  != NULL)) {
            qDebug() << Q_FUNC_INFO << "Garmin: ML Characteristic found. Send " << serviceSnd << ", Receive " << serviceRec;
            lock.unlock();

            enableNotification(serviceRec);
            connect(this,&QBLEService::characteristicChanged,this,&CommunicatorV2::onCharacteristicChanged);
            connect(mState->characteristicReceive.data(),&QBLECharacteristic::characteristicRead,this,&CommunicatorV2::onCharacteristicChanged);

            const QByteArray closeAll = createCloseAllServicesMessage();
            QString errorMsg;
            mState->characteristicSend->writeValue(closeAll,&errorMsg);
            if (!errorMsg.isEmpty())
                return false;

            return true;
        }
    }

    qDebug() << Q_FUNC_INFO << "Garmin: Failed to find any known Garmin ML characteristics";


     return false;
}

void CommunicatorV2::sendMessage(const QString& taskName, const QByteArray& message) {
    // Send a message to the device via the GFDI service
    //
    // Messages are COBS-encoded and sent via MLR protocol or directly depending on setup.

    qDebug() <<Q_FUNC_INFO << "Garmin: Send Message " << taskName << " Content " << message;
    if (message.isEmpty())
        return;

    //Locking
    QMutexLocker lock(&m_mutex);


    QSharedPointer<MlrCommunicator> mlr;
    //QBLECharacteristic *sendChar;
    quint8 gfdiHandle = 0;
    int maxWriteSize = 20;

    if (!mState->handleByService.contains(Service::GFDI)) {
        qDebug() << Q_FUNC_INFO << "Garmin: No GFDI handle found";
        return;
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
        mlr->sendMessage(taskName, payload);
        return;
    }

    // No MLR => fragment if needed and write directly

    maxWriteSize = mState->maxWriteSize;
    if (!mState->characteristicSend) {
        qDebug() << Q_FUNC_INFO << "Garmin: No send characteristic found.";
        return;
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
                return;


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
            return;

    }
}

void CommunicatorV2::onCharacteristicChanged(const QString &characteristic, const QByteArray& data) {

    //handles incoming messages for a characteristic

    if (data.isEmpty()) return ;

    QSharedPointer<MlrCommunicator> mlr;

    if (data.size() >= 2 && (quint8(data[0]) & 0x80) != 0) {
        const quint8 handle = (quint8(data[0]) & 0x70) >> 4;
        mlr = mState->mlrCommunicators.value(handle);
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

    // handle message if not mlr
    handleDecodedMessage(data);
    return;
}

void CommunicatorV2::onDeviceInformationReceived(DeviceInformationMessage &message)
{
    //received device information is " << message.maxPacketSize;
    mDeviceInfo.deviceName=message.deviceName;
    mDeviceInfo.deviceModel=message.deviceModel;
    mDeviceInfo.serialNumber=message.unitNumber;
    mDeviceInfo.softwareRevision=message.softwareVersion;

    //set max packet size
    onDeviceMaxPacketSize(message.maxPacketSize);

    //response to the information received
    Result<QByteArray> response =  GfdiMessageGenerator::deviceInformationResponse(message);
    if (response.ok)
    {
        sendMessage("Device Information Response",response.value);
    }
}



void CommunicatorV2::handleDecodedMessage(const QByteArray& decodedWithHandle) {
    qDebug() << Q_FUNC_INFO << "Garmin: handleDecodedMessage: " << decodedWithHandle.toHex();
    if (decodedWithHandle.isEmpty())
    {
        qDebug() << Q_FUNC_INFO << "Garmin: decoded with handle is empty!";
        return;
    }

    const quint8 handle = quint8(decodedWithHandle[0]);
    const QByteArray payload = decodedWithHandle.mid(1);

    Service service;

    QSharedPointer<ServiceCallback> svcCb;

    qDebug() << Q_FUNC_INFO << "Garmin: handle decoded message: Found handle " << handle;

    service = mState->serviceByHandle.value(handle, Service::GFDI);

    switch (service) {

    case Service::GFDI:
        qDebug() << Q_FUNC_INFO << "Garmin: handle GFDI";
        if (mState->serviceCallbacks.contains(Service::GFDI))
        {
            mState->cobsCodec.receiveBytes(decodedWithHandle);
            auto decoded = mState->cobsCodec.retrieveMessage();
            if (decoded.has_value()) {
                return;
            }
            mState->serviceCallbacks.value(Service::GFDI)->onMessage(payload);
        }
        return;

    case Service::RealtimeSpo2:
        qDebug() << Q_FUNC_INFO << "Garmin: handle Realtime Spo2";
        if (mState->serviceCallbacks.contains(Service::RealtimeSpo2))
        {
            qDebug() << Q_FUNC_INFO << "Garmin: calling Dpo2 callback";

            mState->serviceCallbacks.value(Service::RealtimeSpo2)->onMessage(payload);
        }
        return;
    case Service::RealtimeHr:
        qDebug() << Q_FUNC_INFO  << "Garmin: handle Realtime HeartRate";
        if (mState->serviceCallbacks.contains(Service::RealtimeHr))
        {
            mState->serviceCallbacks.value(Service::RealtimeHr)->onMessage(payload);
        }
        return;
    case Service::RealtimeHrv:
        qDebug() << Q_FUNC_INFO  << "Garmin: handle Realtime HRV";
        if (mState->serviceCallbacks.contains(Service::RealtimeHrv))
        {
            mState->serviceCallbacks.value(Service::RealtimeHrv)->onMessage(payload);
        }
        return;
    case Service::RealtimeSteps:
        qDebug() << Q_FUNC_INFO  << "Garmin: handle Realtime Steps";
        if (mState->serviceCallbacks.contains(Service::RealtimeSteps))
        {
            mState->serviceCallbacks.value(Service::RealtimeSteps)->onMessage(payload);
        }
        return;
    default:
        qDebug() << Q_FUNC_INFO << "Garmin: decoded message is not handled: " << serviceToString(service);
    }
}

void CommunicatorV2::processHandleManagement(const QByteArray& message) {
    qDebug() << Q_FUNC_INFO << "Garmin: processHandlemanagement called";
    if (message.isEmpty()) {
        return;
    }

    auto requestType = requestTypeFromU8(quint8(message[0]));
    if (!requestType.ok) {
        qDebug() << Q_FUNC_INFO << "Garmin: no valid request type";
        return;
    }


    if (message.size() < 9) {
        qDebug() << Q_FUNC_INFO << "Garmin: message too small";
        return;
    }

    const quint64 clientId = readLe64(message, 1);
    if (clientId != AMAZFISH_CLIENT_ID) {
        return;
    }

    qDebug() << Q_FUNC_INFO << "Garmin: Processing handle management:" << requestTypeToString(requestType.value);
    const QByteArray payload = message.mid(9);

    switch (requestType.value) {
    case RequestType::RegisterMlResp:
        qDebug() << "Garmin: handle RegisterMlResp";
        processRegisterMlResp(payload);
        return;
    case RequestType::CloseHandleResp:
        qDebug() << "Garmin: handle CloseHendleResp";
        processCloseHandleResp(payload);
        return;
    case RequestType::CloseAllResp:
        qDebug() << "Garmin: handle CloseAllResp";
        processCloseAllResp();
        return;
    case RequestType::UnkResp:
        qDebug() << "Garmin: Received unkown response message:" << message;
        return;
    default:
        qDebug() << Q_FUNC_INFO << "Garmin:Received unkown request type: " << requestTypeToString(requestType.value);
        return;
    }
}

void CommunicatorV2::processRegisterMlResp(const QByteArray& payload) {
    if (payload.size() < 5) {
        qDebug() << Q_FUNC_INFO << "Garmin: MLR Registration Response payload too short: " << payload.size();
        return;
    }

    const quint16 serviceCodeLE = readLe16(payload, 0);
    const quint8 status = quint8(payload[2]);
    const quint8 handle = quint8(payload[3]);
    const quint8 reliable = quint8(payload[4]);

    auto svc = serviceFromCode(serviceCodeLE);

    if (!svc.ok) {
        qDebug() << Q_FUNC_INFO << "Garmin: Got register response status " << status << " for unknown service " << serviceCodeLE;
        return;
    }

    qDebug() << Q_FUNC_INFO << "Garmin: Got register response status " << status << " for service " << serviceToString(svc.value);

    if (status != 0)  {
        qDebug() << Q_FUNC_INFO << "Garmin: Failed to register" << serviceCodeLE << ", status = " << status;
        return;
    }

    // The service handle has the MLR bit encoded in it
    // For example: handle=134 (0x86) means MLR handle 6 with reliable bit set
    // Extract the actual MLR handle from bits 0-3 of the service handle

    const quint8 mlrHandle = (reliable != 0) ? (handle & 0x0F) : handle;

    const Service service = svc.value;

    QSharedPointer<MlrCommunicator> createdMlr;

    qDebug() << Q_FUNC_INFO <<"Garmin: Inserting handle " << handle <<" for service " << serviceToString(service);
    mState->serviceByHandle.insert(handle, service);
    mState->handleByService.insert(service, handle);

    // Create MLR communicator if reliable is enabled
    if ((reliable != 0) && !mState->mlrCommunicators.contains(mlrHandle)) {
        QSharedPointer<QBLECharacteristic> sendChar = mState->characteristicSend;
        if (!sendChar) {
            qDebug() << Q_FUNC_INFO << "Garmin: No send characteristic found!";
            return;
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

        createdMlr = mlr;

     }

    // If no callback is registered for this service, create a default one for known services
    if (!mState->serviceCallbacks.contains(service))
    {
        QMutexLocker lock(&m_mutex);
        switch (service) {
        case Service::GFDI:
            qDebug() << Q_FUNC_INFO <<  "Garmin: Inserting GFDI callback handle";
            registerServiceCallback(Service::GFDI,QSharedPointer<ServiceCallback>(new GfdiServiceCallback(mMessageCallback,this)));
            break;
        case Service::RealtimeSpo2:

            qDebug() << Q_FUNC_INFO << "Garmin: Inserting Spo2 callback handle";
            registerServiceCallback(Service::RealtimeSpo2,QSharedPointer<ServiceCallback>(new RealtimeSpo2Callback(this)));
            break;
        case Service::RealtimeHr:
            qDebug() << Q_FUNC_INFO << "Garmin: Inserting realtime HR callback handle";
            registerServiceCallback(Service::RealtimeHr,QSharedPointer<ServiceCallback>(new RealtimeHeartRateCallback(this)));
            break;
        case Service::RealtimeHrv:
            qDebug() << Q_FUNC_INFO << "Garmin: Inserting realtime HRV callback handle";
            registerServiceCallback(Service::RealtimeHrv,QSharedPointer<ServiceCallback>(new RealtimeHRVCallback(this)));
            break;
        case Service::RealtimeSteps:
            qDebug() << Q_FUNC_INFO << "Garmin: Inserting realtime Steps callback handle";
            registerServiceCallback(Service::RealtimeSteps,QSharedPointer<ServiceCallback>(new RealtimeStepsCallback(this)));
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
       cb = mState->serviceCallbacks.value(service);
       if (mState->characteristicSend) sendChar = mState->characteristicSend;
       if (cb && sendChar) {
           QSharedPointer<ServiceWriter> writer(new MlrServiceWriter(mlrHandle,sendChar));
           (void)cb->onConnect(writer);
       }
    }

    // complete the pairing - technically only needed once, not on every connect. Doesn't seem to break anything but should be looked into

    completePairing();
    emit mlrConnected();
    return;
}

void CommunicatorV2::processCloseHandleResp(const QByteArray& payload) {
    if (payload.size() < 4) {
        qDebug() << Q_FUNC_INFO << "CloseHandleResp payload too short";
        return;
    }

    const quint16 serviceCodeLE = readLe16(payload, 0);
    const quint8 handle = quint8(payload[2]);

    auto svc = serviceFromCode(serviceCodeLE);
    if (!svc.ok)
        return;
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
    return;
}

void CommunicatorV2::processCloseAllResp() {
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
        registerService(Service::GFDI, true);
        //const QByteArray reg = createRegisterServiceMessage(Service::GFDI, true);
        //sendChar->writeValue(reg);
    }
}

void CommunicatorV2::registerService(Service service, bool reliable) {
    qDebug() << Q_FUNC_INFO << "Garmin: Registering service " << serviceToString(service);

    const QByteArray msg = createRegisterServiceMessage(service, reliable);
    QString errorMsg;
    if (!mState->characteristicSend) {
        qDebug() << Q_FUNC_INFO << "Garmin: Can't register service due to missing send characteristic.";
        return;
    }
    mState->characteristicSend->writeValue(msg,&errorMsg);
    if (!errorMsg.isEmpty())
        qDebug() << Q_FUNC_INFO << "Garmin: could not register Service " << serviceToString(service);
}

void CommunicatorV2::closeService(Service service) {
    quint8 handle = 0;
    if (!mState->handleByService.contains(service)) {
        return;
    }
    handle = mState->handleByService.value(service);

    const QByteArray msg = createCloseServiceMessage(service, handle);
    sendMessage(QStringLiteral("close_%1").arg(serviceToString(service)), msg);
}

void CommunicatorV2::registerHandle(Service service, quint8 handle) {
    mState->serviceByHandle.insert(handle, service);
    mState->handleByService.insert(service, handle);
}

bool CommunicatorV2::completePairing() {
    // To complete the initial pairing, some messages must be sent to the device
    qDebug()<< Q_FUNC_INFO << "Garmin: Checking for first connection";
    if (isFirstConncet && !isPairing) {
        isPairing = true;

        auto msg1 = GfdiMessageGenerator::systemEvent(8,0);
        if (!msg1.ok) return false;
        sendMessage("SYNC_READY",msg1.value);

        auto msg2 = GfdiMessageGenerator::systemEvent(4,0);
        if (!msg2.ok) return false;
        sendMessage("PAIR_COMPLETE",msg2.value);

        auto msg3 = GfdiMessageGenerator::systemEvent(0,0);
        if (!msg3.ok) return false;
        sendMessage("SYNC_COMPLETE",msg3.value);

        auto msg4 = GfdiMessageGenerator::systemEvent(14,0);
        if (!msg4.ok) return false;
        sendMessage("SETUP_WIZARD_COMPLETE",msg4.value);

        isPairing=false;
        isFirstConncet=false;
        qDebug()<< Q_FUNC_INFO << "Garmin: pairing complete.";
        emit pairingComplete();
        return true;
    }
    return true;
}

void CommunicatorV2::registerServices() {
    qDebug() << "Garmin: handle service registration";
    if (!mState->characteristicSend)
    {
        qDebug() << "Garmin: Can't register services due to missing send characteristic";
    }
    QString errorMsg;


    // GFDI Service is already registered
    //registerService(Service::GFDI, true);
    registerService(Service::RealtimeSpo2, true);
    registerService(Service::RealtimeHr, true);
    registerService(Service::RealtimeHrv, true);
    registerService(Service::RealtimeSteps, true);

    // Battery Status needs to be treated differently
    auto msg = GfdiMessageGenerator::protobufBatteryStatusRequest(0);
    if (!msg.ok) {
        qDebug() << Q_FUNC_INFO << "Garmin: could not generate battery request message";
        return;
    }
    mState->characteristicSend->writeValue(msg.value,&errorMsg);
    if (!errorMsg.isEmpty())
        qDebug() << Q_FUNC_INFO << errorMsg;

}

void CommunicatorV2::dispose() {
    mState->mlrCommunicators.clear();
}

void CommunicatorV2::onConnectionStateChange(bool connected) {
    if (!connected) clearAndPauseMlr();
}

void CommunicatorV2::pauseMlr() {
    for (auto it = mState->mlrCommunicators.begin(); it != mState->mlrCommunicators.end(); ++it) {
        it.value()->pause();
    }
}

void CommunicatorV2::resumeMlr() {
    for (auto it = mState->mlrCommunicators.begin(); it != mState->mlrCommunicators.end(); ++it) {
        it.value()->resume();
    }
}

void CommunicatorV2::clearAndPauseMlr() {
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
    appendLe64(b, AMAZFISH_CLIENT_ID);
    appendLe16(b, 0);
    b.append(char(0));
    return b;
}

QByteArray CommunicatorV2::createRegisterServiceMessage(Service service, bool reliable) const {
    QByteArray b;
    b.reserve(13);
    b.append(char(0));
    b.append(char(quint8(RequestType::RegisterMlReq)));
    appendLe64(b, AMAZFISH_CLIENT_ID);
    appendLe16(b, serviceCode(service));
    b.append(char(reliable ? 2 : 0));
    return b;
}

QByteArray CommunicatorV2::createCloseServiceMessage(Service service, quint8 handle) const {
    QByteArray b;
    b.reserve(12);
    b.append(char(0));
    b.append(char(quint8(RequestType::CloseHandleReq)));
    appendLe64(b, AMAZFISH_CLIENT_ID);
    appendLe16(b, serviceCode(service));
    b.append(char(handle));
    return b;
}

void CommunicatorV2::characteristicRead(const QString &c, const QByteArray &value)
{

        qDebug() << Q_FUNC_INFO << c << "=================" << value;
        qDebug() << "Garmin: Charateristics Read called";

}

void CommunicatorV2::setSteps(quint32 val)
{
    qDebug() << Q_FUNC_INFO << "Garmin: setting steps to " << val;
    mSteps = val;
    emit informationChanged(Amazfish::Info::INFO_STEPS, QString::number(val));
}

void CommunicatorV2::setStepsGoal(quint32 val)
{
    qDebug() << Q_FUNC_INFO << "Garmin: setting steps goal to " << val;
    int configuredGoal=AmazfishConfig::instance()->profileFitnessGoal();
    if((val != mStepsGoal) || (val != configuredGoal))
    {

        mStepsGoal=val;
        AmazfishConfig::instance()->setProfileFitnessGoal(val);
    }

    emit informationChanged(Amazfish::Info::INFO_STEPS, QString::number(val));
}

void CommunicatorV2::setHeartRate(quint8 val)
{
    qDebug() << Q_FUNC_INFO << "Garmin: setting Heart Rate to " << val;
    mHeartRate =val;
    emit informationChanged(Amazfish::Info::INFO_HEARTRATE, QString::number(val));
}

void CommunicatorV2::setHRV(quint16 val)
{
    qDebug() << Q_FUNC_INFO << "Garmin: setting HRV to " << val;
    mHRV =val;
    // Future use - savin HRV
    //saveHRVRecord();

}

void CommunicatorV2::setSpo2(quint8 val)
{
    qDebug() << Q_FUNC_INFO << "Garmin: setting Spo2 to " << val;
    mSpo2 =val;
    // Future use - save Spo2
    //saveSpo2Record();
}



void CommunicatorV2::saveHRVRecord()
{

    KDbConnection *con = qobject_cast<GarminDevice *> (m_device)->database();
    if (!con)
    {
        qDebug() << Q_FUNC_INFO << "Garmin: database connection not available to save HRV.";
    }
    qDebug() << Q_FUNC_INFO << "Garmin: saving HRV to database: " << mHRV;

    QSharedPointer<KDbSqlResult> result;

    KDbTransaction transaction = con->beginTransaction();
    KDbTransactionGuard tg(transaction);

    QDateTime timestamp(QDateTime::currentDateTime());

    if (con && con->isDatabaseUsed()) {
        auto hrvData = con->tableSchema("hrv");
        KDbFieldList hrvFields;
        hrvFields.addField(hrvData->field("hrv_timestamp"));
        hrvFields.addField(hrvData->field("hrv_timestamp_dt"));
        hrvFields.addField(hrvData->field("hrv_value"));

        QList<QVariant> hrvValues;
        hrvValues << timestamp.toMSecsSinceEpoch() / 1000;
        hrvValues << timestamp;
        hrvValues << mHRV;

        result = con->insertRecord(&hrvFields, hrvValues);
        if (result->lastResult().isError()) {
            qDebug() << Q_FUNC_INFO << "Error inserting hrv record";
        }
    }

    tg.commit();
    return;
}

 void CommunicatorV2::saveSpo2Record()
    {
        KDbConnection *con = qobject_cast<GarminDevice *> (m_device)->database();
        if (!con)
        {
            qDebug() << Q_FUNC_INFO << "Garmin: database connection not available to save Spo2.";
        }
        qDebug() << Q_FUNC_INFO << "Garmin: saving Spo2 to database: " << mSpo2;
        QSharedPointer<KDbSqlResult> result;


        KDbTransaction transaction = con->beginTransaction();
        KDbTransactionGuard tg(transaction);

        QDateTime timestamp(QDateTime::currentDateTime());

        //sometime Spo2 value is 255, avoid adding data > 100
        if (con && con->isDatabaseUsed() && (mSpo2 <=100 )) {
            auto spoData = con->tableSchema("spo2");
            KDbFieldList spoFields;
            spoFields.addField(spoData->field("spo2_timestamp"));
            spoFields.addField(spoData->field("spo2_timestamp_dt"));
            spoFields.addField(spoData->field("spo2_automatic"));
            spoFields.addField(spoData->field("spo2_value"));

            QList<QVariant> spoValues;
            spoValues << timestamp.toMSecsSinceEpoch() / 1000;
            spoValues << timestamp;
            spoValues << true;
            spoValues << mSpo2;

            result = con->insertRecord(&spoFields, spoValues);
            if (result->lastResult().isError()) {
                qDebug() << Q_FUNC_INFO << "Error inserting Spo2 record";
            }
        }

    tg.commit();
    return;
}




// =============================================================================
// MlrServiceWriter
// =============================================================================

MlrServiceWriter::MlrServiceWriter(quint8 handle,
                                   QSharedPointer<QBLECharacteristic> sendChar)
    : m_handle(handle), m_sendChar(sendChar)
{}


void MlrServiceWriter::write(const QString& taskName, const QByteArray& data) {
    QByteArray payload;
    payload.reserve(data.size() + 1);
    payload.append(char(m_handle));
    payload.append(data);
    //return awaitBleWrite(taskName, payload);
    QString errorMsg;
    m_sendChar->writeValue(data,&errorMsg);
    qDebug() << "Garmin: Mlr Service writer wrote, error returned: " << errorMsg;
    return;
}

// =============================================================================
// Callbacks handling the actual responses based on the handle/service
// =============================================================================


GfdiServiceCallback::GfdiServiceCallback(QSharedPointer<GfdiMessageCallback> cb, CommunicatorV2* parent)
    : m_cb(std::move(cb)), mCommunicator(parent)
{
    qDebug() << "Garmin: GFDI Callback called";
}

void GfdiServiceCallback::onConnect(QSharedPointer<ServiceWriter> writer) {
    Q_UNUSED(writer);
}

void GfdiServiceCallback::onClose() {
}

void GfdiServiceCallback::onMessage(const QByteArray& data) {
    GfdiMessageParser parser;
    connect(&parser,&GfdiMessageParser::deviceInformationReceived,mCommunicator,&CommunicatorV2::onDeviceInformationReceived);
    parser.parse(data);

}


RealtimeHeartRateCallback::RealtimeHeartRateCallback(CommunicatorV2* parent) : mCommunicator(parent)
{

}
void RealtimeHeartRateCallback::onConnect(QSharedPointer<ServiceWriter> writer) { Q_UNUSED(writer);  }
void RealtimeHeartRateCallback::onClose() { }
void RealtimeHeartRateCallback::onMessage(const QByteArray& data) {
    qDebug() << Q_FUNC_INFO << "Garmin: Realtime Heart Rate Servicecallback called with data " << data;
    quint8 type = data[0]; // 0/2/3? 3 == realtime?
    quint8 hr = data[1] & 0xff;
    quint8 resting = data[2] & 0xff;
    qDebug() << "Garmin: Got realtime HR type: " << type << ", hr=" << hr << ", resting=" << resting;
    mCommunicator->setHeartRate(hr);

}

RealtimeHRVCallback::RealtimeHRVCallback(CommunicatorV2* parent) : mCommunicator(parent)
{

}
void RealtimeHRVCallback::onConnect(QSharedPointer<ServiceWriter> writer) { Q_UNUSED(writer); }
void RealtimeHRVCallback::onClose() {  }
void RealtimeHRVCallback::onMessage(const QByteArray& data) {
    qDebug() << Q_FUNC_INFO << "Garmin: Realtime HRV Servicecallback called with data " << data;
    quint16 rr = le16(data.constData());
    quint16 unk = le32(data.constData() + 2);
    qDebug() << Q_FUNC_INFO << "Garmin: realtime HRV: " << rr << ", Unknown " << unk;
    mCommunicator->setHRV(rr);

}
RealtimeSpo2Callback::RealtimeSpo2Callback(CommunicatorV2* parent) : mCommunicator(parent)
{

}
void RealtimeSpo2Callback::onConnect(QSharedPointer<ServiceWriter> writer) { Q_UNUSED(writer);   }
void RealtimeSpo2Callback::onClose() {  }
void RealtimeSpo2Callback::onMessage(const QByteArray& data) {
    qDebug() << Q_FUNC_INFO << "Garmin: Realtime  Spo2 Servicecallback called with data " << data;
    quint8 spo2 = data[0];
    quint32 ts = le32(data.constData()+1);

    ts=ts + 631065600; // Unix timestamp in seconds
    QDateTime Timestamp;

    qDebug() << Q_FUNC_INFO << "Garmin: realtime Spo2 Callback: " << spo2 << ", Timestamp " << QDateTime::fromTime_t(ts).toString();
    mCommunicator->setSpo2(spo2);

}

RealtimeStepsCallback::RealtimeStepsCallback(CommunicatorV2* parent) : mCommunicator(parent)
{

}
void RealtimeStepsCallback::onConnect(QSharedPointer<ServiceWriter> writer) { Q_UNUSED(writer); }
void RealtimeStepsCallback::onClose() {  }
void RealtimeStepsCallback::onMessage(const QByteArray& data) {
    qDebug() << Q_FUNC_INFO << "Garmin: Realtime  Steps Servicecallback called with data " << data;

    if (data.size() >= 8)
    {
        quint32 steps = le32(data.constData());
        quint32 stepsGoal = le32(data.constData()+4);
        qDebug() << Q_FUNC_INFO << "Garmin: Realtime  Steps :  " << steps << ", goal = " << stepsGoal;

        mCommunicator->setSteps(steps);
    }
}


