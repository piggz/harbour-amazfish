#ifndef GARMINHRMMESSAGE_H
#define GARMINHRMMESSAGE_H

#include <QObject>

#include "garmintypes.h"
#include "communicator_v2.h"

class GarminHrmMessage : public ServiceCallback {
    Q_OBJECT
        public:
            explicit GarminHrmMessage(CommunicatorV2* parent);

            void onConnect();
            void onClose() override;
            void onMessage(const QByteArray& data) override;

        private:
            CommunicatorV2 *mCommunicator;
};

#endif // GARMINHRMMESSAGE_H
