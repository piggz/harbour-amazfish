#include "communicator_v2.h"
#include "devices/garmin/garmindevice.h"
#include "cobscodec.h"      // CobsCoDec
#include "garminmlr.h"   // MlrCommunicator (Qt)
#include "garmintypes.h"
#include "garmingfdimessage.h"
#include "garminspo2message.h"
#include "garminhrmmessage.h"
#include "garminhrvmessage.h"
#include "garminstepsmessage.h"
#include "amazfishconfig.h"
#include "garmindevicestatusmessage.h"
#include "garminnotificationsubscriptionmessage.h"

#include <QtCore/QStringList>
#include <QtCore/QMetaObject>
#include <QDomDocument>
#include <QByteArray>
#include <QBuffer>
#include <QDataStream>
#include <QObject>
#include <QMutex>



const char* CommunicatorV2::BASE_UUID                   = "6a4e%1-667b-11e3-949a-0800200c9a66";
const char* CommunicatorV2::UUID_SERVICE_GARMIN_ML_GFDI = "6a4e2800-667b-11e3-949a-0800200c9a66";
const char* CommunicatorV2::UUID_SERVICE_GARMIN_V0_SEND = "df334c80-e6a7-d082-274d-78fc66f85e16";
const char* CommunicatorV2::UUID_SERVICE_GARMIN_V0_RECV = "4acbcd28-7425-868e-f447-915c8f00d0cb";
const char* CommunicatorV2::UUID_SERVICE_GARMIN_V1_SEND = "6a4e4c80-667b-11e3-949a-0800200c9a66";
const char* CommunicatorV2::UUID_SERVICE_GARMIN_V1_RECV = "6a4ecd28-667b-11e3-949a-0800200c9a66";





static inline QString fmtUuid(quint16 shortId) {
    return QStringLiteral("6A4E%1-667B-11E3-949A-0800200C9A66")
        .arg(shortId, 4, 16, QLatin1Char('0'))
        .toUpper();
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
    // add protobuf Handler
    mProtobufHandler = QSharedPointer<ProtobufHandler>(new ProtobufHandler(this));

    mState->cobsCodec=new CobsCoDec(this);

    initializeDevice();

}

void CommunicatorV2::setStatus(const QString &status)
{
    if (status != mStatus) {
        mStatus = status;
        emit statusChanged();
    }
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
            mState->UUIDReceive = serviceRec;
            mState->characteristicSend = characteristicMap.value(serviceSnd);
            mState->UUIDSend=serviceSnd;
            mIsMlProtocol = true;

        }

    }
    // No V2 protocol found, checking for other protocols

    if (characteristicMap.contains(UUID_SERVICE_GARMIN_V0_RECV) && characteristicMap.contains(UUID_SERVICE_GARMIN_V0_SEND))
    {

        mState->characteristicReceive = characteristicMap.value(UUID_SERVICE_GARMIN_V0_RECV);
        mState->UUIDReceive = UUID_SERVICE_GARMIN_V0_RECV;

        mState->characteristicSend = characteristicMap.value(UUID_SERVICE_GARMIN_V0_SEND);
        mState->UUIDSend = UUID_SERVICE_GARMIN_V0_SEND;


        if ((mState->characteristicSend != NULL) && (mState->characteristicReceive  != NULL)) {
            qDebug() << Q_FUNC_INFO << "Garmin: ML Characteristic found. Send " << UUID_SERVICE_GARMIN_V0_SEND << ", Receive " << UUID_SERVICE_GARMIN_V0_RECV;
            mIsMlProtocol = false;
        }
    }
    else if (characteristicMap.contains(UUID_SERVICE_GARMIN_V1_RECV) && characteristicMap.contains(UUID_SERVICE_GARMIN_V1_SEND))
    {

        mState->characteristicReceive = characteristicMap.value(UUID_SERVICE_GARMIN_V1_RECV);
        mState->UUIDReceive = UUID_SERVICE_GARMIN_V1_RECV;

        mState->characteristicSend = characteristicMap.value(UUID_SERVICE_GARMIN_V1_SEND);
        mState->UUIDSend = UUID_SERVICE_GARMIN_V1_RECV;


        if ((mState->characteristicSend != NULL) && (mState->characteristicReceive  != NULL)) {
            qDebug() << Q_FUNC_INFO << "Garmin: ML Characteristic found. Send " << UUID_SERVICE_GARMIN_V1_SEND << ", Receive " << UUID_SERVICE_GARMIN_V1_RECV;
            mIsMlProtocol = false;
        }
    }


    if ((mState->characteristicSend != NULL) && (mState->characteristicReceive  != NULL)) {
        qDebug() << Q_FUNC_INFO << "Garmin: ML Characteristic found. Send " << mState->UUIDSend << ", Receive " << mState->UUIDReceive;
        enableNotification(mState->UUIDReceive);
        connect(this,&QBLEService::characteristicChanged,this,&CommunicatorV2::onCharacteristicChanged);
        connect(mState->characteristicReceive.data(),&QBLECharacteristic::characteristicRead,this,&CommunicatorV2::onCharacteristicChanged);

        setStatus(QStringLiteral("GFDI ready for communication"));
        qDebug() << Q_FUNC_INFO << (QStringLiteral("Garmin: GFDI: Ready for communication"));

        if (mIsMlProtocol) {
            const QByteArray closeAll = createCloseAllServicesMessage();
            QString errorMsg;
            mState->characteristicSend->writeValue(closeAll,&errorMsg);
            if (!errorMsg.isEmpty())
            {
                qDebug() << Q_FUNC_INFO << "Garmin: closeall failed " << errorMsg;
                return false;
            }
        }

        return true;
    }

    qDebug() << Q_FUNC_INFO << "Garmin: Failed to find any known Garmin ML characteristics";
    return false;
}


