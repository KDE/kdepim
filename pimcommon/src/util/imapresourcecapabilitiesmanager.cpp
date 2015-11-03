/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "imapresourcecapabilitiesmanager.h"
#include "util/pimutil.h"
#include <AkonadiCore/AgentManager>
#include <KDBusConnectionPool>
#include <QDBusInterface>
#include <qdbuspendingcall.h>
#include <qdbuspendingreply.h>
#include "pimcommon_debug.h"
using namespace PimCommon;
ImapResourceCapabilitiesManager::ImapResourceCapabilitiesManager(QObject *parent)
    : QObject(parent)
{
    init();
}

ImapResourceCapabilitiesManager::~ImapResourceCapabilitiesManager()
{

}

void ImapResourceCapabilitiesManager::slotInstanceAdded(const Akonadi::AgentInstance &instance)
{
    searchCapabilities(instance.identifier());
}

void ImapResourceCapabilitiesManager::slotInstanceRemoved(const Akonadi::AgentInstance &instance)
{
    mImapResource.remove(instance.identifier());
}

void ImapResourceCapabilitiesManager::searchCapabilities(const QString &identifier)
{
    //By default makes it as true.
    mImapResource.insert(identifier, true);
    QDBusInterface iface(
        QLatin1String("org.freedesktop.Akonadi.Resource.") + identifier,
        QStringLiteral("/"), QStringLiteral("org.kde.Akonadi.ImapResourceBase"),
        KDBusConnectionPool::threadConnection(), this);

    if (iface.isValid()) {
        QDBusPendingCall call = iface.asyncCall(QStringLiteral("serverCapabilities"));
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
        watcher->setProperty("identifier", identifier);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &ImapResourceCapabilitiesManager::slotCapabilities);
    } else {
        qCDebug(PIMCOMMON_LOG) << "interface not valid";
    }
}

void ImapResourceCapabilitiesManager::slotCapabilities(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QStringList> reply = *watcher;
    if (reply.isValid()) {
        if (watcher->property("identifier").isValid()) {
            const QStringList capabilities = reply.value();
            mImapResource.insert(watcher->property("identifier").toString(), capabilities.contains(QStringLiteral("ANNOTATEMORE")));
        }
    }
    watcher->deleteLater();
    watcher = Q_NULLPTR;
}

void ImapResourceCapabilitiesManager::init()
{
    Q_FOREACH (const Akonadi::AgentInstance &instance, Akonadi::AgentManager::self()->instances()) {
        const QString identifier = instance.identifier();
        if (PimCommon::Util::isImapResource(identifier)) {
            searchCapabilities(identifier);
        }
    }
    connect(Akonadi::AgentManager::self(), &Akonadi::AgentManager::instanceAdded, this, &ImapResourceCapabilitiesManager::slotInstanceAdded);
    connect(Akonadi::AgentManager::self(), &Akonadi::AgentManager::instanceRemoved, this, &ImapResourceCapabilitiesManager::slotInstanceRemoved);
}

bool ImapResourceCapabilitiesManager::hasAnnotationSupport(const QString &identifier) const
{
    if (!PimCommon::Util::isImapResource(identifier)) {
        return false;
    }
    return mImapResource.value(identifier, true);
}

