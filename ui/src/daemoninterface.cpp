#include "daemoninterface.h"

#include <QDBusReply>
#include <QDebug>
#include <QFile>
#include <QStandardPaths>

#include <QSqlDatabase>
#include <QSqlError>
#include <amazfishconfig.h>

DaemonInterface::DaemonInterface(QObject *parent)
    : QObject(parent)
    , m_serviceWatcher(new QDBusServiceWatcher(
                           QStringLiteral(SERVICE_NAME), QDBusConnection::sessionBus(),
                           QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration))
{
    QObject::connect(m_serviceWatcher, &QDBusServiceWatcher::serviceRegistered,   this, &DaemonInterface::connectDaemon);
    QObject::connect(m_serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, &DaemonInterface::connectDatabase);
    QObject::connect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &DaemonInterface::disconnectDaemon);

    connectDaemon();
    connectDatabase();
}

DaemonInterface::~DaemonInterface()
{
    if (iface) {
        iface->deleteLater();
    }
}


void DaemonInterface::connectDaemon()
{
    qDebug() << "Connecting to daemon signals";

    if (iface) {
        iface->deleteLater();
    }

    iface = new QDBusInterface(QStringLiteral(SERVICE_NAME), QStringLiteral("/application"), QString(), QDBusConnection::sessionBus());

    if (!iface->isValid()) {
        iface->deleteLater();
        iface = nullptr;
        return;
    }

    connect(iface, SIGNAL(message(QString)), this, SIGNAL(message(QString)), Qt::UniqueConnection);
    connect(iface, SIGNAL(downloadProgress(int)), this, SIGNAL(downloadProgress(int)), Qt::UniqueConnection);
    connect(iface, SIGNAL(operationRunningChanged()), this, SIGNAL(operationRunningChanged()), Qt::UniqueConnection);
    connect(iface, SIGNAL(buttonPressed(int)), this, SIGNAL(buttonPressed(int)), Qt::UniqueConnection);
    connect(iface, SIGNAL(informationChanged(int, QString)), this, SIGNAL(informationChanged(int,QString)), Qt::UniqueConnection);

    connect(iface, SIGNAL(requestPasskeyFromUI(const QString&, const QString&)),
	    this, SIGNAL(requestPasskey(const QString&, const QString&)), Qt::UniqueConnection);

    connect(iface, SIGNAL(requestPinCodeFromUI(const QString&, const QString&)),
	    this, SIGNAL(requestPinCode(const QString&, const QString&)), Qt::UniqueConnection);

    connect(iface, SIGNAL(requestConfirmationFromUI(const QString&, const QString&, uint)),
	    this, SIGNAL(requestConfirmation(const QString&, const QString&, uint)), Qt::UniqueConnection);

    connect(iface, SIGNAL(displayPasskeyInUI(const QString&, const QString&, uint)), this,
        SIGNAL(displayPasskey(const QString&, const QString&, uint)), Qt::UniqueConnection);

    //Property proxying
    connect(iface, SIGNAL(connectionStateChanged()), this, SLOT(changeConnectionState()), Qt::UniqueConnection);
    changeConnectionState();
}

void DaemonInterface::pair(const QString &name, const QString &deviceType, const QString &address)
{
    qDebug() << Q_FUNC_INFO << name << deviceType << address;

    if (m_connectionState == "pairing") {
        return;
    }

    if (!iface || !iface->isValid()) {
        emit paired(name, address, tr("Unexpected error"));
        return;
    }

    auto watcher = new QDBusPendingCallWatcher(iface->asyncCall(QStringLiteral("pair"), name, deviceType, address));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, name, deviceType, address]() {
        watcher->deleteLater();
    });
}

void DaemonInterface::changeConnectionState()
{
    qDebug() << Q_FUNC_INFO;

    if (!iface || !iface->isValid()) {
        QString state(QStringLiteral("disconnected"));
        if (m_connectionState != state) {
            m_connectionState.swap(state);
            emit connectionStateChanged();
        }
        return;
    }

    auto watcherCount = new QDBusPendingCallWatcher(iface->asyncCall(QStringLiteral("connectionStateChangedCount")));
    connect(watcherCount, &QDBusPendingCallWatcher::finished, this, [this, watcherCount]() {
        QDBusReply<int> reply(watcherCount->reply());
        auto count = reply.value();
        m_connectionStateChangedCount = count;
        emit connectionStateChangedCountChanged();
    });

    auto watcher = new QDBusPendingCallWatcher(iface->asyncCall(QStringLiteral("connectionState")));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher]() {
        QDBusReply<QString> reply(watcher->reply());
        auto state = reply.value();
        if (m_connectionState != state) {
            m_connectionState.swap(state);
            emit connectionStateChanged();
        }
        watcher->deleteLater();
    });

}

void DaemonInterface::connectToDevice()
{
    if (!iface || !iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("connectToDevice"), true);
}

void DaemonInterface::disconnect()
{
    if (!iface || !iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("disconnect"));
}

void DaemonInterface::unpair()
{
    if (!iface || !iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("unpair"));
}


