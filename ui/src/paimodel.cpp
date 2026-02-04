#include "paimodel.h"


PaiModel::PaiModel()
{

}

Q_INVOKABLE QVariantMap PaiModel::get(int row) const
{
    if (row < 0 || row >= m_data.size()) {
        return QVariantMap();
    }

    QVariantMap map;
    map["pai_day"] = m_data.at(row).day;
    map["pai_low"] = m_data.at(row).low;
    map["pai_moderate"] = m_data.at(row).moderate;
    map["pai_high"] = m_data.at(row).high;
    map["pai_time_low"] = m_data.at(row).time_low;
    map["pai_time_moderate"] = m_data.at(row).time_moderate;
    map["pai_time_high"] = m_data.at(row).time_high;
    map["pai_total_today"] = m_data.at(row).total_today;
    map["pai_total"] = m_data.at(row).total;

    return map;
}

QVariant PaiModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > m_data.length()) {
        return QVariant();
    }

    const PaiRecord &item = m_data.at(index.row());

    switch (role) {
    case PaiDay:
        return item.day;
        break;
    case PaiLow:
        return item.low;
        break;
    case PaiModerate:
        return item.moderate;
        break;
    case PaiHigh:
        return item.high;
        break;
    case PaiTimeLow:
        return item.time_low;
        break;
    case PaiTimeModerate:
        return item.time_moderate;
        break;
    case PaiTimeHigh:
        return item.time_high;
        break;
    case PaiTotalToday:
        return item.total_today;
        break;
    case PaiTotal:
        return item.total;
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> PaiModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[PaiDay] = "pai_day";
    names[PaiLow] = "pai_low";
    names[PaiModerate] = "pai_moderate";
    names[PaiHigh] = "pai_high";
    names[PaiTimeLow] = "pai_time_low";
    names[PaiTimeModerate] = "pai_time_moderate";
    names[PaiTimeHigh] = "pai_time_high";
    names[PaiTotalToday] = "pai_total_today";
    names[PaiTotal] = "pai_total";

    return names;
}

int PaiModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.length();
}

void PaiModel::setConnection(KDbConnection *conn)
{
    m_connection = conn;
}

void PaiModel::update()
{
    beginResetModel();

    QString qry = "SELECT pai_date, pai_low, pai_moderate, pai_high, pai_low_time, pai_moderate_time, pai_high_time, pai_today, pai_total from pai WHERE pai_date > (SELECT DATETIME('now', '-7 day')) ORDER BY pai_date ASC";
    m_data.clear();

    if (m_connection && m_connection->isDatabaseUsed()) {
        KDbCursor *curs = m_connection->executeQuery(KDbEscapedString(qry));
        if (curs) {
            if (curs->open() && curs->moveFirst()) {
                while (!curs->eof()) {

                    PaiRecord data;
                    data.day = curs->value(0).toDate();
                    data.low = curs->value(1).toFloat();
                    data.moderate = curs->value(2).toFloat();
                    data.high = curs->value(3).toFloat();
                    data.time_low = curs->value(4).toInt();
                    data.time_moderate = curs->value(5).toInt();
                    data.time_high = curs->value(6).toInt();
                    data.total_today = curs->value(7).toFloat();
                    data.total = curs->value(8).toFloat();
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
