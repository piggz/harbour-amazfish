#include "dfuworker.h"
#include "qdebug.h"

#include <QThread>
#include "dfuservice.h"

DfuWorker::DfuWorker(QObject *parent) : QObject(parent)
{

}

void DfuWorker::sendFirmware(DfuService *service, const QByteArray &fwBytes, int notificationPackets)
{
    int len = fwBytes.length();
    int packetLength = 20;
    int packets = len / packetLength;

    // going from 0 to len
    int firmwareProgress = 0;

    for (int i = 0; i < packets; i++) {
        QByteArray fwChunk = fwBytes.mid(i * packetLength, packetLength);

        if (QThread::currentThread()->isInterruptionRequested()) {
            qDebug() << "Interrupted!";
            m_interupted = true;
            break;
        }

        service->writeValue(DfuService::UUID_CHARACTERISTIC_DFU_PACKET, fwChunk);
        firmwareProgress += packetLength;

        if ((i > 0) && (i % notificationPackets == 0)) {
            service->setWaitForWatch(true);
            emit packetNotification();
            int waitCount = 0;
            while(service->waitForWatch() && !m_interupted) {
                QThread::msleep(50);
                waitCount++;
                if (waitCount > 100) { //5 seconds
                    qDebug() << "Timeout waiting for reply from watch!";
                    m_interupted = true;
                }
            }
        }
        if (m_interupted) {
            break;
        }
    }

    if (!m_interupted) {
        if (firmwareProgress < len) {
            QByteArray lastChunk = fwBytes.mid(packets * packetLength);
            service->writeValue(DfuService::UUID_CHARACTERISTIC_DFU_PACKET, lastChunk);
        }
        qDebug() << Q_FUNC_INFO << "Finished sending firmware";
    }
    qDebug() << Q_FUNC_INFO << "Finished worker";
    emit done();
}
