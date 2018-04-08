#include "bipinterface.h"

BipInterface::BipInterface()
{
    m_infoService = new BipInfoService(this);

    m_genericServices.append(new BipService("{0000FEE0-0000-1000-8000-00805f9b34fb}", this));
    m_genericServices.append(new BipService("{0000FEE1-0000-1000-8000-00805f9b34fb}", this));
    m_genericServices.append(new BipService("{00001530-0000-3512-2118-0009af100700}", this));
    m_genericServices.append(new BipService("{00001800-0000-1000-8000-00805f9b34fb}", this));
    m_genericServices.append(new BipService("{00001801-0000-1000-8000-00805f9b34fb}", this));
    m_genericServices.append(new BipService("{00001802-0000-1000-8000-00805f9b34fb}", this));
    m_genericServices.append(new BipService("{0000180d-0000-1000-8000-00805f9b34fb}", this));
    m_genericServices.append(new BipService("{00001811-0000-1000-8000-00805f9b34fb}", this));
    m_genericServices.append(new BipService("{00003802-0000-1000-8000-00805f9b34fb}", this));

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
            Q_FOREACH(BipService* s, m_genericServices) {
                s->connectToService();
            }

            m_ready = true;
            emit readyChanged();

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

void BipInterface::updateServiceController()
{
    m_infoService->setController(m_controller);

    Q_FOREACH(BipService* s, m_genericServices) {
        s->setController(m_controller);
    }
}
