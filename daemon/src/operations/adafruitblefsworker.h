#ifndef ADAFRUITBLEFSWORKER_H
#define ADAFRUITBLEFSWORKER_H

#include <QObject>
#include "abstractfirmwareinfo.h"
#include "adafruitblefsoperation.h"

class AdafruitBleFsService;

class BleFsWorker : public QObject
{
    Q_OBJECT
public:
    explicit BleFsWorker(const AbstractFirmwareInfo *info, QObject *parent = nullptr);
    ~BleFsWorker();

private:
    std::vector<AdafruitBleFsOperation::File> getRemoteFileList(AdafruitBleFsOperation* service);
    void eraseRemoteFile(AdafruitBleFsOperation* service, QString file);

    bool m_interupted = false;
    const AbstractFirmwareInfo *info;

public slots:
    void updateFiles(AdafruitBleFsOperation* service, int mtu);

signals:
    void done();
};

#endif // ADAFRUITBLEFSWORKER_H
