#include "abstractoperationservice.h"


AbstractOperationService::AbstractOperationService(const QString &uuid, const QString &path, QObject *parent)
    : QBLEService(uuid, path, parent)
{
    m_operationTimeout = new QTimer();
    connect(m_operationTimeout, &QTimer::timeout, this, &AbstractOperationService::operationTimeout);
}

bool AbstractOperationService::registerOperation(AbstractOperation *operation)
{
    qDebug() << Q_FUNC_INFO;
    if (m_currentOperation) {
        if (m_queuedOperation) {
            return false;
        }  else {
            qDebug() << "Setting the queued operation";
            m_queuedOperation = operation;
        }
    } else {
        qDebug() << "Setting the current operation";
        m_currentOperation = operation;
    }
    return true;
}

AbstractOperation *AbstractOperationService::currentOperation()
{
    return m_currentOperation;
}

void AbstractOperationService::notifyOperation(const QString &characteristic, const QByteArray &value)
{
    QVector <AbstractOperation *> finishedList;

    if (m_currentOperation) {
        m_operationTimeout->start(10000);
        bool finished = m_currentOperation->characteristicChanged(characteristic, value);

        if (finished) {
            qDebug() << "Operation complete, deleting";
            emit operationComplete(m_currentOperation);
            delete m_currentOperation;
            m_currentOperation = nullptr;

            if (m_queuedOperation) {
                m_currentOperation = m_queuedOperation;
                m_queuedOperation = nullptr;
            } else {
                m_operationTimeout->stop();
                emit operationRunningChanged();
            }
        }
    }
}

void AbstractOperationService::cancelOperation()
{
    if (m_currentOperation) {
        delete m_currentOperation;
        m_currentOperation = nullptr;
        emit operationRunningChanged();
        m_operationTimeout->stop();
    }
}

bool AbstractOperationService::operationRunning() {
    return m_currentOperation;
}

void AbstractOperationService::operationTimeout()
{
    qDebug() << Q_FUNC_INFO << "Timeout while waiting for operation data";
    cancelOperation();
}
