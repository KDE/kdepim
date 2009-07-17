/*
    Copyright (C) 2009    Frank Osterfeld <osterfeld@kde.org>

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

#include "itemlistjob.h"
#include "item.h"

#include <akonadi/itemfetchscope.h>
#include <KDebug>
#include <QHash>

#include <boost/bind.hpp>

#include <numeric>

#include <cassert>

using namespace boost;
using namespace KRss;

ItemListJob::ItemListJob( QObject* parent ) : KJob( parent ) {}

ItemListJob::~ItemListJob() {
}

class CompositeItemListJob::Private {
    CompositeItemListJob* q;
public:

    explicit Private( CompositeItemListJob* qq ) : q( qq ), haveFilter( false ) {}
    QList<ItemListJob*> jobs;
    QList<KRss::Item> items;
    Akonadi::ItemFetchScope scope;
    QHash<KJob*,unsigned long> percentages;
    function1<bool, const Item&> filter;
    bool haveFilter;

    void jobDone( KJob* job );
    void updatePercent();
    void slotPercent(KJob*, unsigned long);
    void connectJob( ItemListJob* );
    void doEmitDone();
    void doStart();
    QList<Item> filtered( const QList<Item>& items ) const;
};

QList<Item> CompositeItemListJob::Private::filtered( const QList<Item>& items ) const {
    if ( !haveFilter )
        return items;
    QList<Item> l( items );
    l.erase( std::remove_if( l.begin(), l.end(), !bind( filter, _1 ) ), l.end() );
    return l;
}

void CompositeItemListJob::Private::updatePercent() {
    const QList<unsigned long> v = percentages.values();
    const unsigned long p = qBound( 0, qRound( std::accumulate( v.begin(), v.end(), 0ul ) / static_cast<double>( percentages.size() ) ), 100 );
    q->setPercent( p );
    kDebug() << "Load progress:" << p;
}

void CompositeItemListJob::Private::slotPercent( KJob* job, unsigned long p ) {
    percentages.insert( job, p );
    updatePercent();
}

CompositeItemListJob::CompositeItemListJob( QObject * parent ) : ItemListJob( parent ), d( new Private( this ) ) {

}

CompositeItemListJob::~CompositeItemListJob() {
    delete d;
}

void CompositeItemListJob::addSubJob( ItemListJob* job ) {
    d->jobs.append( job );
}

void CompositeItemListJob::removeSubJob( ItemListJob* job ) {
    d->jobs.removeAll( job );
}

void CompositeItemListJob::setFetchScope( const Akonadi::ItemFetchScope &fetchScope ) {
    d->scope = fetchScope;
}

Akonadi::ItemFetchScope& CompositeItemListJob::fetchScope() {
    return d->scope;
}

QList<KRss::Item> CompositeItemListJob::items() const {
    return d->items;
}

void CompositeItemListJob::Private::connectJob( ItemListJob* job ) {
    percentages.insert( job, 0 );
    connect( job, SIGNAL(percent(KJob*,unsigned long)),
             q, SLOT(slotPercent(KJob*,unsigned long)) );
    connect( job, SIGNAL(finished(KJob*)),
             q, SLOT(jobDone(KJob*)) );
    connect( job, SIGNAL(warning(KJob*,QString,QString)),
             q, SIGNAL(warning(KJob*,QString,QString)) );
    connect( job, SIGNAL(infoMessage(KJob*,QString,QString)),
             q, SIGNAL(infoMessage(KJob*,QString,QString)) );
}

void CompositeItemListJob::setFilter( const function1<bool, const Item&>& filter ) {
    d->haveFilter = true;
    d->filter = filter;
}

void CompositeItemListJob::clearFilter() {
    d->haveFilter = false;
}

void CompositeItemListJob::start() {
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void CompositeItemListJob::Private::doStart() {
    if ( jobs.isEmpty() ) {
        q->emitResult();
        return;
    }

    if ( !scope.isEmpty() )
        Q_FOREACH( ItemListJob* const i, jobs )
            i->setFetchScope( scope );

    Q_FOREACH( ItemListJob* const i, jobs )
        connectJob( i );

    Q_FOREACH( ItemListJob* const i, jobs )
        i->start();
}

void CompositeItemListJob::Private::jobDone( KJob* j ) {
    ItemListJob* job = qobject_cast<ItemListJob*>( j );
    assert( job && jobs.contains( job ) );

    //store first error only
    if ( job->error() && !q->error() ) {
        q->setError( job->error() );
        q->setErrorText( job->errorText() );
    }

    percentages.insert( job, 100 );
    items << filtered( job->items() );

    jobs.removeAll( job );

    updatePercent();

    if ( !jobs.empty() )
        return;

    q->emitResult();
}

#include "itemlistjob.moc"
