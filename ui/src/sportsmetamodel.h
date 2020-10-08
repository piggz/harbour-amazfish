#ifndef SPORTSDATAMODEL_H
#define SPORTSDATAMODEL_H

#include <QAbstractListModel>
#include <QVariant>

#include <KDb3/KDbConnection>

typedef struct {
    int meta_id;
    int sport_id;
    QString key;
    QString value;
    QString unit;
} SportsMeta;

class SportsMetaModel : public QAbstractListModel
{
    Q_OBJECT
public:
    SportsMetaModel();

    enum Roles {
        SportMetaId = Qt::UserRole + 1,
        SportId,
        SportKey,
        SportValue,
        SportUnit
    };


    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QHash<int, QByteArray> 	roleNames() const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void setConnection(KDbConnection *conn);

    Q_INVOKABLE void update(uint sportid);

private:
    KDbConnection *m_connection;
    QList<SportsMeta> m_data;
};

#endif // SPORTSDATAMODEL_H
