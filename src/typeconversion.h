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
int toUint16(char val1, char val2);

}

#endif // TYPECONVERSION_H
