#ifndef PROTOBUFTOOLS_H
#define PROTOBUFTOOLS_H

#pragma once

#include <QString>
#include <QByteArray>


// -----------------------------------------------------------------------------
// Truncate String wo maxLen
// -----------------------------------------------------------------------------
QString truncateByChars(const QString& s, quint32 maxLen);

// -----------------------------------------------------------------------------
// Decode protobuf varint
// Returns: (value, bytesConsumed)
// -----------------------------------------------------------------------------
bool decodeVarint(const QByteArray& data, quint64& result, int& bytesRead);

// -----------------------------------------------------------------------------
// Encode protobuf varint
// -----------------------------------------------------------------------------
void encodeVarint(QByteArray& buffer, quint64 value);

// -----------------------------------------------------------------------------
// Encode field key
// -----------------------------------------------------------------------------
void encodeFieldKey(QByteArray& buffer, quint32 fieldNum, quint8 wireType);

// -----------------------------------------------------------------------------
// Add varint to buffer
// -----------------------------------------------------------------------------
void addVarint(QByteArray& buffer, quint32 fieldNum, quint64 data);

// -----------------------------------------------------------------------------
// Add Byte Array to buffer
// -----------------------------------------------------------------------------
void addByteArray(QByteArray& buffer, quint32 fieldNum, const QByteArray data);

// -----------------------------------------------------------------------------
// Parse one protobuf field from data[startOffset...]
// Returns fieldNum, wireType, fieldData (payload only), nextCursor absolute offset
// -----------------------------------------------------------------------------
bool parseField(const QByteArray& data,
                int startOffset,
                quint32& fieldNum,
                quint8& wireType,
                QByteArray& fieldData,
                int& nextCursor);

#endif // PROTOBUFTOOLS_H
