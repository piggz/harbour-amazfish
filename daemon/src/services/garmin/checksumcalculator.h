#ifndef CHECKSUMCALCULATOR_H
#define CHECKSUMCALCULATOR_H

#include <qbuffer.h>

class ChecksumCalculator
{
public:
    ChecksumCalculator();
    int computeCrc(QByteArray data, int offset, int length);
    int computeCrc(QBuffer bytebuffer, int offset, int length);
    int computeCrc(int initialCrc, QByteArray data, int offset, int length);
private:
    static constexpr const int CONSTANTS[] = {
        0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
        0xA001, 0x6C00,0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
    };
};

#endif // CHECKSUMCALCULATOR_H