void CommunicatorV2::sendRawBytes(const QString &label,const QByteArray &bytes)
{
    qDebug() <<Q_FUNC_INFO << "Garmin: Send Message " << label << " Content " << bytes.toHex();
    mSendQueue.enqueue(qMakePair(bytes, label));
    processSendQueue();
}

void CommunicatorV2::processSendQueue()
{

    if (mSendInProgress || mSendQueue.isEmpty())
    {
        return;
    }

    if (!mState->characteristicSend) {
        qWarning() << Q_FUNC_INFO <<"Garmin: DeviceConnection: no send characteristic, dropping" << mSendQueue.size() << "queued message(s)";
        mSendQueue.clear();
        return;
    }

    mSendInProgress = true;
    const QPair<QByteArray, QString> item = mSendQueue.dequeue();

    QString errorMsg;

    qDebug() <<Q_FUNC_INFO << "Garmin: Sending data to watch " <<item.second ;

    mState->characteristicSend->writeValue(item.first,&errorMsg);

    qDebug() <<Q_FUNC_INFO << "Garmin: writing result:" << errorMsg;
    if (!errorMsg.isEmpty())
    {
        qWarning() << Q_FUNC_INFO <<"Garmin: Could not send message: " << errorMsg;
    }
    mSendInProgress = false;
    if (!mSendQueue.isEmpty()) processSendQueue();

}

bool CommunicatorV2::sendMessage(const QString& taskName, const QByteArray& message) {
    qDebug() <<Q_FUNC_INFO << "Garmin: Send Message " << taskName << " Content " << message.toHex();

    const QByteArray framed = CobsCoDec::encode(message);

    // We don't negotiate a larger ATT MTU, and BlueZ's automatic long-write
    // queueing can't be relied on across devices, so fragment defensively at
    // a safe default MTU write size - same approach Gadgetbridge always
    // uses (CommunicatorV1/V2.sendMessage), regardless of whether a given
    // message happens to be small enough to not need it.

    const int handlePrefixSize = mIsMlProtocol ? 1 : 0;
    const int maxChunk = mState->maxWriteSize - handlePrefixSize;

    quint8 gfdiHandle;

    if (!mState->handleByService.contains(Service::GFDI)) {
        qDebug() << Q_FUNC_INFO << "Garmin: No GFDI handle found";
        return false;
    }

    gfdiHandle = mState->handleByService.value(Service::GFDI);

    if (mIsReliable) {
        QSharedPointer<MlrCommunicator> mlr;
        const quint8 mlrHandle = (gfdiHandle & 0x0F);
        mlr = mState->mlrCommunicators.value(mlrHandle);
        if (mlr) {
            qDebug() << Q_FUNC_INFO << "Garmin: Sending message via MLR";
            mlr->sendMessage(taskName, framed);
            return true;
        }

    }

    int offset = 0;
    int fragmentNumber = 0;
    while (offset < framed.size()) {
        const int chunkLen = qMin(maxChunk, framed.size() - offset);
        QByteArray chunk = framed.mid(offset, chunkLen);
        if (mIsMlProtocol)
            chunk.prepend(char(gfdiHandle));
        sendRawBytes(QStringLiteral("GFDI message %1 part %2").arg(taskName).arg(++fragmentNumber), chunk);
        offset += chunkLen;
    }
    return true;
}

