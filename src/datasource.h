#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QObject>
#include <QVariant>

#include <KDb3/KDbConnection>

class DataSource: public QObject
{
    Q_OBJECT
    Q_ENUMS(Type)
public:
    enum Type {
        Heartrate = 1,
        Steps = 2,
        Sleep = 3,
        Intensity = 4
    };
    DataSource();
    void setConnection(KDbConnection *conn);

    Q_INVOKABLE QVariant data(Type type, const QDate  &day);

private:
    KDbConnection *m_conn = nullptr;
};

#endif // DATASOURCE_H
