#ifndef TIMEFORMATTER_H
#define TIMEFORMATTER_H

#include <QObject>
#include <QString>

class TimeFormatter : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE static QString formatHMS_fromSeconds(const uint seconds);
    Q_INVOKABLE static QString formatHMS(const uint hours, const uint minutes, const uint seconds);
};

#endif // TIMEFORMATTER_H
