#ifndef DFUOPERATION_H
#define DFUOPERATION_H

#include "abstractoperation.h"
#include "bipfirmwareinfo.h"

class DfuOperation : public AbstractOperation
{
public:
    DfuOperation(const AbstractFirmwareInfo *info, QBLEService *service);

    bool handleMetaData(const QByteArray &meta) override;
    void handleData(const QByteArray &data) override;
    void start() override;

protected:

    const AbstractFirmwareInfo *m_info = nullptr;
    QByteArray m_fwBytes;

    virtual bool sendFwInfo();
    virtual void sendFirmwareData();

    bool m_startWithFWInfo = true;

private:
    bool m_needToSendFwInfo = true;
};

#endif // DFUOPERATION_H
