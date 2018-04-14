#include "bipinterface.h"

BipInterface::BipInterface()
{
    m_infoService = new BipInfoService(this);
    m_mibandService = new MiBandService(this);
    m_miband2Service = new MiBand2Service(this);

    //m_genericServices.append(new BipService("{00001530-0000-3512-2118-0009af100700}", this)); //Firmware Service
    //m_genericServices.append(new BipService("{00001800-0000-1000-8000-00805f9b34fb}", this)); //Generic Access
    //m_genericServices.append(new BipService("{00001801-0000-1000-8000-00805f9b34fb}", this)); //Generic Attribute
    //m_genericServices.append(new BipService("{00001802-0000-1000-8000-00805f9b34fb}", this)); //Immediate Alert
    //m_genericServices.append(new BipService("{0000180d-0000-1000-8000-00805f9b34fb}", this)); //Heart rate service
    //m_genericServices.append(new BipService("{00001811-0000-1000-8000-00805f9b34fb}", this)); //Alert notification service
    //m_genericServices.append(new BipService("{00003802-0000-1000-8000-00805f9b34fb}", this)); //Unknown

    connect(m_infoService, &BipService::readyChanged, this, &BipInterface::serviceReady);
    connect(m_mibandService, &BipService::readyChanged, this, &BipInterface::serviceReady);
    connect(m_miband2Service, &BipService::readyChanged, this, &BipInterface::serviceReady);

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

void BipInterface::updateServiceController()
{
    m_infoService->setController(m_controller);
    m_mibandService->setController(m_controller);
    m_miband2Service->setController(m_controller);

    Q_FOREACH(BipService* s, m_genericServices) {
        s->setController(m_controller);
    }
}

void BipInterface::enableNotifications()
{
    m_miband2Service->enableNotification(MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH);
}

void BipInterface::serviceReady(bool r)
{
    bool ready = m_infoService->ready() && m_mibandService->ready() && m_miband2Service->ready();
    qDebug() << "A service is ready" << r << ready;

    if (ready != m_ready) {
        m_ready = ready;

        if (m_ready) {
            qDebug() << "All services ready, initialising...";
            enableNotifications();
            m_miband2Service->initialise();
        }
        emit readyChanged();
    }

}
