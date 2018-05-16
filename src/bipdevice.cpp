#include "bipdevice.h"

BipDevice::BipDevice()
{
    m_infoService = new BipInfoService(this);
    m_mibandService = new MiBandService(this);
    m_miband2Service = new MiBand2Service(this);
    m_alertNotificationService = new AlertNotificationService(this);
    m_hrmService = new HRMService(this);

    connect(m_miband2Service, &MiBand2Service::authenticated, this, &BipInterface::authenticated);

}
