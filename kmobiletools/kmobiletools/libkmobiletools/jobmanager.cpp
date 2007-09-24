/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "jobmanager.h"

#include <QtCore/QMultiHash>
#include <KDE/ThreadWeaver/Weaver>
#include <KDE/KGlobal>

namespace KMobileTools {

class JobManagerInstance {
public:
    JobManager m_uniqueInstance;
};

class JobManagerPrivate {
public:
    ThreadWeaver::Weaver* weaver;
    QMultiHash<QString,JobXP*> enqueuedJobs;

    QString deviceNameFromJob( JobXP* job ) {
        return enqueuedJobs.key( job, "" );
    }
};

K_GLOBAL_STATIC(JobManagerInstance, jobManagerInstance)

JobManager::JobManager()
    : d( new JobManagerPrivate )
{
    d->weaver = ThreadWeaver::Weaver::instance();
}

JobManager* JobManager::instance() {
    // instance is automatically created
    return &jobManagerInstance->m_uniqueInstance;
}

void JobManager::emitJobStarted( ThreadWeaver::Job* job ) {
    JobXP* jobXP = dynamic_cast<JobXP*>( job );
    QString deviceName = d->deviceNameFromJob( jobXP );
    if( !deviceName.isEmpty() )
        emit jobStarted( deviceName, jobXP );
}

void JobManager::emitJobDone( ThreadWeaver::Job* job ) {
    JobXP* jobXP = dynamic_cast<JobXP*>( job );
    QString deviceName = d->deviceNameFromJob( jobXP );
    if( !deviceName.isEmpty() )
        emit jobDone( deviceName, jobXP );
}

void JobManager::emitJobFailed( ThreadWeaver::Job* job ) {
    JobXP* jobXP = dynamic_cast<JobXP*>( job );
    QString deviceName = d->deviceNameFromJob( jobXP );
    if( !deviceName.isEmpty() )
        emit jobFailed( deviceName, jobXP );
}

void JobManager::enqueueJob( const QString& deviceName, JobXP* job ) {
    if( !d->enqueuedJobs.contains( deviceName, job ) ) {
        d->weaver->enqueue( job );
        d->enqueuedJobs.insert( deviceName, job );
        emit jobEnqueued( deviceName, job );

        // set-up connections
        connect( job, SIGNAL(started(ThreadWeaver::Job*)),
                 this, SLOT(emitJobStarted(ThreadWeaver::Job*)) );
        connect( job, SIGNAL(done(ThreadWeaver::Job*)),
                 this, SLOT(emitJobDone(ThreadWeaver::Job*)) );
        connect( job, SIGNAL(failed(ThreadWeaver::Job*)),
                 this, SLOT(emitJobFailed(ThreadWeaver::Job*)) );
    }
}

void JobManager::dequeueJob( JobXP* job ) {
    QString deviceName( d->deviceNameFromJob( job ) );
    if( !deviceName.isEmpty() ) {
        d->weaver->dequeue( job );
        d->enqueuedJobs.remove( deviceName, job );
        emit jobDequeued( deviceName, job );
    }
}

JobManager::~JobManager()
{
}


}

#include "jobmanager.moc"
