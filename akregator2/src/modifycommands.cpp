/*
    This file is part of Akregator2.

    Copyright (C) 2009 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "modifycommands.h"

#include <krss/feedcollection.h>
#include <KRss/Item>

#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/Session>

#include <KLocalizedString>

#include <QPointer>

#include <algorithm>

using namespace Akonadi;
using namespace Akregator2;

class MarkAsReadCommand::Private {
public:

    Private() : pendingFetches( 0 ), pendingModifies( 0 ) {}
    Collection::List collections;
    QPointer<Akonadi::Session> session;
    int pendingFetches;
    int pendingModifies;
};

MarkAsReadCommand::MarkAsReadCommand( QObject* parent )
    : Command( parent )
    , d( new Private )
{
    setShowErrorDialog( true );
    setUserVisible( false );
}

MarkAsReadCommand::~MarkAsReadCommand()
{
    delete d;
}

void MarkAsReadCommand::setCollections( const Collection::List& c ) {
    d->collections = c;
}

void MarkAsReadCommand::setSession( Session* s ) {
    d->session = s;
}

void MarkAsReadCommand::doStart() {
    Q_ASSERT( d->session );

    Q_FOREACH ( const Collection& i, d->collections )
        if ( !i.isValid() ) {
            setError( KJob::UserDefinedError );
            setErrorText( i18n("Invalid collection.") );
            emitResult();
            return;
        }
    CollectionFetchJob* job = new CollectionFetchJob( d->collections, CollectionFetchJob::Recursive, d->session );
    connect( job, SIGNAL(finished(KJob*)), this, SLOT(collectionsFetched(KJob*)) );
    job->start();
}

void MarkAsReadCommand::collectionsFetched( KJob* j ) {
    if ( j->error() ) {
        setError( KJob::UserDefinedError );
        setErrorText( i18n("Could not fetch collection for mark as read: %1", j->errorString() ) );
        emitResult();
        return;
    }

    const CollectionFetchJob * const fjob = qobject_cast<const CollectionFetchJob*>( j );
    Q_ASSERT( fjob );

    const Collection::List l = d->collections + fjob->collections();

    if ( l.isEmpty() ) {
        emitResult();
        return;
    }

    Q_FOREACH( const Collection& i, l ) {
        ItemFetchJob* job = new ItemFetchJob( i, d->session );
        job->fetchScope().fetchFullPayload( false );
        connect( job, SIGNAL(finished(KJob*)), this, SLOT(itemsFetched(KJob*)) );
        ++d->pendingFetches;
        job->start();
    }
}

struct ItemIsRead {
    bool operator()( const Akonadi::Item& item ) const {
        return !KRss::Item::isUnread( item );
    }
};


void MarkAsReadCommand::itemsFetched( KJob* j ) {
    --d->pendingFetches;
    Q_ASSERT( d->pendingFetches >= 0 );

    if ( j->error() ) {
        setError( KJob::UserDefinedError );
        setErrorText( i18n("Could not fetch items for collection: %1", j->errorString() ) );
    } else {
        const ItemFetchJob * const fjob = qobject_cast<const ItemFetchJob*>( j );
        Q_ASSERT( fjob );
        Akonadi::Item::List items = fjob->items();

        items.erase( std::remove_if( items.begin(), items.end(), ItemIsRead() ), items.end() );
        if ( !items.isEmpty() ) {
            Akonadi::Item::List::Iterator it = items.begin();
            for ( ; it != items.end(); ++it )
                it->setFlag( KRss::Item::flagRead() );
            ItemModifyJob* mjob = new ItemModifyJob( items, d->session );
            mjob->setIgnorePayload( true );
            connect( mjob, SIGNAL(finished(KJob*)), this, SLOT(itemsModified(KJob*)) );
            ++d->pendingModifies;
            mjob->start();
        }
    }

    if ( d->pendingFetches == 0 && d->pendingModifies == 0 )
        emitResult();
}

void MarkAsReadCommand::itemsModified( KJob* j ) {
    --d->pendingModifies;
    Q_ASSERT( d->pendingModifies >= 0 );

    if ( j->error() ) {
        setError( KJob::UserDefinedError );
        setErrorText( i18n("Could not mark items as read: %1", j->errorString() ) );
    }

    if ( d->pendingFetches == 0 && d->pendingModifies == 0 )
        emitResult();
}
