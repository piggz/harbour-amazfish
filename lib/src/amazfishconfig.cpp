#include "amazfishconfig.h"

#include <QCoreApplication>

#ifdef MER_EDITION_SAILFISH
#include <MDConfGroup>
#endif

#include <functional>


AmazfishConfig::AmazfishConfig(QObject *parent)
    : QObject(parent)
#ifdef MER_EDITION_SAILFISH
    , m_group(new MDConfGroup(QStringLiteral("/uk/co/piggz/amazfish"), this))
#endif
{

}

AmazfishConfig *AmazfishConfig::instance()
{
    static AmazfishConfig *inst = nullptr;
    if (!inst) {
        inst = new AmazfishConfig(qApp);
    }
    return inst;
}

QVariant AmazfishConfig::value(const QString &key, const QVariant &def) const
{
#ifdef MER_EDITION_SAILFISH
    return m_group->value(key, def);
#else
    QSettings settings;
    return settings.value(key, def);
#endif
}

void AmazfishConfig::setValue(const QString &key, const QVariant &value)
{
#ifdef MER_EDITION_SAILFISH
    m_group->setValue(key, value);
#else
    QSettings settings;
    settings.setValue(key, value);
#endif
}

void AmazfishConfig::setValue(const QString &key, const QVariant &value, signal_ptr signal)
{
#ifdef MER_EDITION_SAILFISH
    auto prev = m_group->value(key);
#else
    QSettings settings;
    auto prev = settings.value(key);
#endif

    if (value != prev) {
#ifdef MER_EDITION_SAILFISH
        m_group->setValue(key, value);
#else
        settings.setValue(key, value);
#endif
        emit std::bind(signal, this)();
    }
}
