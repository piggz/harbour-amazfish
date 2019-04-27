#ifndef BTDISCOVERYMODEL_H
#define BTDISCOVERYMODEL_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QAbstractListModel>

class BTDiscoveryModel : public QAbstractListModel
{
public:
    BTDiscoveryModel();

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    enum BTRoles {
            NameRole = Qt::UserRole + 1,
            AddressRole,
            ClassRole
        };

private:
    QBluetoothDeviceDiscoveryAgent m_agent;
    QList<QBluetoothDeviceInfo> m_devices;

    Q_SLOT void foundDevice(const QBluetoothDeviceInfo &device);
};

#endif // BTDISCOVERYMODEL_H
