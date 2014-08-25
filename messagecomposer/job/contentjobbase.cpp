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

#include "contentjobbase.h"
#include "contentjobbase_p.h"


#include <QDebug>

#include <kmime/kmime_content.h>

using namespace MessageComposer;

void ContentJobBasePrivate::init( QObject *parent )
{
    Q_Q( ContentJobBase );
    ContentJobBase *parentJob = dynamic_cast<ContentJobBase*>( parent );
    if( parentJob ) {
        parentJob->appendSubjob( q );
    }
}

void ContentJobBasePrivate::doNextSubjob()
{
    Q_Q( ContentJobBase );
    if( q->hasSubjobs() ) {
        q->subjobs().first()->start();
    } else {
        qDebug() << "Calling process.";
        q->process();
    }
}



ContentJobBase::ContentJobBase( QObject *parent )
    : JobBase( *new ContentJobBasePrivate( this ), parent )
{
    Q_D( ContentJobBase );
    d->init( parent );
}

ContentJobBase::ContentJobBase( ContentJobBasePrivate &dd, QObject *parent )
    : JobBase( dd, parent )
{
    Q_D( ContentJobBase );
    d->init( parent );
}

ContentJobBase::~ContentJobBase()
{
}

void ContentJobBase::start()
{
    doStart();
}

KMime::Content *ContentJobBase::content() const
{
    Q_D( const ContentJobBase );
    //Q_ASSERT( !hasSubjobs() ); // Finished. // JobBase::hasSubjobs is not const :-/ TODO const_cast??
    Q_ASSERT( d->resultContent ); // process() should do something.
    return d->resultContent;
}

bool ContentJobBase::appendSubjob( ContentJobBase *job )
{
    job->setParent( this );
    return KCompositeJob::addSubjob( job );
}

void ContentJobBase::setExtraContent( KMime::Content* extra )
{
    Q_D( ContentJobBase );

    d->extraContent = extra;
}

KMime::Content* ContentJobBase::extraContent() const {
    Q_D( const ContentJobBase );

    return d->extraContent;
}

bool ContentJobBase::addSubjob( KJob *job )
{
    Q_UNUSED( job );
    qCritical() << "Use appendJob() instead.";
    Q_ASSERT( false );
    return false;
}

void ContentJobBase::doStart()
{
    Q_D( ContentJobBase );
    Q_ASSERT( d->resultContent == 0 && d->subjobContents.isEmpty() ); // Not started.
    Q_ASSERT( !error() ); // Jobs emitting an error in doStart should not call ContentJobBase::doStart().
    d->doNextSubjob();
}

void ContentJobBase::slotResult( KJob *job )
{
    Q_D( ContentJobBase );
    KCompositeJob::slotResult( job ); // Handles errors and removes subjob.
    qDebug() << "A subjob finished." << subjobs().count() << "more to go.";
    if( error() ) {
        return;
    }

    Q_ASSERT( dynamic_cast<ContentJobBase*>( job ) );
    ContentJobBase *cjob = static_cast<ContentJobBase*>( job );
    d->subjobContents.append( cjob->content() );
    d->doNextSubjob();
}

