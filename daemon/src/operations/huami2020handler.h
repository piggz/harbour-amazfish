#ifndef HUAMI2_2_HANDLER_H
#define HUAMI2_2_HANDLER_H

#include <QByteArray>

class Huami2020Handler
{
public:
    Huami2020Handler();
    virtual void handle2021Payload(short type, const QByteArray &data) = 0;
};

#endif // HUAMI2_2_HANDLER_H