void CommunicatorV2::onCharacteristicChanged(const QString &characteristic, const QByteArray& data) {

    //handles incoming messages for a characteristic
    qDebug() <<Q_FUNC_INFO << "Garmin: Received Message " << data.toHex();


    if (data.isEmpty()) return ;

    // handle 0 => management
    const quint8 handle = quint8(data[0]);
    const QByteArray payload = data.mid(1);
    if (handle == 0) {
        processHandleManagement(payload);
        return;
    }

    if (mIsReliable) {
        QSharedPointer<MlrCommunicator> mlr;

        if (data.size() >= 2 && (quint8(data[0]) & 0x80) != 0) {
            const quint8 handle = (quint8(data[0]) & 0x70) >> 4;
            mlr = mState->mlrCommunicators.value(handle);
        }

        if (mlr) {
            // Pass data to mlr, which will emit gfdiDecoded with the decoded data.
            QByteArray rest = data.mid(1);
            qDebug() << Q_FUNC_INFO << "Garmin: passing data to Mlr handler: " << rest.toHex();
            mlr->onPacketReceived(data); // data with handle
            return;
        }
    } else {
        // handle message directly if not mlr
        qDebug() <<Q_FUNC_INFO << "Garmin: Not an MLR Message, handling directly ";

        handleNonMlrMessage(data);
        return;
    }
}

void CommunicatorV2::onDeviceInformationReceived(DeviceInformationMessage &message)
{
    mDeviceInfo.deviceName=QString(message.deviceName);
    mDeviceInfo.deviceModel=QString(message.deviceName) +QString(" ") + QString(message.deviceModel);
    mDeviceInfo.serialNumber=QString::number(message.unitNumber);
    mDeviceInfo.softwareRevision=QString::number(message.softwareVersion);
    mDeviceInfo.deviceManufacturer="Garmin";

    //set max packet size
    onDeviceMaxPacketSize(message.maxPacketSize);

}

void CommunicatorV2::onConfigurationReceived() {
   QByteArray response = GfdiMessageGenerator::systemEvent(8, 0);
   sendMessage("SYNC READY", response);
   response = GfdiMessageGenerator::systemEvent(6, 0);
   sendMessage("HOST FOREGROUND", response);
}

void CommunicatorV2::onNotificationDataRequested(const NotificationControlMessage& msg){
    qDebug() << Q_FUNC_INFO;
    // Notification Handler needs to take care of this
    emit NotificationDataRequested(msg);
}

void CommunicatorV2::onNotificationPerformAction(const NotificationControlMessage& msg){
    qDebug() << Q_FUNC_INFO;
    // Notification Handler needs to take care of this
    emit NotificationPerformAction(msg);
}


void CommunicatorV2::onSynchronizationReceived(const SynchronizationMessage& msg) {
    qDebug() << Q_FUNC_INFO;
}

void CommunicatorV2::onFilterStatusReceived(const FilterStatusMessage& msg) {
    qDebug() << Q_FUNC_INFO;

}

void CommunicatorV2::onWeatherRequestReceived(const WeatherRequestMessage& msg) {
    qDebug() << Q_FUNC_INFO;

}


void CommunicatorV2::onProtobufMessageReceived( const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO;
    /*
    the handler elaborates the followup message but might change the status message since it does
    check the integrity of the incoming message payload. Hence we let the handlers elaborate the
    incoming message, then we send the status message of the incoming message, then the response
    and finally we send the followup.
     */
    QSharedPointer<GarminProtobufMessage> followup=QSharedPointer<GarminProtobufMessage>();
    QSharedPointer<GarminProtobufMessage> message=QSharedPointer<GarminProtobufMessage>(new GarminProtobufMessage(data, this));
    QSharedPointer<GarminProtobufMessage> parsed = message->parse();
    followup = mProtobufHandler->processIncoming(parsed);
    qDebug() << Q_FUNC_INFO << "Garmin: Sendig Ackbytestream " << parsed->getAckByteStream().toHex();
    sendMessage("SEND STATUS", parsed->getAckByteStream()); //send status message

    if (parsed->toSend()) sendMessage("SEND PROTOBUF REPLY", parsed->getMessageBytes()); //send reply if any

    if (!followup.isNull()&& followup->toSend()) sendMessage("SEND PROTOBUF FOLWOW UP", followup->getMessageBytes()); //send followup message if any

}

