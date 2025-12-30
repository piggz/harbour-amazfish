#ifndef ZEPPOSNOTIFICATIONSERVICE_H
#define ZEPPOSNOTIFICATIONSERVICE_H

#include "zeppos/abstractzepposservice.h"
#include "amazfish.h"

class ZeppOsNotificationService : public AbstractZeppOsService
{
public:
    static const uint8_t NOTIFICATION_CMD_CAPABILITIES_REQUEST = 0x01;
    static const uint8_t NOTIFICATION_CMD_CAPABILITIES_RESPONSE = 0x02;
    static const uint8_t NOTIFICATION_CMD_SEND = 0x03;
    static const uint8_t NOTIFICATION_CMD_REPLY = 0x04;
    static const uint8_t NOTIFICATION_CMD_DISMISS = 0x05;
    static const uint8_t NOTIFICATION_CMD_REPLY_ACK = 0x06;
    static const uint8_t NOTIFICATION_CMD_ICON_REQUEST = 0x10;
    static const uint8_t NOTIFICATION_CMD_ICON_REQUEST_ACK = 0x11;
    static const uint8_t NOTIFICATION_CMD_PICTURE_REQUEST = 0x19;
    static const uint8_t NOTIFICATION_CMD_PICTURE_REQUEST_ACK = 0x1a;
    static const uint8_t NOTIFICATION_TYPE_NORMAL = 0xfa;
    static const uint8_t NOTIFICATION_TYPE_CALL = 0x03;
    static const uint8_t NOTIFICATION_TYPE_SMS = 0x05;
    static const uint8_t NOTIFICATION_SUBCMD_SHOW = 0x00;
    static const uint8_t NOTIFICATION_SUBCMD_DISMISS_FROM_PHONE = 0x02;
    static const uint8_t NOTIFICATION_DISMISS_NOTIFICATION = 0x03;
    static const uint8_t NOTIFICATION_DISMISS_MUTE_CALL = 0x02;
    static const uint8_t NOTIFICATION_DISMISS_REJECT_CALL = 0x01;
    static const uint8_t NOTIFICATION_CALL_STATE_START = 0x00;
    static const uint8_t NOTIFICATION_CALL_STATE_END = 0x02;

    ZeppOsNotificationService(ZeppOSDevice *device);

    void handlePayload(const QByteArray &payload) override;
    QString name() const override;

    void sendAlert(const Amazfish::WatchNotification &notification) const;
    void incomingCall(const QString &caller);
};

#endif // ZEPPOSNOTIFICATIONSERVICE_H
