#ifndef SPORTSDATAMODEL_H
#define SPORTSDATAMODEL_H

#include <QAbstractListModel>
#include <QVariant>
#include <QGeoCoordinate>

#include <KDb3/KDbConnection>

typedef struct {
    int id;
    QString name;
    int version;
    QDateTime startDate;
    QDateTime endDate;
    int kind;
    double baseLongitude;
    double baseLatitude;
    int baseAltitude;
} SportsData;

class SportsDataModel : public QAbstractListModel
{
    Q_OBJECT
public:
    SportsDataModel();

    enum Roles {
        SportId = Qt::UserRole + 1,
        SportName,
        SportVersion,
        SportStartDate,
        SportEndDate,
        SportKind,
        SportKindString,
        SportBaseLongitude,
        SportBaseLatitude,
        SportBaseAltitude
    };


    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QHash<int, QByteArray> 	roleNames() const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void setConnection(KDbConnection *conn);

    Q_INVOKABLE void update();
    Q_INVOKABLE QString gpx(uint id);

private:
    KDbConnection *m_connection = nullptr;
    QList<SportsData> m_data;
    QList<QGeoCoordinate> m_points;
};

#endif // SPORTSDATAMODEL_H
