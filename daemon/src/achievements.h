#ifndef ACHIEVEMENTS_H
#define ACHIEVEMENTS_H

#include <QString>
#include <QObject>
#include <QDebug>
#include <map>
#include <memory>
#include <QDate>

class Achievements : public QObject
{
    Q_OBJECT
public:
    explicit Achievements(QObject *parent = nullptr);
    void updateStepsStatus(int steps, int goal);

private:
    std::map<float, QString> loreStepMessages;
    std::map<float, QString> stepMessages;
    QString selectStepsMessage(int steps, double goal);
};

#endif // ACHIEVEMENTS_H
