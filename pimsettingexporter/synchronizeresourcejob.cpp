/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "synchronizeresourcejob.h"

#include "messageviewer/utils/kcursorsaver.h"

#include <akonadi/resourcesynchronizationjob.h>
#include <Akonadi/AgentInstance>
#include <Akonadi/AgentManager>


#include <QStringList>
#include <QDebug>

SynchronizeResourceJob::SynchronizeResourceJob(QObject *parent)
    : QObject(parent),
      mIndex(0)
{
    MessageViewer::KCursorSaver busy( MessageViewer::KBusyPtr::busy() );
}

SynchronizeResourceJob::~SynchronizeResourceJob()
{
}

void SynchronizeResourceJob::start()
{
    if (!mListResources.isEmpty()) {
        nextSync();
    } else {
        Q_EMIT synchronizationFinished();
    }
}

void SynchronizeResourceJob::nextSync()
{
    if (mIndex < mListResources.count()) {
        const Akonadi::AgentInstance resource = Akonadi::AgentManager::self()->instance( mListResources.at(mIndex) );
        qDebug()<<" resource.name"<<resource.name();
        Akonadi::ResourceSynchronizationJob *job = new Akonadi::ResourceSynchronizationJob(resource);
        job->setCollectionTreeOnly(true);
        connect( job, SIGNAL(result(KJob*)), SLOT(slotSynchronizationFinished(KJob*)) );
        job->start();
    } else {
        Q_EMIT synchronizationFinished();
    }
}

void SynchronizeResourceJob::slotSynchronizationFinished(KJob *job)
{
    Akonadi::ResourceSynchronizationJob *resourceSync = qobject_cast<Akonadi::ResourceSynchronizationJob *>( job );
    const QString instanceName = resourceSync->resource().name();
    if (job->error()) {
        Q_EMIT synchronizationInstanceFailed(instanceName);
    } else {
        Q_EMIT synchronizationInstanceDone(instanceName);
    }
    ++mIndex;
    nextSync();
}

void SynchronizeResourceJob::setListResources(const QStringList &resources)
{
    mListResources = resources;
}

#include "moc_synchronizeresourcejob.cpp"
