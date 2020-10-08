#include "sportsmetamodel.h"

SportsMetaModel::SportsMetaModel()
{

}

void SportsMetaModel::setConnection(KDbConnection *conn)
{
    m_connection = conn;
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
    if (m_connection && m_connection->isDatabaseUsed()) {
        KDbCursor *curs = m_connection->executeQuery(KDbEscapedString(qry));
        if (curs) {
            if (curs->open() && curs->moveFirst()) {
                while (!curs->eof()) {

                    SportsMeta data;
                    data.meta_id = curs->value(0).toInt();
                    data.sport_id = curs->value(1).toInt();
                    data.key = curs->value(2).toString();
                    data.value = curs->value(3).toString();
                    data.unit = curs->value(4).toString();

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
