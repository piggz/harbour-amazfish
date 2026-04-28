#ifndef GARMINSERVICE__H
#define GARMINSERVICE__H

//#include "abstractoperationservice.h"
#include "garminmlrservice.h"
#include <qmap.h>
#include <qbledevice.h>

#include <optional>
#include <vector>

class GarminService : public QObject
{
    class Service {
    public:
        enum Value {
            GFDI = 1,
            REGISTRATION = 4,
            REALTIME_HR = 6,
            REALTIME_STEPS = 7,
            REALTIME_CALORIES = 8,
            REALTIME_INTENSITY = 10,
            REALTIME_HRV = 12,
            REALTIME_STRESS = 13,
            REALTIME_ACCELEROMETER = 16,
            REALTIME_SPO2 = 19,
            REALTIME_BODY_BATTERY = 20,
            REALTIME_RESPIRATION = 21,
            FILE_TRANSFER_2 = 0x2018,
            FILE_TRANSFER_4 = 0x4018,
            FILE_TRANSFER_6 = 0x6018,
            FILE_TRANSFER_A = 0xa018,
            FILE_TRANSFER_C = 0xc018,
            FILE_TRANSFER_E = 0xe018
        };

    private:
        short mCode;

    public:
        explicit Service(int code) : mCode(static_cast<short>(code)) {}

        short getCode() const {
            return mCode;
        }

        static std::optional<Service> fromCode(int code) {
            static const std::vector<Service> services = {
                Service(GFDI),
                Service(REGISTRATION),
                Service(REALTIME_HR),
                Service(REALTIME_STEPS),
                Service(REALTIME_CALORIES),
                Service(REALTIME_INTENSITY),
                Service(REALTIME_HRV),
                Service(REALTIME_STRESS),
                Service(REALTIME_ACCELEROMETER),
                Service(REALTIME_SPO2),
                Service(REALTIME_BODY_BATTERY),
                Service(REALTIME_RESPIRATION),
                Service(FILE_TRANSFER_2),
                Service(FILE_TRANSFER_4),
                Service(FILE_TRANSFER_6),
                Service(FILE_TRANSFER_A),
                Service(FILE_TRANSFER_C),
                Service(FILE_TRANSFER_E)
            };

            for (const auto& service : services) {
                if (service.mCode == code) {
                    return service;
                }
            }
            return std::nullopt;
        }
    };

    Q_OBJECT
public:
    explicit GarminService(const QString &path, QObject *parent);
    static const char* BASE_UUID;
    static const char* UUID_SERVICE_GARMIN_ML_GFDI;
    int mMaxWriteSize = 20;

    void onMtuChanged(int mtu);
    bool initializeDevice( QBLEDevice builder);

private:
    QString mPath;
    QObject *mParent;
    QBLECharacteristic *mCharacteristicSend;
    QBLECharacteristic *mCharacteristicReceive;
    //static GarminDevice mDevice;

    QMap<int, Service> mServiceByHandle;
    QMap<Service, int> mHandleByService;
    //QMap<Service, ServiceCallback> mServiceCallbacks;

    QMap<int, GarminMlrService> mMlrServices;

    bool mRealtimeHrOneShot = false;
    int mPreviousSteps = -1;

    int calcMaxWriteChunk(int mtu);


};




#endif // GARMINSERVICE__H
