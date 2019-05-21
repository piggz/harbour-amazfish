#include "btdiscoverymodel.h"

BTDiscoveryModel::BTDiscoveryModel()
{

}

int BTDiscoveryModel::rowCount(const QModelIndex &parent) const
{
    return m_devices.count();
}

QVariant BTDiscoveryModel::data(const QModelIndex &index, int role) const
{
    if (index.row() >= rowCount(index)) {
        return QVariant();
    }

    if (role == NameRole) {
        return m_devices[index.row()].name();
    } else if (role == AddressRole) {
        return m_devices[index.row()].address().toString();
    } else {
        return QVariant();
    }
}

QHash<int, QByteArray> BTDiscoveryModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[AddressRole] = "address";
    roles[ClassRole] = "class";
    return roles;
}

void BTDiscoveryModel::foundDevice(const QBluetoothDeviceInfo &device)
{
    beginResetModel();
    m_devices << device;
    endResetModel();
}
