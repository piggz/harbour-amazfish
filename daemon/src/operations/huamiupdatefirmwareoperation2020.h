#ifndef HUAMIUPDATEFIRMWAREOPERATION2020_H
#define HUAMIUPDATEFIRMWAREOPERATION2020_H

#include "updatefirmwareoperation.h"

class HuamiUpdateFirmwareOperation2020 : public UpdateFirmwareOperation
{
public:
    HuamiUpdateFirmwareOperation2020(const AbstractFirmwareInfo *info, QBLEService *service, QBLEService &mibandService);

    bool handleMetaData(const QByteArray &meta) override;
    void start() override;

protected:
    bool sendFwInfo() override;

private:
    static constexpr uint8_t COMMAND_REQUEST_PARAMETERS = 0xd0;
    static constexpr uint8_t COMMAND_START_FILE = 0xd1;
    static constexpr uint8_t COMMAND_SEND_FIRMWARE_INFO = 0xd2;
    static constexpr uint8_t COMMAND_START_TRANSFER = 0xd3;
    static constexpr uint8_t REPLY_UPDATE_PROGRESS = 0xd4;
    static constexpr uint8_t COMMAND_COMPLETE_TRANSFER = 0xd5;
    static constexpr uint8_t COMMAND_FINALIZE_UPDATE = 0xd6;

    bool requestParameters();
    bool sendFirmwareDataChunk(int offset);
    void sendTransferStart();
    void sendTransferComplete();
    void sendFinalize();

    QBLEService &m_mibandService;

    int mChunkLength = -1;
};

#endif // HUAMIUPDATEFIRMWAREOPERATION2020_H
