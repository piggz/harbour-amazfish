#include "asteroidtimeservice.h"
#include "typeconversion.h"


const char* AsteroidTimeService::UUID_SERVICE_ASTEROID_TIME = "00005071-0000-0000-0000-00a57e401d05";
const char* AsteroidTimeService::UUID_CHARACTERISTIC_ASTEROID_TIME_SET = "00005001-0000-0000-0000-00a57e401d05";

AsteroidTimeService::AsteroidTimeService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_ASTEROID_TIME, path, parent)
{
    qDebug() << Q_FUNC_INFO;
}

void AsteroidTimeService::setCurrentTime()
{

    QDateTime t = QDateTime::currentDateTime();

    QByteArray data(6, 0);
    data[0] = t.date().year() - 1900;
    data[1] = t.date().month()-1;
    data[2] = t.date().day();
    data[3] = t.time().hour();
    data[4] = t.time().minute();
    data[5] = t.time().second();


    qDebug() << Q_FUNC_INFO << "Setting time to:" << t << data.toHex();
    writeValue(UUID_CHARACTERISTIC_ASTEROID_TIME_SET, data);

}
