#ifndef HUAMI2021HANDLER_H
#define HUAMI2021HANDLER_H

#include <QByteArray>

class Huami2021Handler
{
public:
    Huami2021Handler();
    virtual void handle2021Payload(short type, const QByteArray &data) = 0;
};

#endif // HUAMI2_2_HANDLER_H
