#include "settingsmanager.h"

SettingsManager::SettingsManager(QObject *parent) : QObject(parent)
{
#if 0
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/pairedAddress", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/pairedName", this);

    m_settings << new MGConfItem("/uk/co/piggz/amazfish/app/notifyconnect", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/app/refreshweather", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/app/autosyncdata", this);

    m_settings << new MGConfItem("/uk/co/piggz/amazfish/profile/name", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/profile/dob", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/profile/gender", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/profile/weight", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/profile/height", this);

    m_settings << new MGConfItem("/uk/co/piggz/amazfish/device/language", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/device/dateformat", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/device/timeformat", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/device/distanceunit", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/device/lastactivitysyncmillis", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/device/lastsportsyncmillis", this);

    m_settings << new MGConfItem("/uk/co/piggz/amazfish/profile/displayonliftwrist", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/profile/alertfitnessgoal", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/profile/fitnessgoal", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/profile/alldayhrm", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/profile/hrmsleepsupport", this);

    m_settings << new MGConfItem("/uk/co/piggz/amazfish/device/displaystatus", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/device/displayactivity", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/device/displayweather", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/device/displayalarm", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/device/displaytimer", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/device/displaycompass", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/device/displaysettings", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/device/displayalipay", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/device/displayweathershortcut", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/device/displayalipayshortcut", this);

    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm1/enabled", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm1/repeat", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm1/hour", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm1/minute", this);

    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm2/enabled", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm2/repeat", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm2/hour", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm2/minute", this);

    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm3/enabled", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm3/repeat", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm3/hour", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm3/minute", this);

    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm4/enabled", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm4/repeat", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm4/hour", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm4/minute", this);

    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm5/enabled", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm5/repeat", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm5/hour", this);
    m_settings << new MGConfItem("/uk/co/piggz/amazfish/alarms/alarm5/minute", this);
#endif
}

QVariant SettingsManager::value(const QString &key)
{
    if (!m_settings.contains(key)) {
        m_settings[key] = new MGConfItem(key, this);
    }

    return m_settings[key]->value();
#if 0
    Q_FOREACH(MGConfItem* item, m_settings) {
        item->sync();
        if (item->key() == key) {
            return item->value();
        }
    }
    return QVariant();
#endif
}

QVariant SettingsManager::value(const QString &key, const QVariant &def)
{
    if (!m_settings.contains(key)) {
        m_settings[key] = new MGConfItem(key, this);
    }

    return m_settings[key]->value(def);
#if 0
    Q_FOREACH(MGConfItem* item, m_settings) {
        item->sync();
        if (item->key() == key) {
            return item->value();
        }
    }
    return def;
#endif
}

void SettingsManager::setValue(const QString&key, const QVariant &value)
{
    if (!m_settings.contains(key)) {
        m_settings[key] = new MGConfItem(key, this);
    }

    m_settings[key]->set(value);
#if 0
        Q_FOREACH(MGConfItem* item, m_settings) {
        item->sync();
        if (item->key() == key) {
            item->set(value);
        }
    }
#endif
}
