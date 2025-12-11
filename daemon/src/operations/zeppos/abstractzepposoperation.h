#ifndef ABSTRACTZEPPOSOPERATION_H
#define ABSTRACTZEPPOSOPERATION_H

class AbstractZeppOsOperation
{
public:
    AbstractZeppOsOperation();
    void perform();

protected:
    virtual void doPerform() = 0;
    void operationFinished();

};

#endif // ABSTRACTZEPPOSOPERATION_H
