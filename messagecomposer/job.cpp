/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "job.h"
#include "job_p.h"

#include "composer.h"

#include <QTimer>

#include <KDebug>

#include <kmime/kmime_content.h>

using namespace MessageComposer;
using namespace KMime;

void JobPrivate::init( QObject *parent )
{
  Composer *parentComposer = dynamic_cast<Composer*>( parent );
  if( parentComposer ) {
    composer = parentComposer;
    return;
  }

  Job *parentJob = dynamic_cast<Job*>( parent );
  if( parentJob ) {
    composer = parentJob->d_ptr->composer;
    parentJob->addSubjob( q_ptr );
    return;
  }

  composer = 0;
}

void JobPrivate::doNextSubjob()
{
  Q_Q( Job );
  if( q->hasSubjobs() ) {
    q->subjobs().first()->start();
  } else {
    kDebug() << "Calling process.";
    q->process();
  }
}



Job::Job( QObject *parent )
  : KCompositeJob( parent )
  , d_ptr( new JobPrivate( this ) )
{
  d_ptr->init( parent );
}

Job::Job( JobPrivate &dd, QObject *parent )
  : KCompositeJob( parent )
  , d_ptr( &dd )
{
  d_ptr->init( parent );
}

Job::~Job()
{
  delete d_ptr;
}

void Job::start()
{
  QTimer::singleShot( 0, this, SLOT(doStart()) );
}

Content *Job::content() const
{
  Q_D( const Job );
  //Q_ASSERT( !hasSubjobs() ); // Finished. // KCompositeJob::hasSubjobs is not const :-/
  Q_ASSERT( d->resultContent ); // process() should do something.
  return d->resultContent;
}

void Job::doStart()
{
  Q_D( Job );
  Q_ASSERT( d->resultContent == 0 && d->subjobContents.isEmpty() ); // Not started.
  Q_ASSERT( !error() ); // Jobs emitting an error in doStart should not call Job::doStart().
  d->doNextSubjob();
}

void Job::slotResult( KJob *job )
{
  Q_D( Job );
  KCompositeJob::slotResult( job ); // Handles errors and removes subjob.
  kDebug() << "A subjob finished." << subjobs().count() << "more to go.";
  if( error() ) {
    return;
  }

  Q_ASSERT( dynamic_cast<Job*>( job ) );
  Job *cjob = static_cast<Job*>( job );
  d->subjobContents.append( cjob->content() );
  d->doNextSubjob();
}

void Job::setComposer( Composer *composer )
{
  d_ptr->composer = composer;
}

#include "job.moc"
