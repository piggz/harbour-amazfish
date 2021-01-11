#include "infinitimenavservice.h"
#include "typeconversion.h"

#include <QDebug>

const char* InfiniTimeNavService::UUID_SERVICE_NAVIGATION = "c7e60001-00fc-48fe-8e23-433b3a1942d0";
const char* InfiniTimeNavService::UUID_CHARACTERISTIC_NAV_FLAG = "c7e60002-00fc-48fe-8e23-433b3a1942d0";
const char* InfiniTimeNavService::UUID_CHARACTERISTIC_NAV_NARRATIVE = "c7e60003-00fc-48fe-8e23-433b3a1942d0";
const char* InfiniTimeNavService::UUID_CHARACTERISTIC_NAV_MAN_DIST = "c7e60004-00fc-48fe-8e23-433b3a1942d0";
const char* InfiniTimeNavService::UUID_CHARACTERISTIC_NAV_PROGRESS = "c7e60005-00fc-48fe-8e23-433b3a1942d0";

InfiniTimeNavService::InfiniTimeNavService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_NAVIGATION, path, parent)
{
    qDebug() << Q_FUNC_INFO;
}

void InfiniTimeNavService::setFlag(const QString &flag)
{
    writeValue(UUID_CHARACTERISTIC_NAV_FLAG, flag.toLocal8Bit());
}

void InfiniTimeNavService::setNarrative(const QString &narrative)
{
    qDebug() << Q_FUNC_INFO << narrative;
    writeValue(UUID_CHARACTERISTIC_NAV_NARRATIVE, narrative.toLocal8Bit());
}

void InfiniTimeNavService::setManDist(const QString &manDist)
{
    writeValue(UUID_CHARACTERISTIC_NAV_MAN_DIST, manDist.toLocal8Bit());
}

void InfiniTimeNavService::setProgress(int progress)
{
    writeValue(UUID_CHARACTERISTIC_NAV_PROGRESS, TypeConversion::fromInt8(progress));
}

