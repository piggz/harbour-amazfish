#include "typeconversion.h"

namespace TypeConversion {

QByteArray fromInt8(int val)
{
    return QByteArray(1, val & 0xff);
}

QByteArray fromInt16(int val)
{
    return QByteArray(1, val & 0xff) + QByteArray(1, ((val >> 8) & 0xff));
}

QByteArray fromInt24(int val)
{
    return QByteArray(1, val & 0xff) + QByteArray(1, ((val >> 8) & 0xff)) + QByteArray(1, ((val >> 16) & 0xff));
}

QByteArray fromInt32(int val)
{
    return QByteArray(1, val & 0xff) + QByteArray(1, ((val >> 8) & 0xff)) + QByteArray(1, ((val >> 16) & 0xff)) + QByteArray(1, ((val >> 24) & 0xff));
}

}