bool DaemonInterface::supportsFeature(Amazfish::Feature f)
{
    if (!iface || !iface->isValid()) {
        return false;
    }
    QDBusReply<bool> reply = iface->call(QStringLiteral("supportsFeature"), (int)f);
    qDebug() << Q_FUNC_INFO << f << reply;
    return reply;
}

bool DaemonInterface::supportsDataType(Amazfish::DataType t)
{
    if (!iface || !iface->isValid()) {
        return false;
    }
    QDBusReply<bool> reply = iface->call(QStringLiteral("supportsDataType"), (int)t);
    qDebug() << Q_FUNC_INFO << t << reply;
    return reply;
}


int DaemonInterface::supportedFeatures()
{
    if (!iface || !iface->isValid()) {
        return 0;
    }
    QDBusReply<int> reply = iface->call(QStringLiteral("supportedFeatures"));
    return reply;
}

int DaemonInterface::supportedDataTypes()
{
    if (!iface || !iface->isValid()) {
        return 0;
    }
    QDBusReply<int> reply = iface->call(QStringLiteral("supportedDataTypes"));
    return reply;
}

QString DaemonInterface::prepareFirmwareDownload(const QString &path)
{
    if (!iface || !iface->isValid()) {
        return QString();
    }
    QDBusReply<QString> reply = iface->call(QStringLiteral("prepareFirmwareDownload"), path);
    return reply;
}

bool DaemonInterface::startDownload()
{
    if (!iface || !iface->isValid()) {
        return false;
    }
    QDBusReply<bool> reply = iface->call(QStringLiteral("startDownload"));
    return reply;
}

void DaemonInterface::fetchData(int dataTypes)
{
    if (!iface || !iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("fetchData"), dataTypes);
}

void DaemonInterface::refreshInformation()
{
    if (!iface || !iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("refreshInformation"));
}

QString DaemonInterface::information(Amazfish::Info i)
{
    if (!iface || !iface->isValid()) {
        return QString();
    }
    QDBusReply<QString> reply = iface->call(QStringLiteral("information"), (int)i);
    return reply;
}

void DaemonInterface::sendAlert(const int notificationId, const QString &appId, const QString &appName, const QString &summary, const QString &body, bool allowDuplicate)
{

    Amazfish::WatchNotification n = {
        .id = notificationId,
        .appId = appId,
        .appName = appName,
        .summary = summary,
        .body = body
    };

    if (!iface || !iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("sendAlert"), n.toQVariantMap(), allowDuplicate);
}

void DaemonInterface::incomingCall(const QString &caller)
{
    if (!iface || !iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("incomingCall"), caller);
}

void DaemonInterface::applyDeviceSetting(Amazfish::Settings s)
{
    if (!iface || !iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("applyDeviceSetting"), (int)s);
}

void DaemonInterface::requestManualHeartrate()
{
    if (!iface || !iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("requestManualHeartrate"));
}

void DaemonInterface::triggerSendWeather()
{
    if (!iface || !iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("triggerSendWeather"));
}

bool DaemonInterface::operationRunning()
{
    if (!iface || !iface->isValid()) {
        return false;
    }
    QDBusReply<bool> reply = iface->call(QStringLiteral("operationRunning"));
    return reply;
}

void DaemonInterface::connectDatabase()
{
    if (QSqlDatabase::database().isOpen()) {
        return;
    }

    QString databaseName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).append("/amazfish.kexi");
    qDebug() << "Database is: " << databaseName;

    // Only the daemon creates the database. The daemon cannot use an empty file made by the UI.
    if (!QFile::exists(databaseName)) {
        qWarning() << "Database does not exist yet";
        return;
    }

    // The models use this default connection.
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    db.setDatabaseName(databaseName);
    if (!db.open()) {
        qWarning() << "Could not open database:" << db.lastError().text();
    }
}

void DaemonInterface::updateCalendar()
{
    if (!iface || !iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("updateCalendar"));
}

void DaemonInterface::reloadCities()
{
    if (!iface || !iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("reloadCities"));
}

void DaemonInterface::enableFeature(Amazfish::Feature feature)
{
    if (!iface || !iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("enableFeature"), (int)feature);
}

void DaemonInterface::requestScreenshot() {
    if (!iface || !iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("requestScreenshot"));
}

QStringList DaemonInterface::supportedDisplayItems()
{
    if (!iface || !iface->isValid()) {
        return QStringList();
    }
    QDBusReply<QStringList> reply = iface->call(QStringLiteral("supportedDisplayItems"));
    return reply;
}

void DaemonInterface::immediateAlert(int level) {
    if (!iface || !iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("immediateAlert"), level);
}

void DaemonInterface::agentPasskeyResponse(const uint response) {
    if (!iface || !iface->isValid()) {
	return;
    }
    iface->call(QStringLiteral("agentPasskeyResponse"), response);
}

void DaemonInterface::agentPinCodeResponse(const QString& response) {
    if (!iface || !iface->isValid()) {
	return;
    }
    iface->call(QStringLiteral("agentPinCodeResponse"), response);
}


void DaemonInterface::agentConfirmationResponse(const bool response) {
    if (!iface || !iface->isValid()) {
	return;
    }
    iface->call(QStringLiteral("agentConfirmationResponse"), response);
}
