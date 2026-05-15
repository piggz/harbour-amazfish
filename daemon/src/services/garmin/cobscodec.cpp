#include "cobscodec.h"

#include <QtGlobal>
#include <algorithm>

void CobsCoDec::ensureTimerStarted()
{
    if (!mTimer.isValid()) {
        mTimer.start(); // monotonic elapsed timer where available [2](https://doc.qt.io/qt-6.11/qelapsedtimer.html)
    }
}

void CobsCoDec::receiveBytes(const QByteArray& bytes)
{
    ensureTimerStarted();

    const qint64 nowMs = mTimer.elapsed();
    if (mLastUpdateMs.has_value()) {
        if ((nowMs - *mLastUpdateMs) > mBufferTimeoutMs) {
            reset();
        }
    }

    mLastUpdateMs = nowMs;
    mBuffer.append(bytes);
    decode();
}

void CobsCoDec::reset()
{
    mBuffer.clear();
    mDecodedMessage.reset();
}

std::optional<QByteArray> CobsCoDec::retrieveMessage()
{
    if (!mDecodedMessage.has_value())
        return std::nullopt;

    auto out = mDecodedMessage;
    mDecodedMessage.reset();
    return out;
}

bool CobsCoDec::hasMessage() const
{
    return mDecodedMessage.has_value();
}

void CobsCoDec::decode()
{
    // If already have a decoded message waiting, don't decode more (matches Rust)
    if (mDecodedMessage.has_value())
        return;

    // Minimum framing length: [0x00][code][...][0x00]
    if (mBuffer.size() < 4)
        return;

    // Must end with trailing 0x00
    if (quint8(mBuffer.at(mBuffer.size()-1)) != 0)
        return;

    // Exclude trailing 0 from processing
    const int bufferLen = mBuffer.size() - 1;

    // Must begin with leading 0x00 (Garmin variant)
    if (quint8(mBuffer[0]) != 0)
        return;

    QByteArray decoded;
    decoded.reserve(bufferLen);

    int pos = 1; // skip leading 0

    while (pos < bufferLen) {
        const quint8 code = quint8(mBuffer[pos]);
        if (code == 0)
            break;

        ++pos;

        const int codeValue = int(code);
        const int payloadSize = codeValue - 1;

        // Copy payload bytes
        for (int i = 0; i < payloadSize; ++i) {
            if (pos >= bufferLen) {
                // incomplete frame; wait for more
                return;
            }
            decoded.append(mBuffer[pos]);
            ++pos;
        }

        // Append implied 0 unless code==0xFF
        if (codeValue != 0xFF) {
            if (payloadSize == 0) {
                // no payload => explicit zero
                decoded.append(char(0x00));
            } else if (pos < bufferLen) {
                // payload => implied zero only if more data follows
                decoded.append(char(0x00));
            }
        }
    }

    // Success: store (optional) and emit signal
    mDecodedMessage = decoded;
    emit messageDecoded(decoded);

    // Remove consumed message including trailing delimiter
    mBuffer.remove(0, bufferLen + 1);
}

QByteArray CobsCoDec::encode(const QByteArray& data)
{
    QByteArray encoded;
    encoded.reserve(data.size() * 2 + 2);

    // Garmin leading padding
    encoded.append(char(0x00));

    if (data.isEmpty()) {
        encoded.append(char(0x01));
        encoded.append(char(0x00));
        return encoded;
    }

    int pos = 0;
    bool lastWasZero = false;
    int lastPayloadSize = 0;

    while (pos < data.size()) {
        const int startPos = pos;

        // Find next zero
        int zeroPos = pos;
        while (zeroPos < data.size() && quint8(data[zeroPos]) != 0) {
            ++zeroPos;
        }

        int payloadSize = zeroPos - startPos;
        int currentStart = startPos;

        // Payloads >= 0xFE => emit 0xFF blocks
        while (payloadSize >= 0xFE) {
            encoded.append(char(0xFF));
            encoded.append(data.mid(currentStart, 0xFE));
            payloadSize -= 0xFE;
            currentStart += 0xFE;
        }

        // Remaining
        encoded.append(char(payloadSize + 1));
        if (payloadSize > 0) {
            encoded.append(data.mid(currentStart, payloadSize));
        }

        lastPayloadSize = payloadSize;

        if (zeroPos < data.size()) {
            pos = zeroPos + 1;
            lastWasZero = true;
        } else {
            pos = zeroPos;
            lastWasZero = false;
        }
    }

    if (lastWasZero && pos >= data.size() && lastPayloadSize > 0) {
        encoded.append(char(0x01));
    }

    // trailing delimiter
    encoded.append(char(0x00));
    return encoded;
}
