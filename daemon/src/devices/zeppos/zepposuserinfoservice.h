#ifndef ZEPPOSUSERINFOSERVICE_H
#define ZEPPOSUSERINFOSERVICE_H

#include "abstractzepposservice.h"
#include "zepposdevice.h"

class ZeppOsUserInfoService : public AbstractZeppOsService
{
public:
    static const uint8_t USER_INFO_CMD_SET = 0x01;
    static const uint8_t USER_INFO_CMD_SET_ACK = 0x02;

    ZeppOsUserInfoService(ZeppOSDevice *device);
    QString name() const override;
    void handlePayload(const QByteArray &payload) override;

    void setUserInfo();

};

#endif // ZEPPOSUSERINFOSERVICE_H
