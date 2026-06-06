#ifndef GARMINHRVMESSAGE_H
#define GARMINHRVMESSAGE_H

#include <QObject>

#include "garmintypes.h"
#include "communicator_v2.h"

class GarminHrvMessage : public ServiceCallback {
    Q_OBJECT
        public:
            explicit GarminHrvMessage(CommunicatorV2* parent);

            void onConnect();
            void onClose() override;
            void onMessage(const QByteArray& data) override;

        private:
            CommunicatorV2 *mCommunicator;
};

#endif // GARMINHRVMESSAGE_H
