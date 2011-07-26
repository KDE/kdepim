/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>

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

#include "virtualfeedlistmodel.h"
#include "krss/virtualfeedpropertiesattribute.h"

#include <Akonadi/Monitor>
#include <KLocale>

using namespace KRss;

VirtualFeedListModel::VirtualFeedListModel( const QList<VirtualFeedCollection>& virtualFeeds, QObject* parent )
    : QAbstractListModel( parent ), m_virtualFeeds( virtualFeeds )
{
      // setup the monitor
    Akonadi::Monitor* const monitor = new Akonadi::Monitor( this );
    monitor->setCollectionMonitored( Akonadi::Collection( 1 ) );
    monitor->fetchCollection( true );
    connect( monitor, SIGNAL( collectionAdded( const Akonadi::Collection&, const Akonadi::Collection& ) ),
             this, SLOT( slotCollectionAdded( const Akonadi::Collection&, const Akonadi::Collection& ) ) );
    connect( monitor, SIGNAL( collectionChanged( const Akonadi::Collection& ) ),
             this, SLOT( slotCollectionChanged( const Akonadi::Collection& ) ) );
    connect( monitor, SIGNAL( collectionRemoved( const Akonadi::Collection& ) ),
             this, SLOT( slotCollectionRemoved( const Akonadi::Collection& ) ) );
}

QVariant VirtualFeedListModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() ) {
        return QVariant();
    }

    const int row = index.row();
    if ( row < 0 || row >= m_virtualFeeds.count() ) {
        return QVariant();
    }

    const VirtualFeedCollection virtualFeed = m_virtualFeeds.at( row );
    if ( role == Qt::DisplayRole )
        return virtualFeed.title();

    if ( role == FeedIdRole )
        return virtualFeed.id();

    return QVariant();
}

int VirtualFeedListModel::rowCount( const QModelIndex &parent ) const
{
    return ( !parent.isValid() ? m_virtualFeeds.count() : 0 );
}

QVariant VirtualFeedListModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation != Qt::Horizontal || role != Qt::DisplayRole )
        return QVariant();

    if ( section == 0 )
        return i18n( "Virtual feeds" );

    return QVariant();
}

int VirtualFeedListModel::rowForFeed( const Akonadi::Collection::Id& id )
{
    int row = 0;
    Q_FOREACH( const VirtualFeedCollection& virtualFeed, m_virtualFeeds ) {
        if ( virtualFeed.id() == id )
            return row;
        else
            ++row;
    }

    return -1;
}

void VirtualFeedListModel::slotCollectionAdded( const Akonadi::Collection& collection,
                                                const Akonadi::Collection& parent )
{
    Q_ASSERT( parent.id() == 1 );
    if ( !collection.hasAttribute<VirtualFeedPropertiesAttribute>() )
        return;

    const int count = m_virtualFeeds.count();
    beginInsertRows( QModelIndex(), count, count );
    m_virtualFeeds.append( collection );
    endInsertRows();
}

void VirtualFeedListModel::slotCollectionChanged( const Akonadi::Collection& collection )
{
    // TODO: check if someone removes the attribute from an existing virtual feed
    if ( !collection.hasAttribute<VirtualFeedPropertiesAttribute>() )
        return;

    int row = 0;
    const int count = m_virtualFeeds.count();
    while( row != count ) {
        if ( m_virtualFeeds.at( row ).id() == collection.id() ) {
            m_virtualFeeds.replace( row, collection );
            const QModelIndex index = QAbstractListModel::index( row );
            emit dataChanged( index, index );
            return;
        }
    }
}

void VirtualFeedListModel::slotCollectionRemoved( const Akonadi::Collection& collection )
{
    int row = 0;
    const int count = m_virtualFeeds.count();
    while( row != count ) {
        if ( m_virtualFeeds.at( row ).id() == collection.id() ) {
            beginRemoveRows( QModelIndex(), row, row );
            m_virtualFeeds.removeAt( row );
            endRemoveRows();
            return;
        }
    }
}

#include "virtualfeedlistmodel.moc"
