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

#include "itemlisting.h"
#include "feed.h"
#include "feed_p.h"
#include "item.h"
#include "item_p.h"

#include <akonadi/monitor.h>
#include <akonadi/item.h>
#include <akonadi/itemfetchscope.h>
#include <boost/bind.hpp>
#include <algorithm>

using namespace boost;
using namespace KRss;

namespace {
    static bool lessByItemId( const Item& lhs, const Item& rhs ) {
        return lhs.id() < rhs.id();
    }

    static bool sameItemId( const Item& lhs, const Item& rhs ) {
        return lhs.id() == rhs.id();
    }
}

ItemListing::Listener::~Listener() {}

class KRss::ItemListingPrivate {
    ItemListing* const q;
public:
    explicit ItemListingPrivate( const QList<Item>& list,
                                 const Akonadi::ItemFetchScope& fetchScope,
                                 ItemListing* qq );
    ~ItemListingPrivate();

    void addItems( const QList<Item>& list );
    void updateItems( const QList<Item>& list );
    void removeItems( const QList<Item>& list );

    void slotItemAdded( const Akonadi::Item& item, const Akonadi::Collection& collection )
    {
        Q_UNUSED( collection )
        Item rssItem;
        rssItem.d->akonadiItem = item;
        addItems( QList<Item>() << rssItem );
    }

    void slotItemChanged( const Akonadi::Item& item, const QSet<QByteArray>& parts )
    {
        Q_UNUSED( parts )
        Item rssItem;
        rssItem.d->akonadiItem = item;
        updateItems( QList<Item>() << rssItem );
    }

    void slotItemRemoved( const Akonadi::Item& item )
    {
        Item rssItem;
        rssItem.d->akonadiItem = item;
        removeItems( QList<Item>() << rssItem );
    }

    QList<Item> items;
    QList<ItemListing::Listener*> listeners;
    Akonadi::Monitor *m_monitor;
};

ItemListingPrivate::ItemListingPrivate( const QList<Item>& list,
                                        const Akonadi::ItemFetchScope& fetchScope,
                                        ItemListing* qq )
    : q( qq )
    , items( list ), m_monitor( new Akonadi::Monitor ) {
    std::sort( items.begin(), items.end(), lessByItemId );

    m_monitor->setItemFetchScope( fetchScope );
    QObject::connect( m_monitor, SIGNAL( itemAdded( const Akonadi::Item&, const Akonadi::Collection& ) ),
                      q, SLOT( slotItemAdded( const Akonadi::Item&, const Akonadi::Collection& ) ) );
    QObject::connect( m_monitor, SIGNAL( itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ),
                      q, SLOT( slotItemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ) );
    QObject::connect( m_monitor, SIGNAL( itemRemoved( const Akonadi::Item& ) ),
                      q, SLOT( slotItemRemoved( const Akonadi::Item& ) ) );
}

ItemListingPrivate::~ItemListingPrivate()
{
    delete m_monitor;
}

void ItemListingPrivate::addItems( const QList<Item>& list_ ) {
    if ( list_.isEmpty() )
        return;
    QList<Item> list( list_ );
    std::sort( list.begin(), list.end(), lessByItemId );
    QList<Item>::iterator from = items.begin();
    Q_FOREACH( const Item& i, list ) {
        QList<Item>::iterator it = std::lower_bound( from, items.end(), i, lessByItemId );
        const int pos = std::distance( items.begin(), it );
        if ( it != items.end() && sameItemId( *it, i ) ) {
            *it = i;
            Q_FOREACH( ItemListing::Listener* const j, listeners )
                j->update( pos );
            from = it;
        } else {
            Q_FOREACH( ItemListing::Listener* const j, listeners )
                j->prepareInsert( pos );
            from = items.insert( it, i );
            Q_FOREACH( ItemListing::Listener* const j, listeners )
                j->finishInsert( pos );
        }
    }
}

void ItemListingPrivate::updateItems( const QList<Item>& list_ ) {
    QList<Item> list( list_ );
    std::sort( list.begin(), list.end(), lessByItemId );
    QList<Item>::iterator from = items.begin();
    Q_FOREACH( const Item & i, list ) {
        const QList<Item>::iterator it = qBinaryFind( from, items.end(), i, lessByItemId );
        if ( it == items.end() )
            continue;
        *it = i;
        from = it;
        Q_FOREACH( ItemListing::Listener* const j, listeners )
            j->update( std::distance( items.begin(), it ) );
    }
}

void ItemListingPrivate::removeItems( const QList<Item>& list_ ) {
    QList<Item> list( list_ );
    std::sort( list.begin(), list.end(), lessByItemId );
    QList<Item>::iterator newEnd = items.end();
    Q_FOREACH( const Item & i, list ) {
        QList<Item>::iterator it = qBinaryFind( items.begin(), items.end(), i, lessByItemId );
        if ( it == items.end() )
            continue;
        const int pos = std::distance( items.begin(), it );
        Q_FOREACH( ItemListing::Listener* const j, listeners )
            j->prepareRemove( pos );
        items.erase( it );
        Q_FOREACH( ItemListing::Listener* const j, listeners )
            j->finishRemove( pos );
    }
}

ItemListing::ItemListing( const QList<Item>& items, const Akonadi::ItemFetchScope& fetchScope, QObject* parent )
    : QObject( parent ), d( new ItemListingPrivate( items, fetchScope, this ) ) {
}

ItemListing::~ItemListing() {
    delete d;
}

const QList<Item>& ItemListing::items() const {
    return d->items;
}

void ItemListing::connectToFeed( const shared_ptr<const Feed>& feed ) {
    d->m_monitor->setCollectionMonitored( feed->d->m_feedCollection );
}

void ItemListing::addListener( ItemListing::Listener* listener ) {
    if ( d->listeners.contains( listener ) )
        return;
    d->listeners.append( listener );
}

void ItemListing::removeListener( ItemListing::Listener* listener ) {
    d->listeners.removeAll( listener );
}

#include "itemlisting.moc"

