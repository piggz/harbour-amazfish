#include "sportsdatamodel.h"
#include "activitykind.h"
#include <QDebug>
#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>

SportsDataModel::SportsDataModel()
{

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

void SportsDataModel::setKind(uint id, const QString &kind) {
    ActivityKind::Type kindType = ActivityKind::fromString(kind);
    qDebug() << "UPDATE sports_data SET kind = " << kindType << " WHERE id = " << id;

    QSqlQuery query;
    if (!query.exec(QString("UPDATE sports_data SET kind = %1 WHERE id = %2").arg(kindType).arg(id))) {
        qWarning() << Q_FUNC_INFO << "Error updating activity type" << query.lastError().text();
    }
}

void SportsDataModel::update()
{
    beginResetModel();
    QString qry = "SELECT id, name, version, start_timestamp_dt, end_timestamp_dt, kind, base_longitude, base_latitude, base_altitude, gpx FROM sports_data ORDER BY start_timestamp_dt DESC";
    m_data.clear();

    QSqlQuery query;
    if (query.exec(qry)) {
        while (query.next()) {
            SportsData data;
            data.id = query.value(0).toInt();
            data.name = query.value(1).toString();
            data.version = query.value(2).toInt();
            data.startDate = query.value(3).toDateTime();
            data.endDate = query.value(4).toDateTime();
            data.kind = query.value(5).toInt();
            data.baseLongitude = query.value(6).toDouble();
            data.baseLatitude = query.value(7).toDouble();
            data.baseAltitude = query.value(8).toDouble();
            m_data << data;
        }
    } else {
        qWarning() << "Error executing query" << query.lastError().text();
    }

    endResetModel();
}

QString SportsDataModel::gpx(uint id)
{
    QString qry = QString("SELECT gpx FROM sports_data WHERE id = %1").arg(id);
    QString gpx;

    QSqlQuery query;
    if (query.exec(qry)) {
        if (query.next()) {
            gpx = query.value(0).toString();
        }
    } else {
        qWarning() << "Error executing query" << query.lastError().text();
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

QString SportsDataModel::rawGpx(uint id)
{
    QString qry = QString("SELECT gpx FROM sports_data WHERE id = %1").arg(id);
    QString gpx;

    QSqlQuery query;
    if (query.exec(qry)) {
        if (query.next()) {
            gpx = query.value(0).toString();
        }
    } else {
        qWarning() << "Error executing query" << query.lastError().text();
    }

    if (gpx.endsWith(".tcx")) {
        gpx.replace(gpx.length() - 4, 4, ".gpx");
    }

    QFile file(gpx);
    if(!file.open(QIODevice::ReadOnly)) {
        return "";
    }

    QTextStream in(&file);
    QString line = in.readAll();

    file.close();
    return line;
}

void SportsDataModel::deleteRecord(uint id)
{
    QString qry = QString("DELETE FROM sports_meta WHERE sport_id = %1").arg(id);

    QSqlQuery query;
    if (!query.exec(qry)) {
        qWarning() << "Error deleting from sports_meta" << query.lastError().text();
    } else {
        qry = QString("DELETE FROM sports_data WHERE id = %1").arg(id);
        if (!query.exec(qry)) {
            qWarning() << "Error deleting from sports_data" << query.lastError().text();
        }
    }
    update();
}
