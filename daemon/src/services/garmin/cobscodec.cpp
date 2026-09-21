#include "cobscodec.h"

#include <QtGlobal>
#include <algorithm>

void CobsCoDec::reset()
{
    mBuffer.clear();
   // mDecodedMessage.reset();
}

void CobsCoDec::feed(const QByteArray &data)
{
    if (mBuffer.size() + data.size() > MAX_BUFFER_SIZE) {
        qDebug() << Q_FUNC_INFO << "COBS buffer overflow, resetting";
        mBuffer.clear();
        emit frameError(QStringLiteral("Buffer overflow"));
    }

    mBuffer.append(data);
    decode();
}


QByteArray CobsCoDec::encode(const QByteArray &data)
{
    QByteArray result;
    result.reserve(data.size() * 2 + 2);

    // Garmin leading padding
    result.append('\0');

    int start = 0;
    bool lastByteWasZero = false;

    while (start < data.size()) {
        int zeroPos = data.indexOf('\0', start);
        int chunkEnd = (zeroPos < 0) ? data.size() : zeroPos;
        int chunkLen = chunkEnd - start;
        lastByteWasZero = (zeroPos >= 0);

        while (chunkLen >= 0xFE) {
            result.append((char)0xFF);
            result.append(data.mid(start, 0xFE));
            start += 0xFE;
            chunkLen -= 0xFE;
        }

        result.append((char)(chunkLen + 1));
        if (chunkLen > 0) {
            result.append(data.mid(start, chunkLen));
        }
        start += chunkLen;

        if (zeroPos >= 0) {
            start++; // skip the zero
        }
    }

    if (lastByteWasZero) {
        // The data ended in a 0x00: emit an explicit empty chunk so the
        // decoder doesn't mistake the frame terminator for that zero.
        result.append((char)0x01);
    }

    result.append('\0'); // trailing frame terminator
    return result;
}


void CobsCoDec::decode()
{

    // Need at least: leading 0x00 + one code byte + trailing 0x00
    if (mBuffer.size() < 4)
        return;

    if ((unsigned char)mBuffer.at(mBuffer.size() - 1) != 0x00)
        return; // no complete frame yet

    if ((unsigned char)mBuffer.at(0) != 0x00) {
        qDebug() << Q_FUNC_INFO << "Garmin: COBS frame missing leading 0x00, discarding" << mBuffer.size() << "bytes";
        emit frameError(QStringLiteral("Missing leading 0x00"));
        mBuffer.clear();
        return;
    }

    const int end = mBuffer.size() - 1; // exclude trailing 0x00
    QByteArray decoded;
    decoded.reserve(end);
    int pos = 1; // skip leading 0x00

    while (pos < end) {
        unsigned char code = (unsigned char)mBuffer.at(pos);
        if (code == 0x00) {
            // Should never happen inside a well-formed frame - COBS
            // eliminates embedded zero bytes by construction.
            break;
        }

        int payloadSize = code - 1;
        if (pos + 1 + payloadSize > end) {
            qDebug() << Q_FUNC_INFO << "COBS truncated frame, discarding";
            emit frameError(QStringLiteral("Truncated frame"));
            mBuffer.clear();
            return;
        }

        decoded.append(mBuffer.mid(pos + 1, payloadSize));
        pos += 1 + payloadSize;

        // Append implicit zero if code != 0xFF and not at end
        if (code != 0xFF && pos < end) {
            decoded.append('\0');
        }
    }

    mBuffer.clear();

    if (!decoded.isEmpty()) {
        emit messageDecoded(decoded);
    }
}
