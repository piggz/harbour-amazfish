#include "garminservice.h"

const char* GarminService::BASE_UUID                   = "6A4E%04X-667B-11E3-949A-0800200C9A66";
const char* GarminService::UUID_SERVICE_GARMIN_ML_GFDI = "6A4E2800-667B-11E3-949A-0800200C9A66";

GarminService::GarminService(const QString &path, QObject *parent) : QObject()
{
    mPath = path;
    mParent = parent;
}

bool GarminService::initializeDevice( QBLEDevice builder) {
    // Iterate through the known ML characteristics until we find a known pair
    // send characteristic = read characteristic + 0x10 (eg. 2810 / 2820)
    for (int i = 0x2810; i <= 0x2814; i++) {
        /*
        characteristicReceive = mSupport.getCharacteristic(UUID.fromString(String.format(BASE_UUID, i)));
        characteristicSend = mSupport.getCharacteristic(UUID.fromString(String.format(BASE_UUID, i + 0x10)));

        if (characteristicSend != null && characteristicReceive != null) {
            LOG.debug("Using characteristics receive/send = {}/{}", characteristicReceive.getUuid(), characteristicSend.getUuid());

            //builder.notify(characteristicReceive, true);
            //builder.write(characteristicSend, closeAllServices());

            return true;
        }
        */
    }

    qDebug() << "Failed to find any known ML characteristics";

    return false;
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