void CommunicatorV2::onProtobufStatusMessageReceived(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO << "Garmin: Received protobuf status message " << data.toHex();
    QSharedPointer<GarminProtobufMessage> followup=QSharedPointer<GarminProtobufMessage>();
    //QByteArray statusData=data.mid(2); // Remove Response field
    QSharedPointer<GarminProtobufStatusMessage> message=QSharedPointer<GarminProtobufStatusMessage>(new GarminProtobufStatusMessage(data, this));
    QSharedPointer<GarminProtobufStatusMessage> parsed  =  message->parse();
    qDebug() << Q_FUNC_INFO << "Garmin: Parsed Protobuf Status is " << parsed->getMessageBytes().toHex();
    followup = mProtobufHandler->processIncoming(parsed);
    qDebug() << Q_FUNC_INFO << "Garmin: protobuf ack byte stream is " << parsed->getAckByteStream().toHex();
    sendMessage("SEND STATUS", parsed->getAckByteStream()); //send status message

    if (parsed->toSend()) sendMessage("SEND PROTOBUF REPLY", parsed->getMessageBytes()); //send reply if any

    if (!followup.isNull()&& followup->toSend()) sendMessage("SEND PROTOBUF FOLWOW UP", followup->getMessageBytes()); //send followup message if any

}


