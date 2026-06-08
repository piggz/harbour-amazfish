#ifndef GARMINCONFIGURATIONMESSAGE_H
#define GARMINCONFIGURATIONMESSAGE_H

#include "garmingfdimessage.h"
#include "communicator_v2.h"

#include <QObject>

// Configuration Message (incoming from watch)
struct ConfigurationMessage {
    QSet<quint16> capabilities;
};

class GarminConfigurationMessage : public GarminGfdiMessage
{
    Q_OBJECT
public:
    GarminConfigurationMessage(CommunicatorV2 *parent) {
        mCommunicator = parent;
    }
    void parse(const QByteArray& data);
    QByteArray generateResponse(const QByteArray& data);
private:
    QSet<quint16> parseCapabilities(const QByteArray& bytes);
    QByteArray generateCapabilities();
    QByteArray setDeviceSettings(bool autoUpload, bool weatherConditions, bool weatherAlerts);
};
#endif // GARMINCONFIGURATIONMESSAGE_H
