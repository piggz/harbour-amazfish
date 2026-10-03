#ifndef COBSCODEC__H
#define COBSCODEC__H

#pragma once

#include <QObject>
#include <QByteArray>
#include <QElapsedTimer>
#include <optional>
#include <QDebug>

class CobsCoDec : public QObject
{
    Q_OBJECT

public:
    static constexpr qint64 kDefaultTimeoutMs = 1500;

    explicit CobsCoDec(QObject* parent = nullptr)
        : QObject(parent = nullptr)
    {
     //   mBuffer.reserve(10000);
    }

    // Streaming input: accumulate and attempt decode; emits messageDecoded() when complete.
    void receiveBytes(const QByteArray& bytes);
    // Feed received bytes into the decoder. Emits messageDecoded when a complete frame is found.
    void feed(const QByteArray &data);

    void reset();

    // Garmin-variant encode: leading 0x00 + trailing 0x00
    static QByteArray encode(const QByteArray &data);

signals:
    void messageDecoded(const QByteArray& message);
    void frameError(const QString &reason);


private:
    void decode(); // emits signal when decode succeeds

    QByteArray mBuffer;

    static const int MAX_BUFFER_SIZE = 10000;


};

#endif // COBSCODEC__H
