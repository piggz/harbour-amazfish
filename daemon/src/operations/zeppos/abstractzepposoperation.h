#ifndef ABSTRACTZEPPOSOPERATION_H
#define ABSTRACTZEPPOSOPERATION_H

#include <qobject.h>

class AbstractZeppOsOperation : public QObject
{
    Q_OBJECT
public:
    AbstractZeppOsOperation();
    void perform();

    Q_SIGNAL void operationComplete();

protected:
    virtual void doPerform() = 0;
    void operationFinished();

};

#endif // ABSTRACTZEPPOSOPERATION_H
