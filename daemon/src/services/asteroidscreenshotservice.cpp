#include "asteroidscreenshotservice.h"
#include "typeconversion.h"


const char* AsteroidScreenshotService::UUID_SERVICE_SCREENSHOT = "00006071-0000-0000-0000-00a57e401d05";
const char* AsteroidScreenshotService::UUID_CHARACTERISTIC_SCREENSHOT_REQUEST = "00006001-0000-0000-0000-00a57e401d05";
const char* AsteroidScreenshotService::UUID_CHARACTERISTIC_SCREENSHOT_CONTENT = "00006002-0000-0000-0000-00a57e401d05";

AsteroidScreenshotService::AsteroidScreenshotService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_SCREENSHOT, path, parent)
{
    qDebug() << Q_FUNC_INFO;
    connect(this, &QBLEService::characteristicChanged, this, &AsteroidScreenshotService::characteristicChanged);
    enableNotification(UUID_CHARACTERISTIC_SCREENSHOT_CONTENT);
    descriptorWrittenInt(UUID_CHARACTERISTIC_SCREENSHOT_CONTENT , QByteArray::fromHex("0100"));
}

void AsteroidScreenshotService::requestScreenshot()
{
    qDebug() << Q_FUNC_INFO;
    if (m_downloading) {
        return;
    }
    m_firstNotify = true;
    writeAsync(UUID_CHARACTERISTIC_SCREENSHOT_REQUEST, QByteArray(1, 0));
}

void AsteroidScreenshotService::characteristicChanged(const QString &c, const QByteArray &value)
{
    if (c == UUID_CHARACTERISTIC_SCREENSHOT_CONTENT) {

        if(m_firstNotify) {
            m_totalSize = (value[0] << 0 | value[1] << 8 | value[2] << 16 | value[3] << 24);
            m_totalData.resize(0);
            m_progress = 0;
            m_firstNotify = false;
            m_downloading = true;
        } else {
            m_progress += value.size();
            m_totalData += value;
            if(m_progress == m_totalSize) {
                qDebug() << "received";
                emit screenshotReceived(m_totalData);
                m_downloading = false;
            } else {
                qDebug() << Q_FUNC_INFO << m_progress << "/" << m_totalSize;
                emit progressChanged((m_progress * 100)/m_totalSize);
            }    
        }
    }
}