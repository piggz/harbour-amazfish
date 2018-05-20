#ifndef TYPECONVERSION_H
#define TYPECONVERSION_H
#include <QByteArray>
#include <QDateTime>

namespace TypeConversion {

QByteArray fromInt8(int val);
QByteArray fromInt16(int val);
QByteArray fromInt24(int val);
QByteArray fromInt32(int val);
QDateTime rawBytesToDateTime(const QByteArray &value, bool honorDeviceTimeOffset);
QByteArray dateTimeToBytes(const QDateTime &dt, int format);
int toUint16(char val1, char val2);
int toUint32(char val1, char val2, char val3, char val4);

}

#endif // TYPECONVERSION_H
