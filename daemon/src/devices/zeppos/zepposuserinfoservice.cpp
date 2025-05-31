#include "zepposuserinfoservice.h"
#include "amazfishconfig.h"
#include "typeconversion.h"

ZeppOsUserInfoService::ZeppOsUserInfoService(ZeppOSDevice *device) : AbstractZeppOsService(device, false)
{
    m_endpoint = 0x0017;
}

QString ZeppOsUserInfoService::name() const
{
    return "time";
}

void ZeppOsUserInfoService::handlePayload(const QByteArray &payload)
{
    qDebug() << Q_FUNC_INFO;

    switch (payload[0]) {
    case USER_INFO_CMD_SET_ACK:
        qDebug() << "Got user info set ack, status = " << payload.toHex();
        return;
    }

    qDebug() << "Unexpected user info payload byte:" << payload.toHex();
}

void ZeppOsUserInfoService::setUserInfo()
{
    qDebug() << Q_FUNC_INFO;

    auto config = AmazfishConfig::instance();
    auto profileName = config->profileName();
    uint id = qHash(profileName);
    auto gender = config->profileGender();
    auto height = config->profileHeight();
    auto weight = config->profileWeight();
    auto dob = config->profileDOB().date();

    QByteArray userInfo;

    qDebug() << Q_FUNC_INFO << "Setting profile" << profileName << id << gender << height << weight << dob;


    if (profileName.isEmpty() || weight == 0 || height == 0 || dob.year() == 0) {
        qDebug() << "Unable to set user info, make sure it is set up";
        return;
    }

    uint8_t genderByte = 2; // other
    if (gender == AmazfishConfig::ProfileGenderMale) {
        genderByte = 0;
    } else if (gender == AmazfishConfig::ProfileGenderFemale) {
        genderByte = 1;
    }

    userInfo += USER_INFO_CMD_SET;
    userInfo += char(0x4f);
    userInfo += char(0x07);
    userInfo += char(0x00);
    userInfo += char(0x00);
    //DOB
    userInfo += TypeConversion::fromInt16(dob.year());
    userInfo += char(dob.month());
    userInfo += char(dob.day());

    //Gender
    userInfo += char(genderByte);

    //Height
    userInfo += TypeConversion::fromInt16(height);

    //Weight
    userInfo += TypeConversion::fromInt16(weight * 200);

    //User ID
    userInfo += TypeConversion::fromInt64(id);

    //Region
    userInfo += "unknown";
    userInfo += char(0x00);
    userInfo += char(0x09); //TODO ?

    //Name
    userInfo += profileName.toLocal8Bit();
    userInfo += char(0x00);

    write(userInfo);
}
