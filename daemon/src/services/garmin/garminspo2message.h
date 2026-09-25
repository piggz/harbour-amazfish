#ifndef GARMINSPO2MESSAGE_H
#define GARMINSPO2MESSAGE_H

#include <QObject>

#include "garmintypes.h"
#include "communicator_v2.h"
//class CommunicatorV2;

class GarminSpo2Message : public ServiceCallback {
    Q_OBJECT
        public:
            explicit GarminSpo2Message(CommunicatorV2* parent);

            void onConnect();
            void onClose() override;
            void onMessage(const QByteArray& data) override;

        private:
            CommunicatorV2 *mCommunicator;
};

#endif // GARMINSPO2MESSAGE_H
