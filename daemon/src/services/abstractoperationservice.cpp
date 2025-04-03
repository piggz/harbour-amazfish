#include "abstractoperationservice.h"


AbstractOperationService::AbstractOperationService(const QString &uuid, const QString &path, QObject *parent)
    : QBLEService(uuid, path, parent)
{

}

void AbstractOperationService::registerOperation(AbstractOperation *operation)
{
    if (!m_operations.contains(operation)) {
        m_operations.push_back(operation);
    }
}

void AbstractOperationService::notifyOperations(const QString &characteristic, const QByteArray &value)
{
    QVector <AbstractOperation *> finishedList;

    foreach (auto operation, m_operations) {
        if (operation->characteristicChanged(characteristic, value)) {
            finishedList.push_back(operation);
        }
    }

    if (finishedList.size() > 0) {
        foreach (auto operation, finishedList) {
            auto op = m_operations.takeAt(m_operations.indexOf(operation));
            delete op;
        }
    }
}