void CommunicatorV2::handleNonMlrMessage(const QByteArray& data) {
    qDebug() << Q_FUNC_INFO << "Garmin: handle non MLR Message: " << data.toHex();
    if (data.size() < 2)
    {
        qDebug() << Q_FUNC_INFO << "Garmin: decoded with handle is too short!";
        return;
    }

    const quint8 handle = quint8(data[0]);
    const QByteArray payload = data.mid(1);


    Service service;

    QSharedPointer<ServiceCallback> svcCb;

    qDebug() << Q_FUNC_INFO << "Garmin: handle decoded message: Found handle " <<handle;

    service = mState->serviceByHandle.value(handle, Service::GFDI);

    switch (service) {
    case Service::GFDI:
        qDebug() << Q_FUNC_INFO << "Garmin: handle GFDI";
        mState->cobsCodec->feed(payload);
        return;
    case Service::RealtimeSpo2:
        qDebug() << Q_FUNC_INFO << "Garmin: handle Realtime Spo2";
        if (mState->serviceCallbacks.contains(Service::RealtimeSpo2))
        {
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
        qDebug() << Q_FUNC_INFO << "Garmin: Non Mlr message is not handled: " << serviceToString(service);
    }

}


void CommunicatorV2::handleIncomingGfdiMessage(const QByteArray& data)
{
    if (data.size()<8) {
        qDebug() << Q_FUNC_INFO << "Garmin: Incoming Gfdi Message is too small";
        return;
    }
    const quint16 declaredLen = le16(data);
    if (declaredLen != data.size())
    {
        qDebug() << Q_FUNC_INFO << "Garmin: Incoming Gfdi Message packet size wrong";
        return ;
    }
    const quint16 receivedCrc = le16(data.constData()+ data.size() - 2);
    const quint16 computedCrc = computeCrc16(data.mid(0,data.size()-2));
    if (receivedCrc != computedCrc)
    {
        qDebug() << Q_FUNC_INFO << "Garmin: Incoming Gfdi Message packet has wrong CRC";
        return ;
    }
    QByteArray payload = data.mid(2, data.size() - 4);

    qDebug() << Q_FUNC_INFO << "Garmin: process incoming GFDI message " << payload.toHex();
    if (mState->serviceCallbacks.contains(Service::GFDI))
        {
            mState->serviceCallbacks.value(Service::GFDI)->onMessage(payload);
        }

}
void CommunicatorV2::handleDecodedMessage(const QByteArray& decodedWithHandle) {
    // This should be a GFDI message

    qDebug() << Q_FUNC_INFO << "Garmin: handleDecodedMessage: " << decodedWithHandle.toHex();
    if (decodedWithHandle.isEmpty())
    {
        qDebug() << Q_FUNC_INFO << "Garmin: decoded with handle is empty!";
        return;
    }
    if (decodedWithHandle.size() < 6)
    {
        qDebug() << Q_FUNC_INFO << "Garmin: decoded message size too small!";
        return;
    }
    const quint16 declaredLen = le16(decodedWithHandle);
    if (declaredLen != decodedWithHandle.size())
    {
        qDebug() << Q_FUNC_INFO << "Garmin: decoded message has wrong size!";
        return ;
    }
    const quint16 receivedCrc = le16(decodedWithHandle.constData() + decodedWithHandle.size()-2);
    const quint16 computedCrc = computeCrc16(decodedWithHandle.mid(0, decodedWithHandle.size() - 2));
    if (receivedCrc != computedCrc)
    {
        qDebug() << Q_FUNC_INFO << "Garmin: decoded message has wrong crc!";
        return;
    }
    QByteArray payload = decodedWithHandle.mid(2, decodedWithHandle.size() - 6);
    qDebug() << Q_FUNC_INFO << "Garmin: handle GFDI";
    if (mState->serviceCallbacks.contains(Service::GFDI))
        {
            mState->serviceCallbacks.value(Service::GFDI)->onMessage(payload);
        }
}

void CommunicatorV2::processHandleManagement(const QByteArray& message) {
    qDebug() << Q_FUNC_INFO << "Garmin: processHandlemanagement called";
    if (message.isEmpty()) {
        return;    }

    auto requestType = requestTypeFromU8(quint8(message[0]));
    if (!requestType.ok) {
        qDebug() << Q_FUNC_INFO << "Garmin: no valid request type";
        return;
    }


    if (message.size() < 9) {
        qDebug() << Q_FUNC_INFO << "Garmin: message too small";
        return;
    }

    const quint64 clientId = u64le(message, 1);
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
        qDebug() << "Garmin: handle CloseHandleResp";
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
    qDebug() << Q_FUNC_INFO << "Garmin: MLR Registration Response: " << payload.toHex();

    if (payload.size() < 5) {
        qDebug() << Q_FUNC_INFO << "Garmin: MLR Registration Response payload too short: " << payload.size();
        return;
    }

    const quint16 serviceCodeLE = u16le(payload, 0);
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
        auto receiver = QSharedPointer<MlrMessageReceiver>::create(mMessageCallback, this);
        auto mlr = QSharedPointer<MlrCommunicator>::create(mlrHandle, 20, sender, receiver);
        //mlr->start();
        qDebug() << Q_FUNC_INFO << "Garmin: Reliable MLR communicator created";

        mState->mlrCommunicators.insert(mlrHandle, mlr);

        // Connect mlr gfdiDecoded to Message handling
        connect(receiver.data(),&MlrMessageReceiver::gfdiDecoded,this,&CommunicatorV2::handleDecodedMessage);

        createdMlr = mlr;

        mIsReliable = true;
     }

    if ((reliable==0)&&(service==Service::GFDI))
    {
        qDebug() << Q_FUNC_INFO << "Garmin: Non reliable ML communicator requested.";

        connect(mState->cobsCodec,&CobsCoDec::messageDecoded,this,&CommunicatorV2::handleIncomingGfdiMessage);
        mIsReliable = false;
    }
    // Register callback services
    if (!mState->serviceCallbacks.contains(service))
    {
        switch (service) {
        case Service::GFDI:
            qDebug() << Q_FUNC_INFO <<  "Garmin: Inserting GFDI callback handle";
            registerServiceCallback(Service::GFDI,QSharedPointer<ServiceCallback>(new GarminGfdiMessage(this)));
            // now we can continue with initialising as we have a GFDI handle to send messages
            //completePairing();
            break;
        case Service::RealtimeSpo2:
            registerServiceCallback(Service::RealtimeSpo2,QSharedPointer<ServiceCallback>(new GarminSpo2Message(this)));
            break;
        case Service::RealtimeHr:
            registerServiceCallback(Service::RealtimeHr,QSharedPointer<ServiceCallback>(new GarminHrmMessage(this)));
            break;
        case Service::RealtimeHrv:
            registerServiceCallback(Service::RealtimeHrv,QSharedPointer<ServiceCallback>(new GarminHrmMessage(this)));
            break;
        case Service::RealtimeSteps:
            registerServiceCallback(Service::RealtimeSteps,QSharedPointer<ServiceCallback>(new GarminStepsMessage(this)));
            break;
        default:
            // Create Default Callback
            //mState->serviceCallbacks.insert(service, QSharedPointer<ServiceCallback>(new DefaultCallback()));
            break;
        }
    }

    emit mlrConnected();
    return;
}

