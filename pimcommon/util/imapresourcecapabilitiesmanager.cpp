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
#include <Akonadi/AgentManager>
#include <Akonadi/DBusConnectionPool>
#include <QDBusInterface>
#include <qdbuspendingcall.h>
#include <qdbuspendingreply.h>
#include <QDebug>

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
                QLatin1String( "org.freedesktop.Akonadi.Resource.")+ identifier,
                QLatin1String( "/" ), QLatin1String( "org.kde.Akonadi.ImapResourceBase" ),
                Akonadi::DBusConnectionPool::threadConnection(), this );
    if (iface.isValid()) {
        QDBusPendingCall call = iface.asyncCall( QLatin1String( "serverCapabilities" ) );
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
        watcher->setProperty("identifier", identifier);
        connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(slotCapabilities(QDBusPendingCallWatcher*)));
    } else {
        qDebug() << "interface not valid";
    }
}

void ImapResourceCapabilitiesManager::slotCapabilities(QDBusPendingCallWatcher* watcher)
{
    QDBusPendingReply<QStringList> reply = *watcher;
    if ( reply.isValid() ) {
        if (watcher->property("identifier").isValid()) {
            const QStringList capabilities = reply.value();
            //qDebug()<<" capabilities"<<capabilities<< " for identifier"<<watcher->property("identifier").toString();
            mImapResource.insert(watcher->property("identifier").toString(), capabilities.contains(QLatin1String("ANNOTATEMORE")));
        }
    }
    watcher->deleteLater();
    watcher = 0;
}

void ImapResourceCapabilitiesManager::init()
{
    Q_FOREACH ( const Akonadi::AgentInstance &instance, Akonadi::AgentManager::self()->instances() ) {
        const QString identifier = instance.identifier();
        if (PimCommon::Util::isImapResource(identifier)) {
            searchCapabilities(identifier);
        }
    }
    connect( Akonadi::AgentManager::self(), SIGNAL(instanceAdded(Akonadi::AgentInstance)), SLOT(slotInstanceAdded(Akonadi::AgentInstance)) );
    connect( Akonadi::AgentManager::self(), SIGNAL(instanceRemoved(Akonadi::AgentInstance)), SLOT(slotInstanceRemoved(Akonadi::AgentInstance)) );
}

bool ImapResourceCapabilitiesManager::hasAnnotationSupport(const QString &identifier) const
{
    if (!PimCommon::Util::isImapResource(identifier)) {
        return false;
    }
    return mImapResource.value(identifier, true);
}

