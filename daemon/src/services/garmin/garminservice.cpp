#include "garminservice.h"
#include "devices/garmin/garmindevice.h"

#include <QDomDocument>
#include <QByteArray>
#include <QBuffer>
#include <QDataStream>

//const char* GarminService::BASE_UUID                   = "6a4e%04X-667b-11e3-949a-0800200c9a66";
const char* GarminService::BASE_UUID                   = "6a4e%1-667b-11e3-949a-0800200c9a66";
const char* GarminService::UUID_SERVICE_GARMIN_ML_GFDI = "6a4e2800-667b-11e3-949a-0800200c9a66";

GarminService::GarminService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_GARMIN_ML_GFDI, path, parent)
{
    qDebug() << "Garmin Service created for " << path;
    mPath = path;
    mDevice = parent;

    connect(this, &QBLEService::characteristicRead, this, &GarminService::characteristicRead);
    //enableNotification();

    initializeDevice();
}

bool GarminService::initializeDevice() {
    // Iterate through the known ML characteristics until we find a known pair
    // send characteristic = read characteristic + 0x10 (eg. 2810 / 2820)
    //QString servicePath = (dynamic_cast<QBLEDevice* > (mDevice))->devicePath();
    qDebug() << "Garmin Service path " << mPath;

    QMap<QString, QBLECharacteristic*> characteristicMap;

    QDBusInterface miIntro("org.bluez", mPath, "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus(), 0);
    QDBusReply<QString> xml = miIntro.call("Introspect");
    QDomDocument doc;

    doc.setContent(xml.value());

    QDomNodeList nodes = doc.elementsByTagName("node");

    for (int x = 0; x < nodes.count(); x++)
    {
        QDomElement node = nodes.at(x).toElement();
        QString nodeName = node.attribute("name");

        if (nodeName.startsWith("char")) {
            QString path = mPath + "/" + nodeName;
            QDBusInterface charInterface("org.bluez", path, "org.bluez.GattCharacteristic1", QDBusConnection::systemBus(), 0);
            characteristicMap[charInterface.property("UUID").toString()] = new QBLECharacteristic(path, this);
        }
    }

    qDebug() << "Garmin characteristics" << characteristicMap.keys();

    for (int i = 2810; i <= 2814; i++) {
        QString serviceRec = QString(GarminService::BASE_UUID).arg(i);
        QString serviceSnd = QString(GarminService::BASE_UUID).arg(i+10);
        qDebug() << "Checking for Garmin    UUID " << serviceRec <<" and " << serviceSnd;

        if (characteristicMap.contains(serviceRec) && characteristicMap.contains(serviceSnd))
        {
            mCharacteristicSend = characteristicMap.value(serviceRec);
            mCharacteristicReceive = characteristicMap.value(serviceSnd);
            qDebug() << " Garmin ML Characteristic found";
        }

        /*
        for (auto c=characteristicMap.begin();c!=characteristicMap.end();++c)
        {
            qDebug() << "Checking against " << c.key();
            if ((c.key()==serviceRec) && (mCharacteristicReceive == NULL))
            {
                    mCharacteristicReceive = c.value();
                    qDebug() << "Garmin receive characteristic found";
            }
            if ((c.key()==serviceSnd) && (mCharacteristicSend == NULL))
            {
                    mCharacteristicSend = c.value();
                    qDebug() << "Garmin send characteristic found";
            }
        }*/

        if ((mCharacteristicSend != NULL) && (mCharacteristicReceive != NULL)) {

            //builder.notify(characteristicReceive, true);
            mCharacteristicSend->writeAsync(closeAllServices());

            return true;
        }

    }

    qDebug() << "Failed to find any known Garmin ML characteristics";

    return false;
}

void GarminService::characteristicRead(const QString &c, const QByteArray &value)
{

        qDebug() << Q_FUNC_INFO << c << "=================" << value;
        qDebug() << "Charateristics Read called";

}


QByteArray GarminService::closeAllServices() {
    QDataStream send;
    send.setByteOrder(QDataStream::LittleEndian);
    send << (qint8)0;
    send << RequestType::CLOSE_ALL_REQ;
    send << (quint64) 2;
    send << (quint16) 0;
    //toSend.write((Byte) 0); // handle
    //toSend.put((byte) RequestType.CLOSE_ALL_REQ.ordinal());
    //toSend.putLong(GADGETBRIDGE_CLIENT_ID);
    //toSend.putShort((short) 0);
    QByteArray res;
    qDebug() << "Garmin cose all services string " << send;
    send >> res;
    return res;
}

void GarminService::onMtuChanged(int mtu) {
    mMaxWriteSize = calcMaxWriteChunk(mtu);
/*
    for (auto it = mMlrServices.begin(); it != mMlrServices.end(); ++it) {
        it->setMaxPacketSize(mMaxWriteSize);
    }
    */
}

int GarminService::calcMaxWriteChunk(int mtu) {
    // the minimum MTU is 23 (Bluetooth spec)
    int safeMtu = std::max(23, mtu);

    // GATT_MAX_ATTR_LEN: no larger than 512 (Bluetooth spec)
    // MTU: overhead of simple write must be supported. Some other operations like
    //      ATT_PREPARE_WRITE_REQ have even larger overhead so the max BLE MTU is larger than 512+3
    return std::min(512, safeMtu - 3);
}
