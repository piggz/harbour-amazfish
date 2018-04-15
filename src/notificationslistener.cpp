/**
 * Copyright 2015 Holger Kaelberer <holger.k@elberer.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusArgument>
#include <QtDebug>
#include <QLoggingCategory>
#include <QStandardPaths>

#include "notificationslistener.h"

NotificationsListener::NotificationsListener(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    //qRegisterMetaTypeStreamOperators<NotifyingApplication>("NotifyingApplication");

    bool ret = QDBusConnection::sessionBus()
                .registerObject(QStringLiteral("/org/freedesktop/Notifications"),
                                this,
                                QDBusConnection::ExportScriptableContents);
    if (!ret)
        qWarning()
                << "Error registering notifications listener for device"
                << ":" << QDBusConnection::sessionBus().lastError();
    else
        qDebug()
                << "Registered notifications listener for device";

    QDBusInterface iface(QStringLiteral("org.freedesktop.DBus"), QStringLiteral("/org/freedesktop/DBus"),
                         QStringLiteral("org.freedesktop.DBus"));
    iface.call(QStringLiteral("AddMatch"),
               "interface='org.freedesktop.Notifications',member='Notify',type='method_call',eavesdrop='true'");
}

NotificationsListener::~NotificationsListener()
{
    qDebug()<< "Destroying NotificationsListener";
    QDBusInterface iface(QStringLiteral("org.freedesktop.DBus"), QStringLiteral("/org/freedesktop/DBus"),
                         QStringLiteral("org.freedesktop.DBus"));
    QDBusMessage res = iface.call(QStringLiteral("RemoveMatch"),
                                  "interface='org.freedesktop.Notifications',member='Notify',type='method_call',eavesdrop='true'");
    QDBusConnection::sessionBus().unregisterObject(QStringLiteral("/org/freedesktop/Notifications"));
}

uint NotificationsListener::Notify(const QString& appName, uint replacesId,
                                   const QString& appIcon,
                                   const QString& summary, const QString& body,
                                   const QStringList& actions,
                                   const QVariantMap& hints, int timeout)
{
    static int id = 0;
    Q_UNUSED(actions);

    emit notificationReceived(appName, summary, body);
    return (replacesId > 0 ? replacesId : id);
}
