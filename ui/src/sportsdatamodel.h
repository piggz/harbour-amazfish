#ifndef SPORTSDATAMODEL_H
#define SPORTSDATAMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QGeoCoordinate>
#include <QVariant>

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
        SportStartDateString,
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

    Q_INVOKABLE void setKind(uint id, const QString &kind);
    Q_INVOKABLE void update();
    Q_INVOKABLE QString gpx(uint id);
    Q_INVOKABLE QString rawGpx(uint id);
    Q_INVOKABLE void deleteRecord(uint id);

private:
    QList<SportsData> m_data;
    QList<QGeoCoordinate> m_points;
};

#endif // SPORTSDATAMODEL_H
