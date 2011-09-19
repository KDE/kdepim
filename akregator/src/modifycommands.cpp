/*
    This file is part of Akregator.

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
#include <krss/item.h>

#include <Akonadi/Collection>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/Session>

#include <KLocalizedString>

#include <QPointer>

using namespace Akonadi;
using namespace Akregator;

class MarkAsReadCommand::Private {
public:

    Private() : session( 0 ) {}
    KRss::FeedCollection collection;
    QPointer<Akonadi::Session> session;
};

MarkAsReadCommand::MarkAsReadCommand( QObject* parent )
    : Command( parent )
    , d( new Private )
{
    setShowErrorDialog( true );
}

void MarkAsReadCommand::setCollection( const Collection& c ) {
    d->collection = c;
}

void MarkAsReadCommand::setSession( Session* s ) {
    d->session = s;
}

void MarkAsReadCommand::doStart() {
    Q_ASSERT( d->session );

    if ( !d->collection.isValid() ) {
        setErrorText( i18n("Invalid collection.") );
        emitResult();
        return;
    }

    ItemFetchJob* job = new ItemFetchJob( d->collection, d->session );
    connect( job, SIGNAL(finished(KJob*)), this, SLOT(itemsFetched(KJob*)) );
    job->start();
}

void MarkAsReadCommand::itemsFetched( KJob* j ) {
    if ( j->error() ) {
        setError( KJob::UserDefinedError );
        setErrorText( i18n("Could not fetch items for collection %1: %2", d->collection.title(), j->errorString() ) );
        emitResult();
        return;
    }

    const ItemFetchJob * const fjob = qobject_cast<const ItemFetchJob*>( j );
    Q_ASSERT( fjob );

    Akonadi::Item::List items = fjob->items();
    if (items.isEmpty() ) {
        emitResult();
        return;
    }

    Akonadi::Item::List::Iterator it = items.begin();
    for ( ; it != items.end(); ++it )
        KRss::Item::setStatus( *it, KRss::Item::status( *it ) & ~KRss::Item::Unread );

    ItemModifyJob* mjob = new ItemModifyJob( items, d->session );
    connect( mjob, SIGNAL(finished(KJob*)), this, SLOT(itemsModified(KJob*)) );
    mjob->start();
}

void MarkAsReadCommand::itemsModified( KJob* j ) {
    if ( j->error() ) {
        setError( KJob::UserDefinedError );
        setErrorText( i18n("Could not mark items as read: %1", j->errorString() ) );
    }

    emitResult();
}