void CommunicatorV2::processCloseHandleResp(const QByteArray& payload) {
    if (payload.size() < 4) {
        qDebug() << Q_FUNC_INFO << "CloseHandleResp payload too short";
        return;
    }

    const quint16 serviceCodeLE = u16le(payload, 0);
    const quint8 handle = quint8(payload[2]);

    auto svc = serviceFromCode(serviceCodeLE);
    if (!svc.ok)
        return;
    const Service service = svc.value;

    QSharedPointer<ServiceCallback> cb;
    {
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

    registerServices();


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
        qDebug() << Q_FUNC_INFO << "Garmin: could not register Service " << serviceToString(service) << errorMsg;
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
    if (isFirstConnect || (qobject_cast<GarminDevice *>(m_device))->isPairing()) {
        qDebug()<< Q_FUNC_INFO << "Garmin: Doing first connection";

        QByteArray msg;
        //msg = GfdiMessageGenerator::supportedFileTypesRequest();
        //if (!sendMessage("SUPPORTED FILE TYPE REQUEST",msg)) return false;

        msg = GfdiMessageGenerator::deviceSettings();
        if (!sendMessage("DEVICE_SETTINGS",msg)) return false;

        // Time Synchronization should come now
        msg = GfdiMessageGenerator::systemEvent(16,0);
        if (!sendMessage("TIME_UPDATED",msg)) return false;

        msg = GfdiMessageGenerator::systemEvent(8,0);
        if (!sendMessage("SYNC_READY",msg)) return false;

        // The next messages should only be sent during pairing, not when connecting to the device

        if ((qobject_cast<GarminDevice *>(m_device))->isPairing())
        {
            msg = GfdiMessageGenerator::systemEvent(4,0);
            if (!sendMessage("PAIR_COMPLETE",msg)) return false;

            msg = GfdiMessageGenerator::systemEvent(0,0);
            if (!sendMessage("SYNC_COMPLETE",msg)) return false;

            msg = GfdiMessageGenerator::systemEvent(14,0);
            if (!sendMessage("SETUP_WIZARD_COMPLETE",msg)) return false;

            (qobject_cast<GarminDevice *>(m_device))->setPaired();
        }

        isFirstConnect=false;
    }
    qDebug()<< Q_FUNC_INFO << "Garmin: First connection already done";

    return true;
}

void CommunicatorV2::registerServices() {
    qDebug() << "Garmin: handle service registration";
    if (!mState->characteristicSend)
    {
        qDebug() << "Garmin: Can't register services due to missing send characteristic";
    }
    if (!m_device)
    {
        qDebug() << "Garmin: No device configured";
    }

    QString errorMsg;


    registerService(Service::GFDI, false); // reliable implemantation is broken, use unreliable for now

    if ((qobject_cast<GarminDevice *>(m_device))->supportedFeatures() &  Amazfish::Feature::FEATURE_SPO2 )
        registerService(Service::RealtimeSpo2, true);
    if ((qobject_cast<GarminDevice *>(m_device))->supportedFeatures() &  Amazfish::Feature::FEATURE_HRM )
        registerService(Service::RealtimeHr, true);
    if ((qobject_cast<GarminDevice *>(m_device))->supportedFeatures() &  Amazfish::Feature::FEATURE_STEPS )
        registerService(Service::RealtimeSteps, true);

    //HRV is not shown in the GUI so no realtime info needed
    //registerService(Service::RealtimeHrv, true);

    // Send Battery Status request
    mBatteryTimer = new QTimer(this);
    mBatteryTimer->setInterval(120000);
    mBatteryTimer->start();
    connect(mBatteryTimer,&QTimer::timeout, this, &CommunicatorV2::getBatteryLevel);
    getBatteryLevel();
    mServicesResolved=true;
}

void CommunicatorV2::getBatteryLevel() {
    GarminDeviceStatusMessage* msg = new GarminDeviceStatusMessage(this);
    QSharedPointer<GarminProtobufMessage> batteryRequest = mProtobufHandler->prepareProtobufRequest(msg->generateBatteryStatusRequest());
    sendMessage("BATTERY STATUS REQUEST",batteryRequest->getOutgoingMessage());
}

void CommunicatorV2::dispose() {
    mState->mlrCommunicators.clear();
    if (mState->characteristicReceive) {
        mState->characteristicReceive->stopNotify();
        disconnect(mState->characteristicReceive.data(),nullptr,this,nullptr);
    }


    if (m_device)
        disconnect((qobject_cast<GarminDevice *>(m_device)), nullptr, this, nullptr);

    mState->characteristicSend.reset();
    mState->characteristicReceive.reset();
    //m_gfdiService.reset();
    mState->cobsCodec->reset();
    mIsMlProtocol = false;
    mIsReliable = false;

    mSendQueue.clear();
    mSendInProgress = false;
    mState->mlrCommunicators.clear();
    mState->serviceByHandle.clear();
    mState->serviceCallbacks.clear();

    mConnected = false;
    mServicesResolved = false;

    if (!isFirstConnect) {
        isFirstConnect = true;
    }

    setStatus(QStringLiteral("Idle"));
}

void CommunicatorV2::onConnectionStateChange(bool connected) {
    qDebug() << Q_FUNC_INFO << connected;
    if (connected==mConnected) return;
    if (connected)
    {
        isFirstConnect=true;
        initializeDevice();
    }
    else {
        dispose();
    }
    mConnected=connected;

}


QByteArray CommunicatorV2::createCloseAllServicesMessage() const {
    QByteArray b;
    b.append(char(0));
    b.append(char(quint8(RequestType::CloseAllReq)));
    writeU64le(b, AMAZFISH_CLIENT_ID);
    writeU16le(b, 0);
    b.append(char(0));
    return b;
}

QByteArray CommunicatorV2::createRegisterServiceMessage(Service service, bool reliable) const {
    QByteArray b;
    b.append(char(0));
    b.append(char(quint8(RequestType::RegisterMlReq)));
    writeU64le(b, AMAZFISH_CLIENT_ID);
    writeU16le(b, serviceCode(service));
    b.append(char(reliable ? 2 : 0));
    return b;
}

QByteArray CommunicatorV2::createCloseServiceMessage(Service service, quint8 handle) const {
    QByteArray b;
    b.reserve(12);
    b.append(char(0));
    b.append(char(quint8(RequestType::CloseHandleReq)));
    writeU64le(b, AMAZFISH_CLIENT_ID);
    writeU16le(b, serviceCode(service));
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
    //qDebug() << Q_FUNC_INFO << "Garmin: setting steps goal to " << val;
    int configuredGoal=AmazfishConfig::instance()->profileFitnessGoal();
    if((val != mStepsGoal) || (val != configuredGoal))
    {
        qDebug() << Q_FUNC_INFO << "Garmin: setting steps goal to value from watch";
        mStepsGoal=val;

        AmazfishConfig::instance()->setProfileFitnessGoal(val);
    }
}

void CommunicatorV2::setHeartRate(quint8 val)
{
    //qDebug() << Q_FUNC_INFO << "Garmin: setting Heart Rate to " << val;
    mHeartRate =val;
    emit informationChanged(Amazfish::Info::INFO_HEARTRATE, QString::number(val));
}

void CommunicatorV2::setHRV(quint16 val)
{
    //qDebug() << Q_FUNC_INFO << "Garmin: setting HRV to " << val;
    mHRV =val;
    // Future use if needed - saving HRV
    //saveHRVRecord();

}

void CommunicatorV2::setSpo2(quint8 val)
{
    //qDebug() << Q_FUNC_INFO << "Garmin: setting Spo2 to " << val;
    mSpo2 =val;
    emit informationChanged(Amazfish::Info::INFO_SPO2, QString::number(val));
    // Future use if needed - save Spo2
    //saveSpo2Record();
}

void CommunicatorV2::setBatteryLevel(quint8 val)
{
    //qDebug() << Q_FUNC_INFO << "Garmin: setting Spo2 to " << val;
    if (mBatteryLevel != val)
    {
        mBatteryLevel =val;
        emit informationChanged(Amazfish::Info::INFO_BATTERY, QString::number(val));
    }
}

void CommunicatorV2::saveHRVRecord()
{
    // This will probably be never used as the proper way is downloading activities
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
        // This will probably be never used as the proper way is downloading activities

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






