#include "zepposservicesservice.h"
#include "typeconversion.h"
#include "zepposdevice.h"

ZeppOsServicesService::ZeppOsServicesService(ZeppOSDevice *device) : AbstractZeppOsService(device, false)
{
    m_endpoint = 0x0000;
}

void ZeppOsServicesService::handlePayload(const QByteArray &payload)
{
    qDebug() << Q_FUNC_INFO << payload.toHex();

    switch (payload[0]) {
    case CMD_RET_LIST:
        handleSupportedServices(payload);
        break;
    default:
        qDebug() << "Unexpected paylod";
    }
}

void ZeppOsServicesService::requestServices()
{
    QByteArray cmd;
    cmd += CMD_GET_LIST;

    m_device->writeToChunked2021(endpoint(), cmd, encrypted());
}

void ZeppOsServicesService::handleSupportedServices(QByteArray payload)
{
    qDebug() << Q_FUNC_INFO;

#if 0
    buf.get(); // discard first byte
            final short numServices = buf.getShort();

            LOG.info("Number of services: {}", numServices);

            for (int i = 0; i < numServices; i++) {
                final short endpoint = buf.getShort();
                final byte encryptedByte = buf.get();
                final Boolean encrypted = booleanFromByte(encryptedByte);

                final AbstractZeppOsService service = getSupport().getService(endpoint);

                LOG.debug(
                        "Zepp OS Service: endpoint={} encrypted={} name={}",
                        String.format("%04x", endpoint),
                        encrypted,
                        service != null ? service.getClass().getSimpleName() : "unknown"
                );

                if (service != null && encrypted != null) {
                    service.setEncrypted(encrypted);
                }

                getSupport().addSupportedService(endpoint, encrypted != null && encrypted);
            }

            getSupport().initializeServices();

            final int remainingBytes = buf.limit() - buf.position();
            if (remainingBytes != 0) {
                LOG.warn("There are {} bytes remaining in the buffer", remainingBytes);
            }
#endif

    short  numServices = TypeConversion::toUint16(payload[1], payload[2]);

    qDebug() << "Num Services: " << numServices;

    int offset = 3;
    for (int i = 0; i < numServices; i++) {
        short endpoint = TypeConversion::toUint16(payload[offset + 0], payload[offset + 1]);
        uint8_t encryptedByte = payload[offset + 2];
        offset += 3;

        bool encrypted = encryptedByte != 0x00;

        AbstractZeppOsService *service = m_device->zosService(endpoint);

        qDebug() << "Zepp OS Service: endpoint=" << endpoint << "encrypted= " << encrypted << "name=" << (service ? service->name() : "");

        if (service) {
            service->setEncrypted(encrypted);
        }

        m_device->addSupportedService(endpoint, encrypted);
    }

    m_device->ready();
}

QString ZeppOsServicesService::name() const
{
    return "ServicesService";
}
