#ifndef GARMINSERVICE_H
#define GARMINSERVICE_H

#include <QBLEService>
#include <QObject>

class GarminService : public QBLEService
{
public:
    explicit GarminService(QObject *parent = nullptr);
};

#endif // GARMINSERVICE_H
