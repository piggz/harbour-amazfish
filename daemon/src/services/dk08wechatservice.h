#ifndef DK08WECHATSERVICE_H
#define DK08WECHATSERVICE_H

#include "qble/qbleservice.h"
#include "devices/abstractdevice.h"


class DK08WechatService : public QBLEService
{
    Q_OBJECT
public:
    DK08WechatService(const QString &path, QObject *parent);

    static const char* UUID_SERVICE_WECHAT;
    static const char* UUID_CHARACTERISTIC_WECHAT_WRITE;
    static const char* UUID_CHARACTERISTIC_WECHAT_INCICATE;
    static const char* UUID_CHARACTERISTIC_WECHAT_READ;
    static const char* UUID_CHARACTERISTIC_WECHAT_PEDOMETER;
    static const char* UUID_CHARACTERISTIC_WECHAT_TARGET;

    Q_INVOKABLE void refreshInformation();

    Q_SIGNAL void informationChanged(Amazfish::Info key, const QString &val);

    int steps() { return m_steps; }

private:
    int m_steps = 0;

    Q_SLOT void characteristicRead(const QString &c, const QByteArray &value);
};

#endif // DK08WECHATSERVICE_H
