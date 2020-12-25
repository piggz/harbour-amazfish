#ifndef DFUWORKER_H
#define DFUWORKER_H

#include <QObject>
#include "dfuservice.h"

class DfuWorker : public QObject
{
    Q_OBJECT
public:
    explicit DfuWorker(QObject *parent = nullptr);

public slots:
    void sendFirmware(DfuService *service, const QByteArray &fwBytes, int notificationPackets);

private:
    bool m_interupted = false;

signals:
    void packetNotification();
    void done();
};

#endif // DFUWORKER_H
