#include "bipinterface.h"

BipInterface::BipInterface()
{
    m_notificationListener = new NotificationsListener(this);

    //m_genericServices.append(new BipService("{00001530-0000-3512-2118-0009af100700}", this)); //Firmware Service
    //m_genericServices.append(new BipService("{00001800-0000-1000-8000-00805f9b34fb}", this)); //Generic Access
    //m_genericServices.append(new BipService("{00001801-0000-1000-8000-00805f9b34fb}", this)); //Generic Attribute
    //m_genericServices.append(new BipService("{00001802-0000-1000-8000-00805f9b34fb}", this)); //Immediate Alert
    //m_genericServices.append(new BipService("{0000180d-0000-1000-8000-00805f9b34fb}", this)); //Heart rate service
    //m_genericServices.append(new BipService("{00003802-0000-1000-8000-00805f9b34fb}", this)); //Unknown

    //Notifications
    connect(m_notificationListener, &NotificationsListener::notificationReceived, this, &BipInterface::notificationReceived);


    // Calls
    m_voiceCallManager = new VoiceCallManager(this);
    connect(m_voiceCallManager, &VoiceCallManager::activeVoiceCallChanged, this, &BipInterface::onActiveVoiceCallChanged);
    //TODO connect(m_mibandService, &MiBandService::ignoreCall, m_voiceCallManager, &VoiceCallManager::silenceRingtone);

    m_connectionState = "disconnected";
}

void BipInterface::connectToDevice(const QString &address)
{
    m_autoReconnect = true;
    m_address = address;
    m_reconnections = 1; //Set to 1 on manual connect to enable notification
    m_bipDevice->connectToDevice(address);

    m_connectionState = "connecting";
    emit connectionStateChanged();
}

void BipInterface::disconnect()
{
    qDebug() << "Disconnecting from device";
    m_autoReconnect = false;
    m_bipDevice->disconnectFromDevice();
}

bool BipInterface::ready() const
{
    return m_ready;
}

QString BipInterface::connectionState() const
{
    return m_connectionState;
}

BipInfoService *BipInterface::infoService() const
{
    return qobject_cast<BipInfoService*>(m_bipDevice->service(BipDevice::UUID_SERVICE_DEVICEINFO));
}

MiBandService *BipInterface::miBandService() const
{
    return qobject_cast<MiBandService*>(m_bipDevice->service(BipDevice::UUID_SERVICE_MIBAND));
}

MiBand2Service *BipInterface::miBand2Service() const
{
    return qobject_cast<MiBand2Service*>(m_bipDevice->service(BipDevice::UUID_SERVICE_MIBAND2));
}

AlertNotificationService *BipInterface::alertNotificationService() const
{
    return qobject_cast<AlertNotificationService*>(m_bipDevice->service(BipDevice::UUID_SERVICE_ALERT_NOTIFICATION));
}

HRMService *BipInterface::hrmService() const
{
    return qobject_cast<HRMService*>(m_bipDevice->service(BipDevice::UUID_SERVICE_HRM));
}

void BipInterface::enableNotifications()
{
    miBand2Service()->enableNotification(MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH);
    miBandService()->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_CONFIGURATION);
    miBandService()->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO);
    miBandService()->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_DEVICE_EVENT);
    miBandService()->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_REALTIME_STEPS);
}

void BipInterface::authenticated()
{
    m_ready = true;
    emit readyChanged();

    m_connectionState = "authenticated";
    emit connectionStateChanged();

    if (!m_triedReconnect || !m_hasInitialised) {
        alertNotificationService()->sendAlert(tr("Amazfish"), tr("Connected"), tr("Phone and watch are connected"), true);
    }
    m_reconnections = 0;
    m_triedReconnect = false;
    m_hasInitialised = true;

    miBandService()->setCurrentTime();
    miBandService()->setLanguage();
    miBandService()->setDateDisplay();
    miBandService()->setTimeFormat();
    miBandService()->setUserInfo();
    miBandService()->setDisplayCaller();
    miBandService()->setRotateWristToSwitchInfo(true);
    miBandService()->setGoalNotification(true);
}

void BipInterface::notificationReceived(const QString &appName, const QString &summary, const QString &body)
{
    alertNotificationService()->sendAlert(appName, summary, body);
}

void BipInterface::onActiveVoiceCallChanged()
{

    VoiceCallHandler* handler = m_voiceCallManager->activeVoiceCall();
    if (handler) {
        connect(handler, SIGNAL(statusChanged()), SLOT(onActiveVoiceCallStatusChanged()));
        connect(handler, SIGNAL(destroyed()), SLOT(onActiveVoiceCallStatusChanged()));
        connect(miBandService(), &MiBandService::declineCall, handler, &VoiceCallHandler::hangup);

        if (handler->status()) onActiveVoiceCallStatusChanged();
    }
}

void BipInterface::onActiveVoiceCallStatusChanged()
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
