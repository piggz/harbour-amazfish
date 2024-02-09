#include "asteroidscreenshotservice.h"
#include "typeconversion.h"


const char* AsteroidScreenshotService::UUID_SERVICE_SCREENSHOT = "00006071-0000-0000-0000-00a57e401d05";
const char* AsteroidScreenshotService::UUID_CHARACTERISTIC_SCREENSHOT_REQUEST = "00006001-0000-0000-0000-00a57e401d05";
const char* AsteroidScreenshotService::UUID_CHARACTERISTIC_SCREENSHOT_CONTENT = "00006002-0000-0000-0000-00a57e401d05";
const char* AsteroidScreenshotService::UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION = "00002902-0000-1000-8000-00805f9b34fb";


AsteroidScreenshotService::AsteroidScreenshotService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_SCREENSHOT, path, parent)
{
    qDebug() << Q_FUNC_INFO;
    connect(this, &QBLEService::characteristicChanged, this, &AsteroidScreenshotService::characteristicChanged);
    enableNotification(UUID_CHARACTERISTIC_SCREENSHOT_CONTENT);
    writeDescriptorAsync(UUID_CHARACTERISTIC_SCREENSHOT_CONTENT, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION, QByteArray::fromHex("0100"));
}

void AsteroidScreenshotService::requestScreenshot()
{
    qDebug() << Q_FUNC_INFO;
    if (m_downloading) {
        return;
    }
    m_downloading = true;
    m_firstNotify = true;
    writeAsync(UUID_CHARACTERISTIC_SCREENSHOT_REQUEST, QByteArray(1, 0));
}

void AsteroidScreenshotService::characteristicChanged(const QString &c, const QByteArray &value)
{
    if (c == UUID_CHARACTERISTIC_SCREENSHOT_CONTENT) {

        if (m_firstNotify) {

            m_totalSize = TypeConversion::toUint32(value[0], value[1], value[2], value[3]);
            m_totalData = QByteArray();
            m_progress = 0;
            m_firstNotify = false;

        } else {
            m_progress += value.size();
            m_totalData += value;
            if (m_progress == m_totalSize) {
                emit screenshotReceived(m_totalData);
                m_downloading = false;
            } else {

                unsigned int progressPct = (m_progress * 100)/m_totalSize;
                if (m_lastProgressPct != progressPct) {
                    m_lastProgressPct = progressPct;
                    qDebug() << Q_FUNC_INFO << m_progress << "/" << m_totalSize << " => " << progressPct << "%";
                    emit progressChanged(progressPct);
                }

            }
        }
    }
}