#ifndef PAIMODEL_H
#define PAIMODEL_H

#include <QAbstractListModel>
#include <QObject>

#include <KDb3/KDbConnection>

class PaiModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit PaiModel();

    struct PaiRecord {
        QDate day;
        float low;
        float moderate;
        float high;
        uint16_t time_low;
        uint16_t time_moderate;
        uint16_t time_high;
        float total_today;
        float total;
    };

    enum Roles {
        PaiDay = Qt::UserRole + 1,
        PaiLow,
        PaiModerate,
        PaiHigh,
        PaiTimeLow,
        PaiTimeModerate,
        PaiTimeHigh,
        PaiTotalToday,
        PaiTotal
    };

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> 	roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void setConnection(KDbConnection *conn);

    Q_INVOKABLE QVariantMap get(int row) const;
    Q_INVOKABLE void update();

private:
    KDbConnection *m_connection = nullptr;
    QList<PaiRecord> m_data;

};

#endif // PAIMODEL_H
