#include "infinitimenavservice.h"
#include "typeconversion.h"

#include <QDebug>

const char* InfiniTimeNavService::UUID_SERVICE_NAVIGATION = "00010000-78fc-48fe-8e23-433b3a1942d0";
const char* InfiniTimeNavService::UUID_CHARACTERISTIC_NAV_FLAG = "00010001-78fc-48fe-8e23-433b3a1942d0";
const char* InfiniTimeNavService::UUID_CHARACTERISTIC_NAV_NARRATIVE = "00010002-78fc-48fe-8e23-433b3a1942d0";
const char* InfiniTimeNavService::UUID_CHARACTERISTIC_NAV_MAN_DIST = "00010003-78fc-48fe-8e23-433b3a1942d0";
const char* InfiniTimeNavService::UUID_CHARACTERISTIC_NAV_PROGRESS = "00010004-78fc-48fe-8e23-433b3a1942d0";

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

