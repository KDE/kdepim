/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  based on localresourcecreator from kjots

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

#include "localresourcecreator.h"

#include <AkonadiCore/agentmanager.h>
#include <AkonadiCore/agentinstancecreatejob.h>
//QT5 #include "maildirsettings.h"

#include "akonadi_next/note.h"

#include <QDebug>
#include <KLocalizedString>
#include <AkonadiCore/resourcesynchronizationjob.h>

using namespace NoteShared;

LocalResourceCreator::LocalResourceCreator(QObject* parent)
    : QObject(parent)
{

}

QString LocalResourceCreator::akonadiNotesInstanceName()
{
    return QLatin1String("akonadi_akonotes_resource");
}

void LocalResourceCreator::createIfMissing()
{
    Akonadi::AgentInstance::List instances = Akonadi::AgentManager::self()->instances();
    bool found = false;
    foreach ( const Akonadi::AgentInstance& instance, instances ) {
        if (instance.type().identifier() == akonadiNotesInstanceName()) {
            found = true;
            break;
        }
    }
    if (found) {
        deleteLater();
        return;
    }
    createInstance();
}

void LocalResourceCreator::createInstance()
{
    Akonadi::AgentType notesType = Akonadi::AgentManager::self()->type( akonadiNotesInstanceName() );

    Akonadi::AgentInstanceCreateJob *job = new Akonadi::AgentInstanceCreateJob( notesType );
    connect( job, SIGNAL(result(KJob*)),
             this, SLOT(slotInstanceCreated(KJob*)) );

    job->start();
}

void LocalResourceCreator::slotInstanceCreated( KJob *job )
{
    if (job->error()) {
        qWarning() << job->errorString();
        deleteLater();
        return;
    }

    Akonadi::AgentInstanceCreateJob *createJob = qobject_cast<Akonadi::AgentInstanceCreateJob*>(job);
    Akonadi::AgentInstance instance = createJob->instance();

    instance.setName( i18nc( "Default name for resource holding notes", "Local Notes" ) );
#if 0 //QT5
    OrgKdeAkonadiMaildirSettingsInterface *iface = new OrgKdeAkonadiMaildirSettingsInterface(
                QLatin1String("org.freedesktop.Akonadi.Resource.") + instance.identifier(),
                QLatin1String("/Settings"), QDBusConnection::sessionBus(), this );

    // TODO: Make errors user-visible.
    if (!iface->isValid() ) {
        qWarning() << "Failed to obtain D-Bus interface for remote configuration.";
        delete iface;
        deleteLater();
        return;
    }
    delete iface;
#endif
    instance.reconfigure();

    Akonadi::ResourceSynchronizationJob *syncJob = new Akonadi::ResourceSynchronizationJob(instance, this);
    connect( syncJob, SIGNAL(result(KJob*)), SLOT(slotSyncDone(KJob*)));
    syncJob->start();
}

void LocalResourceCreator::slotSyncDone(KJob* job)
{
    if ( job->error() ) {
        qWarning() << "Synchronizing the resource failed:" << job->errorString();
        deleteLater();
        return;
    }

    qWarning() << "Instance synchronized";

}

void LocalResourceCreator::finishCreateResource()
{
    deleteLater();
}

