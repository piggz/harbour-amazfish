#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <MGConfItem>

class SettingsManager : public QObject
{
    Q_OBJECT
public:
    explicit SettingsManager(QObject *parent = nullptr);

    Q_INVOKABLE QVariant value(const QString &key);
    Q_INVOKABLE void setValue(const QString&key, const QVariant &value);

private:
    QList<MGConfItem*> m_settings;
};

#endif // SETTINGSMANAGER_H
