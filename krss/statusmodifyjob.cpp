/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>
    based on itemlistjob.cpp, Copyright (C) 2009    Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "statusmodifyjob.h"

#include <QHash>
#include <numeric>
#include <cassert>

using namespace boost;
using namespace KRss;

StatusModifyJob::StatusModifyJob( QObject* parent ) : KJob( parent ) {}

StatusModifyJob::~StatusModifyJob() {}

class CompositeStatusModifyJob::Private
{
    CompositeStatusModifyJob* q;
public:

    explicit Private( CompositeStatusModifyJob* qq ) : q( qq ) {}
    QList<StatusModifyJob*> jobs;
    QHash<KJob*,unsigned long> percentages;
    QList<Item::StatusFlag> m_setFlags;
    QList<Item::StatusFlag> m_clearFlags;

    void jobDone( KJob* job );
    void updatePercent();
    void slotPercent( KJob*, unsigned long );
    void connectJob( StatusModifyJob* );
    void doStart();
};

void CompositeStatusModifyJob::Private::updatePercent()
{
    const QList<unsigned long> v = percentages.values();
    const unsigned long p = qBound( 0, qRound( std::accumulate( v.begin(), v.end(), 0ul )
                                               / static_cast<double>( percentages.size() ) ), 100 );
    q->setPercent( p );
}

void CompositeStatusModifyJob::Private::slotPercent( KJob* job, unsigned long p )
{
    percentages.insert( job, p );
    updatePercent();
}

CompositeStatusModifyJob::CompositeStatusModifyJob( QObject * parent )
: StatusModifyJob( parent ), d( new Private( this ) )
{
}

CompositeStatusModifyJob::~CompositeStatusModifyJob()
{
    delete d;
}

void CompositeStatusModifyJob::addSubJob( StatusModifyJob* job )
{
    d->jobs.append( job );
}

void CompositeStatusModifyJob::removeSubJob( StatusModifyJob* job )
{
    d->jobs.removeAll( job );
}

void CompositeStatusModifyJob::clearFlags( const QList<KRss::Item::StatusFlag>& flags )
{
    d->m_clearFlags = flags;
}

void CompositeStatusModifyJob::setFlags( const QList<KRss::Item::StatusFlag>& flags )
{
    d->m_setFlags = flags;
}

void CompositeStatusModifyJob::Private::connectJob( StatusModifyJob* job )
{
    percentages.insert( job, 0 );
    connect( job, SIGNAL(percent(KJob*,unsigned long)),
             q, SLOT(slotPercent(KJob*,unsigned long)) );
    connect( job, SIGNAL(result(KJob*)),
             q, SLOT(jobDone(KJob*)) );
    connect( job, SIGNAL(warning(KJob*,QString,QString)),
             q, SIGNAL(warning(KJob*,QString,QString)) );
    connect( job, SIGNAL(infoMessage(KJob*,QString,QString)),
             q, SIGNAL(infoMessage(KJob*,QString,QString)) );
}

void CompositeStatusModifyJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void CompositeStatusModifyJob::Private::doStart()
{
    if ( jobs.isEmpty() ) {
        q->emitResult();
        return;
    }

    Q_FOREACH( StatusModifyJob* const i, jobs ) {
        i->clearFlags( m_clearFlags );
        i->setFlags( m_setFlags );
        connectJob( i );
    }

    Q_FOREACH( StatusModifyJob* const i, jobs )
        i->start();
}

void CompositeStatusModifyJob::Private::jobDone( KJob* j )
{
    StatusModifyJob* const job = qobject_cast<StatusModifyJob*>( j );
    assert( job && jobs.contains( job ) );

    //store first error only
    if ( job->error() && !q->error() ) {
        q->setError( job->error() );
        q->setErrorText( job->errorText() );
    }

    percentages.insert( job, 100 );
    jobs.removeOne( job );
    updatePercent();

    if ( !jobs.empty() )
        return;

    q->emitResult();
}

#include "statusmodifyjob.moc"
