/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

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

#include "itemmodel.h"
#include "feed.h"
#include "person.h"
#include "itemlisting.h"
#include "itemlistjob.h"

#include <akonadi/itemfetchscope.h>

#include <KIcon>
#include <KGlobal>
#include <KLocale>
#include <KDateTime>
#include <KDebug>

#include <QtCore/QTimer>

#include <cassert>

using namespace boost;
using namespace KRss;

class KRss::ItemModelPrivate  : public ItemListing::Listener {
    ItemModel* const q;

public:
    explicit ItemModelPrivate( const shared_ptr<ItemListing>& coll, ItemModel* qq );
    ~ItemModelPrivate();

    void prepareInsert( int idx ) {
        q->beginInsertRows( QModelIndex(), idx, idx );
    }

    void finishInsert( int ) {
        q->endInsertRows();
    }

    void prepareRemove( int idx ) {
        q->beginRemoveRows( QModelIndex(), idx, idx );
    }

    void finishRemove( int ) {
        q->endRemoveRows();
    }

    void update( int idx ) {
        emit q->dataChanged( q->index( idx, ItemModel::ItemTitleColumn ), q->index( idx, ItemModel::ColumnCount - 1 ) );
    }

    const shared_ptr<ItemListing> items;
    KIcon m_importantIcon;
};

ItemModelPrivate::ItemModelPrivate( const shared_ptr<ItemListing>& items_, ItemModel* qq )
    : q( qq ), items( items_ ), m_importantIcon( KIcon( "mail-mark-important" ) )
{
    items->addListener( this );
}

ItemModelPrivate::~ItemModelPrivate() {
    items->removeListener( this );
}


ItemModel::ItemModel( const shared_ptr<ItemListing>& coll, QObject *parent )
    : QAbstractTableModel( parent ), d( new ItemModelPrivate( coll, this ) )
{
}

ItemModel::~ItemModel()
{
    delete d;
}

Item ItemModel::itemForIndex( const QModelIndex &index ) const
{
    const int row = index.row();
    if ( row < 0 || row >= d->items->items().count() )
        return Item();

    return d->items->items().at( row );
}

int ItemModel::columnCount( const QModelIndex &parent ) const
{
    return ( !parent.isValid() ? 4 : 0 );
}

QVariant ItemModel::data( const QModelIndex &index, int role ) const
{
    if ( !index.isValid() ) {
        return QVariant();
    }

    const int row = index.row();

    if ( row < 0 || row >= d->items->items().count() ) {
        return QVariant();
    }

    const Item& item = d->items->items()[row];

    if ( role == SortRole && index.column() == DateColumn )
        return item.dateUpdated().toTime_t();

    if ( role == IsDeletedRole )
        return false;

    if ( role == Qt::DisplayRole || role == SortRole ) {
        QString authors;
        switch ( index.column() ) {
            case ItemTitleColumn:
                return item.titleAsPlainText();
            case AuthorsColumn:
                Q_FOREACH( const KRss::Person &person, item.authors() ) {
                    authors += person.name() + ';';
                }
                authors.remove( authors.length() - 1, 1 );
                return authors;
            case DateColumn:
                return KGlobal::locale()->formatDateTime( item.dateUpdated(),
                                                          KLocale::FancyShortDate );
            case FeedTitleColumn:
#ifdef TEMPORARILY_REMOVED
               return d->m_feed->title();
#endif
            default:
                return QVariant();
        }
    }

    if ( role == ItemRole ) {
        QVariant var;
        var.setValue( item );
        return var;
    }

    if ( role == ItemStatusRole ) {
        QVariant var;
        var.setValue( item.status() );
        return var;
    }

    if ( role == IsImportantRole )
        return item.isImportant();

    if ( role == IsNewRole )
        return item.isNew();

    if ( role == IsUnreadRole )
        return item.isUnread();

    if ( role == IsReadRole )
        return item.isRead();

    if ( role == IsDeletedRole )
        return item.isDeleted();

    if ( role == LinkRole )
        return item.link();

    //PENDING(frank) TODO: use configurable colors
    if ( role == Qt::ForegroundRole ) {
        if ( item.isNew() )
            return Qt::red;
        if ( item.isUnread() )
            return Qt::blue;
    }

    if ( role == Qt::DecorationRole && index.column() == ItemTitleColumn )
        return item.isImportant() ? d->m_importantIcon : QVariant();

    return QVariant();
}

int ItemModel::rowCount( const QModelIndex &parent ) const
{
    return ( !parent.isValid() ? d->items->items().count() : 0 );
}

QVariant ItemModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation != Qt::Horizontal || role != Qt::DisplayRole ) {
        return QVariant();
    }

    switch ( section ) {
        case ItemTitleColumn:
            return i18n( "Title" );
        case AuthorsColumn:
            return i18n( "Authors" );
        case DateColumn:
            return i18n( "Date" );
        case FeedTitleColumn:
            return i18n( "Feed" );
        default:
            return QVariant();
    }

    return QVariant();
}

#include "itemmodel.moc"
