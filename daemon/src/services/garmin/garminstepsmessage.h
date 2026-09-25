#ifndef GARMINSTEPSMESSAGE_H
#define GARMINSTEPSMESSAGE_H

#include <QObject>

#include "garmintypes.h"
#include "communicator_v2.h"

class GarminStepsMessage : public ServiceCallback {
    Q_OBJECT
        public:
            explicit GarminStepsMessage(CommunicatorV2* parent);

            void onConnect();
            void onClose() override;
            void onMessage(const QByteArray& data) override;

        private:
            CommunicatorV2 *mCommunicator;
};

#endif // GARMINSTEPSMESSAGE_H
