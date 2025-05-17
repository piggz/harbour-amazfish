#ifndef TYPECONVERSION_H
#define TYPECONVERSION_H
#include <QByteArray>
#include <QDateTime>

namespace TypeConversion {

QByteArray fromInt8(int val);
QByteArray fromInt16(int val);
QByteArray fromInt24(int val);
QByteArray fromInt32(int val);
QByteArray BEfromInt16(int val);
QByteArray fromInt64(long long int val);
QDateTime rawBytesToDateTime(const QByteArray &value, bool honorDeviceTimeOffset);
QByteArray dateTimeToBytes(const QDateTime &dt, int format, bool adjustForTZ = true);
int toUint16(char val1, char val2);
uint32_t toUint32(char val1, char val2, char val3, char val4);
int32_t toInt32(char val1, char val2, char val3, char val4);
int64_t toInt64(char val1, char val2, char val3, char val4, char val5, char val6, char val7, char val8);
long toUnsigned(int unsignedInt);
int toUnsigned(short value);
int toUnsigned(char value);
int16_t toInt16(uint8_t val1, uint8_t val2);
uint16_t toUint16(char value);
}

#endif // TYPECONVERSION_H
