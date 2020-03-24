#include "amazfishconfig.h"

#include <QCoreApplication>
#include <MDConfGroup>

#include <functional>


AmazfishConfig::AmazfishConfig(QObject *parent)
    : QObject(parent)
    , m_group(new MDConfGroup(QStringLiteral("/uk/co/piggz/amazfish"), this))
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
    return m_group->value(key, def);
}

void AmazfishConfig::setValue(const QString &key, const QVariant &value)
{
    m_group->setValue(key, value);
}

void AmazfishConfig::setValue(const QString &key, const QVariant &value, signal_ptr signal)
{
    auto prev = m_group->value(key);
    if (value != prev) {
        m_group->setValue(key, value);
        emit std::bind(signal, this)();
    }
}
