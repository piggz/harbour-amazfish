#include "protobuftools.h"

#include <QDebug>

// -----------------------------------------------------------------------------
// Helpers, might be transferred into seperate file later to be resued by other
// -----------------------------------------------------------------------------
QString truncateByChars(const QString& s, quint32 maxLen)
{
    // maxLen comes from the wire. Compare it before narrowing, a value above
    // INT_MAX turns negative and no truncation happens at all.
    if (maxLen == 0 || quint64(maxLen) >= quint64(s.size())) {
        return s;
    }
    return s.left(int(maxLen));
}

// -----------------------------------------------------------------------------
// Decode protobuf varint
// Returns: (value, bytesConsumed)
// -----------------------------------------------------------------------------
bool decodeVarint(const QByteArray& data, quint64& result, int& bytesRead)
{
    int shift = 0;
    bytesRead = 0;
    result = 0;

    for (auto ch : data) {
        const quint8 byte = quint8(ch);
        ++bytesRead;

        result |= (quint64(byte & 0x7F) << shift);

        if ((byte & 0x80) == 0) {
            return true;
        }

        shift += 7;
        if (shift >= 64) {
            return false; // Varint too long
        }

        if (bytesRead >= 10) {
            break;
        }
    }

    return false; // Incomplete varint
}



// -----------------------------------------------------------------------------
// Encode protobuf varint
// -----------------------------------------------------------------------------
void encodeVarint(QByteArray& buffer, quint64 value)
{
    while (true) {
        quint8 byte = quint8(value & 0x7F);
        value >>= 7;

        if (value != 0) {
            byte |= 0x80;
        }

        buffer.append(char(byte));

        if (value == 0) {
            break;
        }
    }
}

// -----------------------------------------------------------------------------
// Encode field key
// -----------------------------------------------------------------------------
void encodeFieldKey(QByteArray& buffer, quint32 fieldNum, quint8 wireType)
{
    const quint32 key = (fieldNum << 3) | quint32(wireType);
    encodeVarint(buffer, key);
}

void addVarint(QByteArray& buffer, quint32 fieldNum, quint64 data)
{
    //Adds a Varint to the buffer
    //First field key is added, then actual data
    quint8 wireType=0;
    encodeFieldKey(buffer,fieldNum,wireType);
    encodeVarint(buffer,data);
}

void addByteArray(QByteArray& buffer, quint32 fieldNum, const QByteArray data)
{
    //Adds a String / Byte Array to the buffer
    //First field key is added, then length and actual data
    quint8 wireType=2;
    encodeFieldKey(buffer,fieldNum,wireType);
    encodeVarint(buffer,data.size());
    buffer.append(data);
}

// -----------------------------------------------------------------------------
// Parse one protobuf field from data[startOffset...]
// Returns fieldNum, wireType, fieldData (payload only), nextCursor absolute offset
// -----------------------------------------------------------------------------
bool parseField(const QByteArray& data,
                int startOffset,
                quint32& fieldNum,
                quint8& wireType,
                QByteArray& fieldData,
                int& nextCursor)
{
    if (startOffset >= data.size()) {
        return false;
    }

    const QByteArray tail = data.mid(startOffset);

    quint64 key;
    int keyLen;
    bool keyRes = decodeVarint(tail,key,keyLen);
    if (!keyRes) {
        return false;
    }

    fieldNum = quint32(key >> 3);
    wireType = quint8(key & 0x07);

    int cursor = startOffset + keyLen;

    switch (wireType) {
    case 0: {
        // varint payload
        int valueLen;
        quint64 value;
        bool valueRes = decodeVarint(data.mid(cursor),value,valueLen);
        if (!valueRes) {
            return false;
        }


        fieldData = data.mid(cursor, valueLen);
        nextCursor = cursor + valueLen;
        return true;
    }

    case 2: {
        // length-delimited
        quint64 length;
        int lenLen;
        bool  lenRes = decodeVarint(data.mid(cursor),length,lenLen);
        if (!lenRes) {
            return false;
        }

        cursor += lenLen;

        // length comes from the wire. A value above INT_MAX turns negative once
        // narrowed, and then it passes any check made on the narrowed value.
        const int remaining = data.size() - cursor;
        if (remaining <= 0 || length > quint64(remaining)) {
            qDebug().noquote() << Q_FUNC_INFO
                               << QStringLiteral("Length-delimited field claims %1 bytes but only %2 available (field %3, cursor %4)")
                                      .arg(length)
                                      .arg(remaining)
                                      .arg(fieldNum)
                                      .arg(cursor);
            return false;
        }

        fieldData = data.mid(cursor, int(length));
        nextCursor = cursor + int(length);
        return true;
    }

    default:
        qDebug() << Q_FUNC_INFO << "Unknown wire type:" << wireType;
        return false;
    }
}
