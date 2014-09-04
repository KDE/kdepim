/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include "archivemailagent.h"
#include "archivemailagentadaptor.h"
#include "archivemaildialog.h"
#include "archivemailmanager.h"
#include "archivemailagentsettings.h"

#include <mailcommon/kernel/mailkernel.h>
#include <AkonadiCore/dbusconnectionpool.h>
#include <Monitor>
#include <Session>
#include <CollectionFetchScope>
#include <KMime/Message>
#include <KWindowSystem>

#include <QTimer>
#include <QPointer>

#include <kdelibs4configmigrator.h>

//#define DEBUG_ARCHIVEMAILAGENT 1

ArchiveMailAgent::ArchiveMailAgent(const QString &id)
    : Akonadi::AgentBase(id)
{
    Kdelibs4ConfigMigrator migrate(QLatin1String("archivemailagent"));
    migrate.setConfigFiles(QStringList() << QLatin1String("akonadi_archivemail_agentrc") << QLatin1String("akonadi_archivemail_agent.notifyrc"));
    migrate.migrate();

    mArchiveManager = new ArchiveMailManager(this);
    connect(mArchiveManager, SIGNAL(needUpdateConfigDialogBox()), SIGNAL(needUpdateConfigDialogBox()));

    Akonadi::Monitor *collectionMonitor = new Akonadi::Monitor(this);
    collectionMonitor->fetchCollection(true);
    collectionMonitor->ignoreSession(Akonadi::Session::defaultSession());
    collectionMonitor->collectionFetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
    collectionMonitor->setMimeTypeMonitored(KMime::Message::mimeType());

    new ArchiveMailAgentAdaptor(this);
    Akonadi::DBusConnectionPool::threadConnection().registerObject(QLatin1String("/ArchiveMailAgent"), this, QDBusConnection::ExportAdaptors);
    Akonadi::DBusConnectionPool::threadConnection().registerService(QLatin1String("org.freedesktop.Akonadi.ArchiveMailAgent"));
    connect(collectionMonitor, SIGNAL(collectionRemoved(Akonadi::Collection)),
            this, SLOT(mailCollectionRemoved(Akonadi::Collection)));

    if (enabledAgent()) {
#ifdef DEBUG_ARCHIVEMAILAGENT
        QTimer::singleShot(1000, mArchiveManager, SLOT(load()));
#else
        QTimer::singleShot(1000 * 60 * 5, mArchiveManager, SLOT(load()));
#endif
    }

    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(reload()));
    mTimer->start(24 * 60 * 60 * 1000);
}

ArchiveMailAgent::~ArchiveMailAgent()
{
}

void ArchiveMailAgent::setEnableAgent(bool enabled)
{
    if (enabled != ArchiveMailAgentSettings::enabled()) {
        ArchiveMailAgentSettings::setEnabled(enabled);
        ArchiveMailAgentSettings::self()->save();
        if (!enabled) {
            mTimer->stop();
            pause();
        } else {
            mTimer->start();
        }
    }
}

bool ArchiveMailAgent::enabledAgent() const
{
    return ArchiveMailAgentSettings::enabled();
}

void ArchiveMailAgent::mailCollectionRemoved(const Akonadi::Collection &collection)
{
    mArchiveManager->removeCollection(collection);
}

void ArchiveMailAgent::showConfigureDialog(qlonglong windowId)
{
    QPointer<ArchiveMailDialog> dialog = new ArchiveMailDialog();
    if (windowId) {
#ifndef Q_OS_WIN
        KWindowSystem::setMainWindow(dialog, windowId);
#else
        KWindowSystem::setMainWindow(dialog, (HWND)windowId);
#endif
    }
    connect(dialog, SIGNAL(archiveNow(ArchiveMailInfo*)), mArchiveManager, SLOT(slotArchiveNow(ArchiveMailInfo*)));
    connect(this, SIGNAL(needUpdateConfigDialogBox()), dialog, SLOT(slotNeedReloadConfig()));
    if (dialog->exec()) {
        mArchiveManager->load();
    }
    delete dialog;
}

void ArchiveMailAgent::doSetOnline(bool online)
{
    if (online) {
        resume();
    } else {
        pause();
    }
}

void ArchiveMailAgent::reload()
{
    if (isOnline() && enabledAgent()) {
        mArchiveManager->load();
        mTimer->start();
    }
}

void ArchiveMailAgent::configure(WId windowId)
{
    showConfigureDialog((qulonglong)windowId);
}

void ArchiveMailAgent::pause()
{
    if (isOnline() && enabledAgent()) {
        mArchiveManager->pause();
    }
}

void ArchiveMailAgent::resume()
{
    if (isOnline() && enabledAgent()) {
        mArchiveManager->resume();
    }
}

QString ArchiveMailAgent::printArchiveListInfo()
{
    return mArchiveManager->printArchiveListInfo();
}

QString ArchiveMailAgent::printCurrentListInfo()
{
    return mArchiveManager->printCurrentListInfo();
}

void ArchiveMailAgent::archiveFolder(const QString &path, Akonadi::Collection::Id collectionId)
{
    mArchiveManager->archiveFolder(path, collectionId);
}

AKONADI_AGENT_MAIN(ArchiveMailAgent)

