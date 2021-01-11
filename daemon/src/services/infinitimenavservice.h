#ifndef INFINITIMENAVSERVICE_H
#define INFINITIMENAVSERVICE_H

#include <QObject>
#include "qble/qbleservice.h"

/** c7e60001-00fc-48fe-8e23-433b3a1942d0 --Nav Service
 * "c7e60002-00fc-48fe-8e23-433b3a1942d0", --Flag
 * "c7e60003-00fc-48fe-8e23-433b3a1942d0", --Narrative
 * "c7e60004-00fc-48fe-8e23-433b3a1942d0", --Manouver Distance
 * "c7e60005-00fc-48fe-8e23-433b3a1942d0", --Progress
 */
class InfiniTimeNavService : public QBLEService
{
    Q_OBJECT
public:
    InfiniTimeNavService(const QString &path, QObject *parent);

    static const char *UUID_SERVICE_NAVIGATION;
    static const char *UUID_CHARACTERISTIC_NAV_FLAG;
    static const char *UUID_CHARACTERISTIC_NAV_NARRATIVE;
    static const char *UUID_CHARACTERISTIC_NAV_MAN_DIST;
    static const char *UUID_CHARACTERISTIC_NAV_PROGRESS;

    void setFlag(const QString &flag);
    void setNarrative(const QString &narrative);
    void setManDist(const QString &manDist);
    void setProgress(int progress);

};

#endif // INFINITIMENAVSERVICE_H
