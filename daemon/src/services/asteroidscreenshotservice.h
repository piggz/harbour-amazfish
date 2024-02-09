#ifndef ASTEROID_SCREENSHOT_SERVICE__H
#define ASTEROID_SCREENSHOT_SERVICE__H

#include <QObject>

#include "qble/qbleservice.h"
#include "abstractdevice.h"

class AsteroidScreenshotService : public QBLEService
{
    Q_OBJECT
public:
    AsteroidScreenshotService(const QString &path, QObject *parent);

    static const char *UUID_SERVICE_SCREENSHOT;
    static const char *UUID_CHARACTERISTIC_SCREENSHOT_REQUEST;
    static const char *UUID_CHARACTERISTIC_SCREENSHOT_CONTENT;
    static const char *UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION;

    void requestScreenshot();

    Q_SIGNAL void screenshotReceived(QByteArray data);
    Q_SIGNAL void progressChanged(unsigned int progress);


private:
    void characteristicChanged(const QString &c, const QByteArray &value);

    bool m_firstNotify = false;
    bool m_downloading = false;
    QByteArray m_totalData;
    unsigned int m_progress;
    unsigned int m_totalSize;
    unsigned int m_lastProgressPct = 101;

};

#endif // ASTEROID_TIME_SERVICE__H
