#include "deviceinterface.h"

DeviceInterface::DeviceInterface()
{
    m_notificationListener = new NotificationsListener(this);

    m_bipDevice = new BipDevice();
    connect(m_bipDevice, &BipDevice::connectionStateChanged, this, &DeviceInterface::connectionStateChanged);
    connect(m_bipDevice, &BipDevice::message, this, &DeviceInterface::message);

    m_adapter.setAdapterPath("/org/bluez/hci0");

    //Notifications
    connect(m_notificationListener, &NotificationsListener::notificationReceived, this, &DeviceInterface::notificationReceived);

    // Calls
    m_voiceCallManager = new VoiceCallManager(this);
    connect(m_voiceCallManager, &VoiceCallManager::activeVoiceCallChanged, this, &DeviceInterface::onActiveVoiceCallChanged);
    //TODO connect(m_mibandService, &MiBandService::ignoreCall, m_voiceCallManager, &VoiceCallManager::silenceRingtone);

    emit connectionStateChanged();
}

void DeviceInterface::connectToDevice(const QString &address)
{
    qDebug() << "BipInterface::connectToDevice:" << address;

    if (m_adapter.deviceIsValid(address)) {
        m_deviceAddress = address;
        m_bipDevice->setDevicePath(address);
        m_bipDevice->connectToDevice();
    }
    else {
        qDebug() << "BipInterface::connectToDevice:device was not valid";
    }
}

void DeviceInterface::pair(const QString &address)
{
    qDebug() << "BipInterface::pair:" << address;
    m_deviceAddress = address;
    m_bipDevice->setDevicePath(address);
    m_bipDevice->pair();
}

void DeviceInterface::disconnect()
{
    qDebug() << "BipInterface::disconnect";
    m_bipDevice->disconnectFromDevice();
}

bool DeviceInterface::ready() const
{
    return m_bipDevice->connectionState() == "authenticated";
}

QString DeviceInterface::connectionState() const
{
    return m_bipDevice->connectionState();
}

DeviceInfoService *DeviceInterface::infoService() const
{
    return qobject_cast<DeviceInfoService*>(m_bipDevice->service(BipDevice::UUID_SERVICE_DEVICEINFO));
}

MiBandService *DeviceInterface::miBandService() const
{
    return qobject_cast<MiBandService*>(m_bipDevice->service(BipDevice::UUID_SERVICE_MIBAND));
}

MiBand2Service *DeviceInterface::miBand2Service() const
{
    return qobject_cast<MiBand2Service*>(m_bipDevice->service(BipDevice::UUID_SERVICE_MIBAND2));
}

AlertNotificationService *DeviceInterface::alertNotificationService() const
{
    return qobject_cast<AlertNotificationService*>(m_bipDevice->service(BipDevice::UUID_SERVICE_ALERT_NOTIFICATION));
}

HRMService *DeviceInterface::hrmService() const
{
    return qobject_cast<HRMService*>(m_bipDevice->service(BipDevice::UUID_SERVICE_HRM));
}



void DeviceInterface::notificationReceived(const QString &appName, const QString &summary, const QString &body)
{
    alertNotificationService()->sendAlert(appName, summary, body);
}

void DeviceInterface::onActiveVoiceCallChanged()
{

    VoiceCallHandler* handler = m_voiceCallManager->activeVoiceCall();
    if (handler) {
        connect(handler, SIGNAL(statusChanged()), SLOT(onActiveVoiceCallStatusChanged()));
        connect(handler, SIGNAL(destroyed()), SLOT(onActiveVoiceCallStatusChanged()));
        connect(miBandService(), &MiBandService::declineCall, handler, &VoiceCallHandler::hangup);

        if (handler->status()) onActiveVoiceCallStatusChanged();
    }
}

void DeviceInterface::onActiveVoiceCallStatusChanged()
{
    VoiceCallHandler* handler = m_voiceCallManager->activeVoiceCall();

    if (!handler || handler->handlerId().isNull()) {
        return;
    }

    switch ((VoiceCallHandler::VoiceCallStatus)handler->status()) {
    case VoiceCallHandler::STATUS_ALERTING:
    case VoiceCallHandler::STATUS_DIALING:
        qDebug() << "Tell outgoing:" << handler->lineId();
        //emit outgoingCall(handlerId, handler->lineId(), m_voiceCallManager->findPersonByNumber(handler->lineId()));
        break;
    case VoiceCallHandler::STATUS_INCOMING:
    case VoiceCallHandler::STATUS_WAITING:
        qDebug() << "Tell incoming:" << handler->lineId();
        if(handler->getState() < VoiceCallHandler::StateRinging) {
            handler->setState(VoiceCallHandler::StateRinging);
            alertNotificationService()->incomingCall(m_voiceCallManager->findPersonByNumber(handler->lineId()));
        }
        break;
    case VoiceCallHandler::STATUS_NULL:
    case VoiceCallHandler::STATUS_DISCONNECTED:
        qDebug() << "Endphone " << handler->handlerId();
        if(handler->getState() < VoiceCallHandler::StateCleanedUp) {
            handler->setState(VoiceCallHandler::StateCleanedUp);
            //emit callEnded(qHash(handler->handlerId()), false);
        }
        break;
    case VoiceCallHandler::STATUS_ACTIVE:
        qDebug() << "Startphone" << handler->handlerId();
        if(handler->getState() < VoiceCallHandler::StateAnswered) {
            handler->setState(VoiceCallHandler::StateAnswered);
            //emit callStarted(qHash(handler->handlerId()));
        }
        break;
    case VoiceCallHandler::STATUS_HELD:
        qDebug() << "OnHold" << handler->handlerId();
        break;
    }
}
