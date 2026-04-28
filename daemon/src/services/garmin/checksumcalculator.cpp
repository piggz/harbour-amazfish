#include "checksumcalculator.h"


ChecksumCalculator::ChecksumCalculator() {

}


int ChecksumCalculator::computeCrc(QByteArray data, int offset, int length) {
        return computeCrc(0, data, offset, length);
    }

int ChecksumCalculator::computeCrc(QBuffer byteBuffer, int offset, int length) {
        QByteArray data=byteBuffer.data();
        return computeCrc(0, data, offset, length);
    }

int ChecksumCalculator::computeCrc(int initialCrc, QByteArray data, int offset, int length) {
        int crc = initialCrc;
        for (int i = offset; i < offset + length; ++i) {
            int b = data[i];
            crc = (((crc >> 4) & 4095) ^ CONSTANTS[crc & 15]) ^ CONSTANTS[b & 15];
            crc = (((crc >> 4) & 4095) ^ CONSTANTS[crc & 15]) ^ CONSTANTS[(b >> 4) & 15];
        }
        return crc;
    }
