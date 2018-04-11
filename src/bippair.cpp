#include "bippair.h"
#include <QLowEnergyCharacteristic>
#include "qaesencryption.h"

BipPair::BipPair()
{
    qDebug() << "Created bip pairer";
    m_deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);

    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BipPair::foundDevice);
    connect(m_deviceDiscoveryAgent, static_cast<void (QBluetoothDeviceDiscoveryAgent::*)(QBluetoothDeviceDiscoveryAgent::Error)>(&QBluetoothDeviceDiscoveryAgent::error),
            this, &BipPair::scanError);

    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &BipPair::scanFinished);
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &BipPair::scanFinished);

//    connect(&manager, &GatoCentralManager::discoveredPeripheral, this, &BipPair::discoveredPeripheral);
}

void BipPair::startScan()
{
    m_deviceDiscoveryAgent->start();
//    manager.scanForPeripherals(GatoCentralManager::PeripheralScanOptionAllowDuplicates);
    setStatus(tr("Searching for watch"));
}
void BipPair::scanError()
{
    setStatus(tr("Scan Error"));
}

void BipPair::scanFinished()
{
    setStatus(tr("Scan Finished"));

    // Connect
    if (m_control) {
        m_control->setRemoteAddressType(QLowEnergyController::RandomAddress);
        m_control->connectToDevice();
        m_control->discoverServices();
    }
}

void BipPair::foundDevice(const QBluetoothDeviceInfo &device)
{
    qDebug() << "Found" << device.address() << device.name();
    // If device is LowEnergy-device, add it to the list
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        qDebug() << "Low Energy device found. Scanning more...";

        if (device.name() == "Amazfit Bip Watch") {
            qDebug() << "Found amazfit";
            setStatus(tr("Found amazfit watch, pairing..."));

            m_control = new QLowEnergyController(device, this);
            connect(m_control, &QLowEnergyController::serviceDiscovered,this, &BipPair::serviceDiscovered);

            connect(m_control, &QLowEnergyController::discoveryFinished,
                    this, [this] {
                qDebug() << "Service scan done.";
            });

            connect(m_control, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                    this, [this](QLowEnergyController::Error error) {
                Q_UNUSED(error);
                setStatus(tr("Cannot connect to remote device."));
            });
            connect(m_control, &QLowEnergyController::connected, this, [this]() {
                setStatus(tr("Connected to watch..."));
                m_control->discoverServices();
            });
            connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
                qDebug() << "LowEnergy controller disconnected";
            });

        }
    }
}

void BipPair::serviceDiscovered(const QBluetoothUuid &gatt)
{
    qDebug() << "Service discovered:" << gatt.toString();

    if (gatt == QBluetoothUuid(UUID_SERVICE_BIP_AUTH)) {
        qDebug() << "Creating service object";
        m_service = m_control->createServiceObject(QBluetoothUuid(UUID_SERVICE_BIP_AUTH), this);
        qDebug() << m_service->serviceName();
        if (m_service) {
            connect(m_service, &QLowEnergyService::stateChanged, this, &BipPair::serviceStateChanged);
            connect(m_service, &QLowEnergyService::characteristicChanged, this, &BipPair::authCharacteristicChanged);
            m_service->discoverDetails();
        } else {
            qDebug() << "Service not found";
        }
    }
}

void BipPair::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    switch (s) {
    case QLowEnergyService::DiscoveringServices:
        qDebug() << "Discovering services...";
        break;
    case QLowEnergyService::ServiceDiscovered:
    {
        qDebug() << "Service discovered.";

        Q_FOREACH(QLowEnergyCharacteristic c, m_service->characteristics()) {
            qDebug() << "Characteristic:" << c.uuid() << c.name();
        }

        qDebug() << "Getting the auth characteristic";
        m_authChar = m_service->characteristic(QBluetoothUuid(UUID_CHARACTERISTEC_AUTH));
        if (!m_authChar.isValid()) {
            qDebug() << "Auth service not found.";
            setStatus(tr("Unable to find auth characteristic"));
            break;
        }

        qDebug() << "Registering for notifications on the auth service";
        m_notificationDesc = m_authChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        if (m_notificationDesc.isValid()) {
            m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
        } else {
            qDebug() << "invalid descriptor";
        }
        qDebug() << "Writing special value to trigger pair dialog";
        m_service->writeCharacteristic(m_authChar, QByteArray(&AUTH_SEND_KEY, 1) + QByteArray(&AUTH_BYTE, 1) + AUTH_SECRET_KEY, QLowEnergyService::WriteWithoutResponse);

        break;
    }
    default:
        //nothing for now
        break;
    }
}

void BipPair::authCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    // ignore any other characteristic change -> shouldn't really happen though
    if (c.uuid() != QBluetoothUuid(UUID_CHARACTERISTEC_AUTH)) {
        qDebug() << "Expecting auth UUID but got" << c.uuid();
        return;
    }

    qDebug() << "Received data:" << value.size() << value.toHex();
    if (value.size() < 3) {
        return;
    }

    if (value[0] == AUTH_RESPONSE && value[1] == AUTH_SEND_KEY && value[2] == AUTH_SUCCESS) {
        qDebug() << "Received initial auth success, requesting random auth number";
        m_service->writeCharacteristic(m_authChar, QByteArray(&AUTH_REQUEST_RANDOM_AUTH_NUMBER, 1) + QByteArray(&AUTH_BYTE, 1), QLowEnergyService::WriteWithoutResponse);
    } else  if (value[0] == AUTH_RESPONSE && value[1] == AUTH_REQUEST_RANDOM_AUTH_NUMBER && value[2] == AUTH_SUCCESS) {
        qDebug() << "Received random auth number, sending encrypted auth number";
        m_service->writeCharacteristic(m_authChar, QByteArray(&AUTH_SEND_ENCRYPTED_AUTH_NUMBER, 1) + QByteArray(&AUTH_BYTE, 1) + handleAesAuth(value.mid(3, 17), AUTH_SECRET_KEY), QLowEnergyService::WriteWithoutResponse);
    } else  if (value[0] == AUTH_RESPONSE && value[1] == AUTH_SEND_ENCRYPTED_AUTH_NUMBER && value[2] == AUTH_SUCCESS) {
        setStatus("Paired");
        m_pairedAddress = m_control->remoteAddress().toString();
        m_pairedName = m_control->remoteName();
        emit pairComplete();
        m_control->disconnectFromDevice();
    } else {
        qDebug() << "Unexpected data";
    }

}

QByteArray BipPair::handleAesAuth(QByteArray data, QByteArray secretKey)
{
    return QAESEncryption::Crypt(QAESEncryption::AES_128, QAESEncryption::ECB, data, secretKey, QByteArray(), QAESEncryption::ZERO);
}

void BipPair::setStatus(const QString &s)
{
    if (s != m_status) {
        m_status = s;
        emit statusChanged();
    }
}

QString BipPair::status() const
{
    return m_status;
}

QString BipPair::watchName() const
{
    return m_pairedName;
}

QString BipPair::watchAddress() const
{
    return m_pairedAddress;
}

