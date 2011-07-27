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
        return QVariant();

    const RssItem item = akonadiItem.payload<RssItem>();
    if ( role == SortRole && column == DateColumn )
        return item.dateUpdated().toTime_t();

    if ( role == IsDeletedRole )
        return false;


    if ( role == Qt::DisplayRole || role == SortRole ) {
        switch ( column ) {
            case ItemTitleColumn:
                return item.titleAsPlainText();
            case AuthorsColumn:
            {
#if 0 //TODO: we probably want to cache this
                QString authors;
                Q_FOREACH( const KRss::Person &person, item.authors() ) {
                    if ( !authors.isEmpty() )
                        authors.append( QLatin1Char(';') );
                    authors += person.name();
                }
                return authors;
#endif
                return QVariant();
            }
            case DateColumn:
                if ( role == SortRole )
                    return item.dateUpdated().toTime_t();
                else
                    return KGlobal::locale()->formatDateTime( item.dateUpdated(),
                                                              KLocale::FancyShortDate );
            case FeedTitleForItemColumn:
#ifdef TEMPORARILY_REMOVED
               return d->m_feed->title();
#endif
            default:
                return QVariant();
        }
    }

    if ( role == ItemRole ) {
        QVariant var;
        var.setValue( akonadiItem );
        return var;
    }

    if ( role == IsImportantRole )
        return RssItem::isImportant( akonadiItem );

    if ( role == IsNewRole )
        return RssItem::isNew( akonadiItem );

    if ( role == IsUnreadRole )
        return RssItem::isUnread( akonadiItem );

    if ( role == IsReadRole )
        return RssItem::isRead( akonadiItem );

    if ( role == IsDeletedRole )
        return RssItem::isDeleted( akonadiItem );

    if ( role == LinkRole )
        return item.link();

    //PENDING(frank) TODO: use configurable colors
    if ( role == Qt::ForegroundRole ) {
        if ( RssItem::isNew( akonadiItem ) )
            return Qt::red;
        if ( RssItem::isUnread( akonadiItem ) )
            return Qt::blue;
    }

    if ( role == Qt::DecorationRole && column == ItemTitleColumn )
        return RssItem::isImportant( akonadiItem ) ? d->importantIcon : QVariant();
    return QVariant();
}

QVariant FeedItemModel::entityData( const Collection &collection, int column, int role ) const {
    if ( role == Qt::DisplayRole && column == FeedTitleColumn ) {
        const QString title = FeedCollection( collection ).title();
        if ( !title.isEmpty() )
            return title;
    }
    return EntityTreeModel::entityData( collection, column, role );
}

int FeedItemModel::entityColumnCount( EntityTreeModel::HeaderGroup headerSet ) const {
    if ( headerSet == ItemListHeaders )
        return ItemColumnCount;
    else
        return FeedColumnCount;
}

QVariant FeedItemModel::entityHeaderData( int section, Qt::Orientation orientation, int role, EntityTreeModel::HeaderGroup headerSet ) const {
    Q_ASSERT( section >= 0 );
    if ( orientation != Qt::Horizontal )
        return QVariant();
    if ( role != Qt::DisplayRole )
        return QVariant();
    switch ( headerSet ) {
        case ItemListHeaders:
        {
            if ( section >= ItemColumnCount )
                return QVariant();
            switch ( section ) {
            case ItemTitleColumn:
                return i18n("Title");
            case AuthorsColumn:
                return i18n("Author");
            case DateColumn:
                return i18n("Date");
            case FeedTitleForItemColumn:
                return i18n("Feed");
            }
        }

        case CollectionTreeHeaders:
        {
            if ( section >= FeedColumnCount )
                return QVariant();
            switch ( section ) {
            case FeedTitleColumn:
                return i18n("Title");
            case UnreadCountColumn:
                return i18n("Unread");
            case TotalCountColumn:
                return i18n("Total");
            }
        }
    }
    return QVariant();
}

