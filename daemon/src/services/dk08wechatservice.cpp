#include "dk08wechatservice.h"
#include "typeconversion.h"

const char* DK08WechatService::UUID_SERVICE_WECHAT = "0000fee7-0000-1000-8000-00805f9b34fb";
const char* DK08WechatService::UUID_CHARACTERISTIC_WECHAT_WRITE = "0000fec7-0000-1000-8000-00805f9b34fb";
const char* DK08WechatService::UUID_CHARACTERISTIC_WECHAT_INCICATE = "0000fec8-0000-1000-8000-00805f9b34fb";
const char* DK08WechatService::UUID_CHARACTERISTIC_WECHAT_READ = "0000fec9-0000-1000-8000-00805f9b34fb";
const char* DK08WechatService::UUID_CHARACTERISTIC_WECHAT_PEDOMETER = "0000fea1-0000-1000-8000-00805f9b34fb";
const char* DK08WechatService::UUID_CHARACTERISTIC_WECHAT_TARGET = "0000fea2-0000-1000-8000-00805f9b34fb";

DK08WechatService::DK08WechatService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_WECHAT, path, parent)
{
    qDebug() << Q_FUNC_INFO;

    connect(this, &QBLEService::characteristicRead, this, &DK08WechatService::characteristicRead);

    readAsync(UUID_CHARACTERISTIC_WECHAT_PEDOMETER);

    // enableNotification(UUID_CHARACTERISTIC_WECHAT_INCICATE);
    // enableNotification(UUID_CHARACTERISTIC_WECHAT_READ);
    enableNotification(UUID_CHARACTERISTIC_WECHAT_PEDOMETER);
    // enableNotification(UUID_CHARACTERISTIC_WECHAT_TARGET);
    writeAsync(UUID_CHARACTERISTIC_WECHAT_INCICATE, QByteArray(1, 0x01));
    // writeDescriptorAsync(UUID_CHARACTERISTIC_SCREENSHOT_CONTENT, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION, QByteArray::fromHex("0100"));
}

void DK08WechatService::characteristicRead(const QString &characteristic, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << "Read:" << characteristic << value.toHex();
    if (characteristic == UUID_CHARACTERISTIC_WECHAT_PEDOMETER) {
        if (value.length() == 4) {
            m_steps = TypeConversion::toUint32(value[1], value[2], value[3], 0);
            emit informationChanged(AbstractDevice::INFO_STEPS, QString::number(m_steps, 10));
        }
    }

}

void DK08WechatService::refreshInformation() {
    readAsync(UUID_CHARACTERISTIC_WECHAT_PEDOMETER);
}
