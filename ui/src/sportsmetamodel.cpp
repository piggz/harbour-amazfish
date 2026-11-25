#include "sportsmetamodel.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

SportsMetaModel::SportsMetaModel()
{

}


QHash<int, QByteArray> SportsMetaModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[SportMetaId] = "sportmetaid";
    names[SportId] = "sportid";
    names[SportKey] = "key";
    names[SportValue] = "value";
    names[SportUnit] = "unit";

    return names;
}

QVariant SportsMetaModel::data(const QModelIndex &index, int role) const
{
    if (index.row() >= 0 && index.row() < m_data.length()) {
        if (role == SportMetaId) {
            return m_data.at(index.row()).meta_id;
        } else if (role == SportId) {
            return m_data.at(index.row()).sport_id;
        } else if (role == SportKey) {
            return m_data.at(index.row()).key;
        } else if (role == SportValue) {
            return m_data.at(index.row()).value;
        } else if (role == SportUnit) {
            return m_data.at(index.row()).unit;
        }
    }
    return QVariant();
}

int SportsMetaModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.length();
}

void SportsMetaModel::update(uint sportid)
{
    beginResetModel();
    QString qry = QString("SELECT id, sport_id, key, value, unit FROM sports_meta WHERE sport_id = %1 ORDER BY key ASC").arg(sportid);

    m_data.clear();
    QSqlQuery query;
    if (query.exec(qry)) {
        while (query.next()) {
            SportsMeta data;
            data.meta_id = query.value(0).toInt();
            data.sport_id = query.value(1).toInt();
            data.key = query.value(2).toString();
            data.value = query.value(3).toString();
            data.unit = query.value(4).toString();
            m_data << data;
        }
    } else {
        qWarning() << "Error executing query" << query.lastError().text();
    }
    endResetModel();
}
