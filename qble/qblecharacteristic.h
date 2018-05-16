#ifndef QBLECHARACTERISTIC_H
#define QBLECHARACTERISTIC_H

#include <QObject>

class QBLECharacteristic : public QObject
{
    Q_OBJECT
public:
    explicit QBLECharacteristic(QObject *parent = nullptr);

signals:

public slots:
};

#endif // QBLECHARACTERISTIC_H