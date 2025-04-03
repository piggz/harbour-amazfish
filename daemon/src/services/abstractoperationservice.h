#ifndef ABSTRACTOPERATIONSERVICE_H
#define ABSTRACTOPERATIONSERVICE_H

#include "abstractoperation.h"

#include <qbleservice.h>


class AbstractOperationService : public QBLEService
{
    Q_OBJECT
public:
    explicit AbstractOperationService(const QString &uuid, const QString &path, QObject *parent);

    bool registerOperation(AbstractOperation *operation);
    void notifyOperation(const QString &characteristic, const QByteArray &value);
    void cancelOperation();

    Q_INVOKABLE virtual bool operationRunning();
    Q_SIGNAL void operationRunningChanged();
    Q_SIGNAL void operationComplete(AbstractOperation* m_currentOperation);

private:
    AbstractOperation* m_currentOperation = nullptr;
    AbstractOperation* m_queuedOperation = nullptr;
    QTimer *m_operationTimeout = nullptr;


    void operationTimeout();
};

#endif // ABSTRACTOPERATIONSERVICE_H
