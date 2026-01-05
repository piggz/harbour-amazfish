#include "sportsdatamodel.h"
#include "activitykind.h"
#include <QFile>

SportsDataModel::SportsDataModel()
{

}

void SportsDataModel::setConnection(KDbConnection *conn)
{
    m_connection = conn;
}


QHash<int, QByteArray> SportsDataModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[SportId] = "id";
    names[SportName] = "name";
    names[SportVersion] = "version";
    names[SportStartDate] = "startdate";
    names[SportStartDateString] = "startdatestring";
    names[SportEndDate] = "enddate";
    names[SportKind] = "kind";
    names[SportKindString] = "kindstring";
    names[SportBaseLongitude] = "baselongitude";
    names[SportBaseLatitude] = "baselatitude";
    names[SportBaseAltitude] = "basealtitude";

    return names;
}

QVariant SportsDataModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > m_data.length()) {
        return QVariant();
    }

    const SportsData &item = m_data.at(index.row());

    switch (role) {
    case SportId:
        return item.id;
    case SportName:
        return item.name;
    case SportVersion:
        return item.version;
    case SportStartDate:
        return item.startDate;
    case SportStartDateString:
        return item.startDate.toString("dd-MMM-yyyy hh:mm");
    case SportEndDate:
        return item.endDate;
    case SportKind:
        return item.kind;
    case SportKindString:
        return ActivityKind::toString((ActivityKind::Type)item.kind);
    case SportBaseLongitude:
        return item.baseLongitude;
    case SportBaseLatitude:
        return item.baseLatitude;
    case SportBaseAltitude:
        return item.baseAltitude;
    }
    return QVariant();
}

int SportsDataModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.length();
}

void SportsDataModel::update()
{
    beginResetModel();
    QString qry = "SELECT id, name, version, start_timestamp_dt, end_timestamp_dt, kind, base_longitude, base_latitude, base_altitude, gpx FROM sports_data ORDER BY start_timestamp_dt DESC";
    m_data.clear();

    if (m_connection && m_connection->isDatabaseUsed()) {
        KDbCursor *curs = m_connection->executeQuery(KDbEscapedString(qry));
        if (curs) {
            if (curs->open() && curs->moveFirst()) {
                while (!curs->eof()) {

                    SportsData data;
                    data.id = curs->value(0).toInt();
                    data.name = curs->value(1).toString();
                    data.version = curs->value(2).toInt();
                    data.startDate = curs->value(3).toDateTime();
                    data.endDate = curs->value(4).toDateTime();
                    data.kind = curs->value(5).toInt();
                    data.baseLongitude = curs->value(6).toDouble();
                    data.baseLatitude = curs->value(7).toDouble();
                    data.baseAltitude = curs->value(8).toDouble();
                    //qDebug() << data.name << curs->value(8) << curs->value(8).toString() << curs->value(8).toString().toInt();
                    m_data << data;
                    curs->moveNext();
                }
            }
            m_connection->deleteCursor(curs);
        } else {
            qDebug() << "Error executing query";
        }
    }

    endResetModel();
}

QString SportsDataModel::gpx(uint id)
{
    QString qry = QString("SELECT gpx FROM sports_data WHERE id = %1").arg(id);
    QString gpx;

    if (m_connection && m_connection->isDatabaseUsed()) {
        KDbCursor *curs = m_connection->executeQuery(KDbEscapedString(qry));
        if (curs) {
            if (curs->open() && curs->moveFirst()) {
                gpx = curs->value(0).toString();
            }
            m_connection->deleteCursor(curs);
        } else {
            qDebug() << "Error executing query";
        }
    }

    QFile file(gpx);
    if(!file.open(QIODevice::ReadOnly)) {
        return gpx;
    }

    QTextStream in(&file);
    QString line = in.readAll();

    file.close();
    return line;
}

void SportsDataModel::deleteRecord(uint id)
{
    QString qry = QString("DELETE FROM sports_meta WHERE sport_id = %1").arg(id);

    if (m_connection && m_connection->isDatabaseUsed()) {
        if (!m_connection->executeSql(KDbEscapedString(qry))) {
            qDebug() << "Error deleting from sports_meta";
        } else {
            qry = QString("DELETE FROM sports_data WHERE id = %1").arg(id);
            if (!m_connection->executeSql(KDbEscapedString(qry))) {
                qDebug() << "Error deleting from sports_data";
            }
        }
    }
    update();
}
