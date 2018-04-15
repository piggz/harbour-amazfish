#include "bipinterface.h"

BipInterface::BipInterface()
{
    m_infoService = new BipInfoService(this);
    m_mibandService = new MiBandService(this);
    m_miband2Service = new MiBand2Service(this);
    m_alertNotificationService = new AlertNotificationService(this);

    m_notificationListener = new NotificationsListener(this);

    //m_genericServices.append(new BipService("{00001530-0000-3512-2118-0009af100700}", this)); //Firmware Service
    //m_genericServices.append(new BipService("{00001800-0000-1000-8000-00805f9b34fb}", this)); //Generic Access
    //m_genericServices.append(new BipService("{00001801-0000-1000-8000-00805f9b34fb}", this)); //Generic Attribute
    //m_genericServices.append(new BipService("{00001802-0000-1000-8000-00805f9b34fb}", this)); //Immediate Alert
    //m_genericServices.append(new BipService("{0000180d-0000-1000-8000-00805f9b34fb}", this)); //Heart rate service
    //m_genericServices.append(new BipService("{00003802-0000-1000-8000-00805f9b34fb}", this)); //Unknown

    connect(m_infoService, &BipService::readyChanged, this, &BipInterface::serviceReady);
    connect(m_mibandService, &BipService::readyChanged, this, &BipInterface::serviceReady);
    connect(m_miband2Service, &BipService::readyChanged, this, &BipInterface::serviceReady);
    connect(m_alertNotificationService, &BipService::readyChanged, this, &BipInterface::serviceReady);
    connect(m_miband2Service, &MiBand2Service::authenticated, this, &BipInterface::authenticated);

    connect(m_notificationListener, &NotificationsListener::notificationReceived, this, &BipInterface::notificationReceived);

    m_connectionState = "disconnected";
}

void BipInterface::connectToDevice(const QString &address)
{
    if (!m_controller) {
        m_address = address;
        m_controller = new QLowEnergyController(QBluetoothAddress(address));
        m_controller->setRemoteAddressType(QLowEnergyController::RandomAddress);

        updateServiceController();

        connect(m_controller, &QLowEnergyController::discoveryFinished,
                this, [this] {

            qDebug() << "Discovered services:";
            Q_FOREACH(QBluetoothUuid s, m_controller->services()) {
                qDebug() << "Service:" << s.toString();
            }

            m_infoService->connectToService();
            m_mibandService->connectToService();
            m_miband2Service->connectToService();
            m_alertNotificationService->connectToService();

            Q_FOREACH(BipService* s, m_genericServices) {
                s->connectToService();
            }
        });

        connect(m_controller, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
            Q_UNUSED(error);
        });
        connect(m_controller, &QLowEnergyController::connected, this, [this]() {
            m_connectionState = "connected";
            emit connectionStateChanged();

            m_controller->discoverServices();
        });
        connect(m_controller, &QLowEnergyController::disconnected, this, [this]() {
            qDebug() << "LowEnergy controller disconnected";
            m_connectionState = "disconnected";
            emit connectionStateChanged();
            m_ready = false;
            emit readyChanged();
        });
    }
    m_controller->connectToDevice();
    m_connectionState = "connecting";
    emit connectionStateChanged();
}

void BipInterface::disconnect()
{
    if (m_controller) {
        m_controller->disconnectFromDevice();
    }
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
    return m_infoService;
}

MiBandService *BipInterface::miBandService() const
{
    return m_mibandService;
}

MiBand2Service *BipInterface::miBand2Service() const
{
    return m_miband2Service;
}

AlertNotificationService *BipInterface::alertNotificationService() const
{
    return m_alertNotificationService;
}

void BipInterface::updateServiceController()
{
    m_infoService->setController(m_controller);
    m_mibandService->setController(m_controller);
    m_miband2Service->setController(m_controller);
    m_alertNotificationService->setController(m_controller);

    Q_FOREACH(BipService* s, m_genericServices) {
        s->setController(m_controller);
    }
}

void BipInterface::enableNotifications()
{
    m_miband2Service->enableNotification(MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH);
    m_mibandService->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_CONFIGURATION);
    m_mibandService->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO);
    m_mibandService->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_DEVICE_EVENT);
}

void BipInterface::serviceReady(bool r)
{
    bool ready = m_infoService->ready() && m_mibandService->ready() && m_miband2Service->ready() && m_alertNotificationService->ready();
    qDebug() << "A service is ready" << r << ready;

    if (ready) {
        qDebug() << "All services ready, initialising...";
        enableNotifications();
        m_miband2Service->initialise();
    }
}

void BipInterface::authenticated()
{
    m_ready = true;
    emit readyChanged();
}

void BipInterface::notificationReceived(const QString &appName, const QString &summary, const QString &body)
{
    m_alertNotificationService->sendAlert(appName, summary, body, AlertNotificationService::CustomHuami, AlertNotificationService::APP_11);
}
