#ifndef COBSCODEC__H
#define COBSCODEC__H

#pragma once

#include <QObject>
#include <QByteArray>
#include <QElapsedTimer>
#include <optional>

class CobsCoDec : public QObject
{
    Q_OBJECT

public:
    static constexpr qint64 kDefaultTimeoutMs = 1500;

    explicit CobsCoDec(QObject* parent = nullptr)
        : QObject(parent), mBufferTimeoutMs(kDefaultTimeoutMs)
    {
        mBuffer.reserve(10'000);
    }

    explicit CobsCoDec(qint64 timeoutMs, QObject* parent = nullptr)
        : QObject(parent), mBufferTimeoutMs(timeoutMs)
    {
        mBuffer.reserve(10'000);
    }

    // Streaming input: accumulate and attempt decode; emits messageDecoded() when complete.
    void receiveBytes(const QByteArray& bytes);

    void reset();

    // Optional compatibility API (polling):
    std::optional<QByteArray> retrieveMessage();
    bool hasMessage() const;

    // Garmin-variant encode: leading 0x00 + trailing 0x00
    static QByteArray encode(const QByteArray& data);

signals:
    void messageDecoded(const QByteArray& message);

private:
    void ensureTimerStarted();
    void decode(); // emits signal when decode succeeds

private:
    QByteArray mBuffer;
    QElapsedTimer mTimer;
    std::optional<qint64> mLastUpdateMs;
    std::optional<QByteArray> mDecodedMessage;
    qint64 mBufferTimeoutMs;
};

#endif // COBSCODEC__H
