#include "sportsdatamodel.h"
#include "activitykind.h"

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
    if (index.row() >= 0 && index.row() < m_data.length()) {
        if (role == SportId) {
            return m_data.at(index.row()).id;
        } else if (role == SportName) {
            return m_data.at(index.row()).name;
        } else if (role == SportVersion) {
            return m_data.at(index.row()).version;
        } else if (role == SportStartDate) {
            return m_data.at(index.row()).startDate;
        } else if (role == SportEndDate) {
            return m_data.at(index.row()).endDate;
        } else if (role == SportKind) {
            return m_data.at(index.row()).kind;
        } else if (role == SportKindString) {
            return ActivityKind::toString((ActivityKind::Type)m_data.at(index.row()).kind);
        } else if (role == SportBaseLongitude) {
            return m_data.at(index.row()).baseLongitude;
        } else if (role == SportBaseLatitude) {
            return m_data.at(index.row()).baseLatitude;
        } else if (role == SportBaseAltitude) {
            return m_data.at(index.row()).baseAltitude;
        }
        return QVariant();
    }
}

int SportsDataModel::rowCount(const QModelIndex &parent) const
{
    return m_data.length();
}

void SportsDataModel::update()
{
    beginResetModel();
    QString qry = "SELECT id, name, version, start_timestamp_dt, end_timestamp_dt, kind, base_longitude, base_latitude, base_altitude FROM sports_data ORDER BY start_timestamp_dt DESC";

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
