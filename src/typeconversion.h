#ifndef TYPECONVERSION_H
#define TYPECONVERSION_H
#include <QByteArray>

namespace TypeConversion {

QByteArray fromInt8(int val);
QByteArray fromInt16(int val);
QByteArray fromInt24(int val);
QByteArray fromInt32(int val);

}

#endif // TYPECONVERSION_H
