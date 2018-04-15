#include "mibandservice.h"

const char* MiBandService::UUID_SERVICE_MIBAND = "{0000FEE0-0000-1000-8000-00805f9b34fb}";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_CONFIGURATION = "{00000003-0000-3512-2118-0009af100700}";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO = "{00000006-0000-3512-2118-0009af100700}";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_DEVICE_EVENT = "{00000010-0000-3512-2118-0009af100700}";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_NOTIFICATION = "{00000002-0000-3512-2118-0009af100700)";

MiBandService::MiBandService(QObject *parent) : BipService(UUID_SERVICE_MIBAND, parent)
{
    connect(this, &BipService::characteristicChanged, this, &MiBandService::characteristicChanged);
    //    connect(this, &BipService::readyChanged, this, &MiBandService::serviceReady);
}

void MiBandService::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qDebug() << "MiBand Changed:" << characteristic.uuid() << "(" << characteristic.name() << "):" << value;
    if (value.length() < 4) {
        return;
    }

    if (value[0] == CHAR_RESPONSE && value[1] == COMMAND_REQUEST_GPS_VERSION && value[2] == CHAR_SUCCESS) {
        m_gpsVersion = value.mid(3);
        qDebug() << "Got gps version = " << m_gpsVersion;
        emit gpsVersionChanged();
    }
}

void MiBandService::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qDebug() << "Read:" << characteristic.uuid() << "(" << characteristic.name() << "):" << value;
}

void MiBandService::requestGPSVersion()
{
    qDebug() << "Ready" << ready();
    if (ready()) {
        QLowEnergyCharacteristic characteristic = service()->characteristic(QBluetoothUuid(QString(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION)));
        service()->writeCharacteristic(characteristic, QByteArray(&COMMAND_REQUEST_GPS_VERSION, 1), QLowEnergyService::WriteWithoutResponse);
    }
}


QString MiBandService::gpsVersion()
{
    return m_gpsVersion;
}

void MiBandService::sendAlert(const QString &sender, const QString &subject, const QString &message, int category, int icon)
{
    qDebug() << "Alert:" << sender << subject << message << category << icon;

    disableNotification(UUID_CHARACTERISTIC_MIBAND_NOTIFICATION);

    QString msg;
    QByteArray utf;
    msg += sender.left(32).toLocal8Bit() + "\0  ";

    if (!subject.isEmpty()) {
        msg += subject.left(128).toLocal8Bit() + "\n\n";
    }

    if (!message.isEmpty()) {
        msg += message.left(128).toLocal8Bit();
    }

    //msg.truncate(230);

    int chunks = 1 + ((msg.length() - 1) / 18);
    utf = msg.toUtf8() + QByteArray(1, 0x00);
    for (int i = 0; i < chunks; i++) {
        QByteArray send;
        QByteArray header;
        if (i == 0) {
            header = QByteArray::fromHex("000000fa0119");
        } else if (i == chunks - 1){
            header = QByteArray(1, 0x00) + QByteArray(1, 0x80) + QByteArray(1, i);
        } else {
            header = QByteArray(1, 0x00) + QByteArray(1, 0x40) + QByteArray(1, i);
        }

        if (i == chunks - 1) { //Enable notification on the final packet
            enableNotification(UUID_CHARACTERISTIC_MIBAND_NOTIFICATION);
        }
        send = header + utf.mid(i * 18, 18);
        qDebug() << send.toHex();
        service()->writeCharacteristic(service()->characteristic(QBluetoothUuid(QString(UUID_CHARACTERISTIC_MIBAND_NOTIFICATION))), send, QLowEnergyService::WriteWithResponse);
    }
}
