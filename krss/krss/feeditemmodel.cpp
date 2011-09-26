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

#include "feedcollection.h"
#include "feeditemmodel.h"
#include "rssitem.h"
#include "person.h"

#include <KDateTime>
#include <KGlobal>
#include <KIcon>
#include <KLocale>

using namespace Akonadi;
using namespace KRss;

class KRss::FeedItemModelPrivate {
    FeedItemModel* const q;
public:
    explicit FeedItemModelPrivate( FeedItemModel* qq ) : q( qq ), importantIcon( KIcon( QLatin1String("mail-mark-important") ) ) {

    }

    KIcon importantIcon;
};


FeedItemModel::FeedItemModel( ChangeRecorder* monitor, QObject* parent ) : EntityTreeModel( monitor, parent ), d( new FeedItemModelPrivate( this ) ) {
    setItemPopulationStrategy( EntityTreeModel::LazyPopulation );
}

FeedItemModel::~FeedItemModel() {
    delete d;
}

QVariant FeedItemModel::entityData( const Akonadi::Item &akonadiItem, int column, int role ) const {
    if ( !akonadiItem.hasPayload<RssItem>() )
        return EntityTreeModel::entityData( akonadiItem, column, role );

    const RssItem item = akonadiItem.payload<RssItem>();
    if ( role == SortRole && column == DateColumn )
        return item.dateUpdated().toTime_t();

    if ( role == IsDeletedRole )
        return KRss::RssItem::isDeleted( akonadiItem );

    if ( role == Qt::DisplayRole || role == SortRole ) {
        switch ( column ) {
            case ItemTitleColumn:
                return item.titleAsPlainText();
            case AuthorsColumn:
            {
                QString authors;
                Q_FOREACH( const KRss::Person &person, item.authors() ) {
                    if ( !authors.isEmpty() )
                        authors.append( QLatin1Char(';') );
                    authors += person.condensedPlainText();
                }
                return authors;
            }
            case DateColumn:
                if ( role == SortRole )
                    return item.dateUpdated().toTime_t();
                else
                    return KGlobal::locale()->formatDateTime( item.dateUpdated(),
                                                              KLocale::FancyShortDate );
            case FeedTitleForItemColumn:
                return FeedCollection( akonadiItem.parentCollection() ).title();
            default:
                return EntityTreeModel::entityData( akonadiItem, column, role );
        }
    }

    switch ( role ) {
        case IsImportantRole:
            return RssItem::isImportant( akonadiItem );
        case IsUnreadRole:
            return RssItem::isUnread( akonadiItem );
        case IsReadRole:
            return RssItem::isRead( akonadiItem );
        case IsDeletedRole:
            return RssItem::isDeleted( akonadiItem );
        case LinkRole:
            return item.link();
    default:
        break;
    }
    //PENDING(frank) TODO: use configurable colors
    if ( role == Qt::ForegroundRole ) {
        if ( RssItem::isUnread( akonadiItem ) )
            return Qt::blue;
    }

    if ( role == Qt::DecorationRole && column == ItemTitleColumn && RssItem::isImportant( akonadiItem ) )
        return d->importantIcon;

    return EntityTreeModel::entityData( akonadiItem, column, role );
}

QVariant FeedItemModel::entityData( const Collection &collection, int column, int role ) const {
    if ( role == Qt::DisplayRole || role == SortRole ) {
        switch ( column ) {
        case FeedTitleColumn:
        {
            const QString title = FeedCollection( collection ).title();
            if ( !title.isEmpty() )
                return title;
            break;
        }
        case UnreadCountColumn:
        {
            return EntityTreeModel::entityData( collection, column, EntityTreeModel::UnreadCountRole );
        }
        case IsFolderRole:
            return FeedCollection( collection ).isFolder();
        default:
            break;
        }
    }
    return EntityTreeModel::entityData( collection, column, role );
}

int FeedItemModel::entityColumnCount( EntityTreeModel::HeaderGroup headerSet ) const {
    switch ( headerSet ) {
    case ItemListHeaders:
        return ItemColumnCount;
    case CollectionTreeHeaders:
        return FeedColumnCount;
    default:
        break;
    }
    return EntityTreeModel::entityColumnCount( headerSet );
}

QVariant FeedItemModel::entityHeaderData( int section, Qt::Orientation orientation, int role, EntityTreeModel::HeaderGroup headerSet ) const {
    Q_ASSERT( section >= 0 );
    if ( orientation != Qt::Horizontal || role != Qt::DisplayRole )
        return EntityTreeModel::entityHeaderData( section, orientation, role, headerSet );
    switch ( headerSet ) {
        case ItemListHeaders:
        {
            switch ( section ) {
            case ItemTitleColumn:
                return i18n("Title");
            case AuthorsColumn:
                return i18n("Author");
            case DateColumn:
                return i18n("Date");
            case FeedTitleForItemColumn:
                return i18n("Feed");
            default:
                break;
            }
        }

        case CollectionTreeHeaders:
        {
          switch ( section ) {
            case FeedTitleColumn:
                return i18n("Title");
            case UnreadCountColumn:
                return i18n("Unread");
            case TotalCountColumn:
                return i18n("Total");
            default:
                break;
            }
        }
        default:
            break;
    }
    return EntityTreeModel::entityHeaderData( section, orientation, role, headerSet );
}

