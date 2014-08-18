/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "monitorsmodel.h"
#include "monitoritem.h"



#include <AkonadiCore/servermanager.h>

#include <QtDBus/QDBusConnection>


MonitorsModel::MonitorsModel(QObject* parent):
  QAbstractItemModel( parent ), mManager( 0 )
{
  QTimer::singleShot( 0, this, SLOT(init()) );
}

MonitorsModel::~MonitorsModel()
{
  qDeleteAll( mData );
  mData.clear();
}

void MonitorsModel::init()
{
  qDeleteAll( mData );
  mData.clear();

  QString service = QLatin1String( "org.freedesktop.Akonadi" );
  if ( Akonadi::ServerManager::hasInstanceIdentifier() ) {
    service += "." + Akonadi::ServerManager::instanceIdentifier();
  }

  mManager = new org::freedesktop::Akonadi::NotificationManager( service,
    QLatin1String( "/notifications" ), QDBusConnection::sessionBus(), this );
  if ( !mManager ) {
    qWarning() << "Failed to connect to org.freedesktop.Akonadi.NotificationManager";
    return;
  }

  const QStringList subscribers = mManager->subscribers();
  Q_FOREACH ( const QString &subscriber, subscribers ) {
    slotSubscriberSubscribed( subscriber );
  }

  connect( mManager, SIGNAL(subscribed(QString)),
           this, SLOT(slotSubscriberSubscribed(QString)) );
  connect( mManager, SIGNAL(unsubscribed(QString)),
           this, SLOT(slotSubscriberUnsubscribed(QString)) );
}

void MonitorsModel::slotSubscriberSubscribed( const QString &identifier )
{
  // Avoid akonadiconsole's Monitors being duplicated on startup
  if ( mData.contains( identifier ) ) {
    return;
  }

  MonitorItem *item = new MonitorItem( identifier, this );
  connect( item, SIGNAL(changed(MonitorsModel::Column)),
           this, SLOT(slotItemChanged(MonitorsModel::Column)) );
  beginInsertRows( QModelIndex(), mData.count(), mData.count() );
  mData.insert( identifier, item );
  endInsertRows();
}

void MonitorsModel::slotSubscriberUnsubscribed( const QString &identifier )
{
  if ( !mData.contains( identifier) ) {
    return;
  }

  const int row = mData.uniqueKeys().indexOf( identifier );
  beginRemoveRows( QModelIndex(), row, row );
  mData.take( identifier )->deleteLater();
  endRemoveRows();
}

void MonitorsModel::slotItemChanged( MonitorsModel::Column column )
{
  MonitorItem *item = qobject_cast<MonitorItem*>( sender() );
  const QModelIndex idx = index( mData.uniqueKeys().indexOf( item->identifier ), static_cast<int>( column ) );
  Q_EMIT dataChanged( idx, idx );
}

QVariant MonitorsModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role == Qt::DisplayRole ) {
    if ( orientation == Qt::Horizontal ) {
      switch ( section ) {
        case 0: return QLatin1String( "Subscriber" );
        case 1: return QLatin1String( "All Monitored" );
        case 2: return QLatin1String( "Monitored Collections" );
        case 3: return QLatin1String( "Monitored Items" );
        case 4: return QLatin1String( "Monitored Resources" );
        case 5: return QLatin1String( "Monitored Mime Types" );
        case 6: return QLatin1String( "Ignored Sessions" );
      }
    }
  }

  return QVariant();
}

QVariant MonitorsModel::data( const QModelIndex &index, int role ) const
{
  if ( role != Qt::DisplayRole ) {
    return QVariant();
  }

  MonitorItem *item = static_cast<MonitorItem*>( index.internalPointer() );
  switch ( index.column() ) {
    case IdentifierColumn: return item->identifier;
    case IsAllMonitoredColumn: return item->allMonitored;
    case MonitoredCollectionsColumn: return item->monitoredCollections;
    case MonitoredItemsColumn: return item->monitoredItems;
    case MonitoredResourcesColumn: return item->monitoredResources;
    case MonitoredMimeTypesColumn: return item->monitoredMimeTypes;
    case IgnoredSessionsColumn: return item->ignoredSessions;
  }

  return QVariant();
}

int MonitorsModel::columnCount( const QModelIndex &parent ) const
{
  return ColumnsCount;
}

int MonitorsModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() ) {
    return 0;
  }

  return mData.count();
}

QModelIndex MonitorsModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child );

  return QModelIndex();
}

QModelIndex MonitorsModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( row >= mData.uniqueKeys().count() ) {
    return QModelIndex();
  }

  const QString key = mData.uniqueKeys().at( row );
  return createIndex( row, column, mData.value( key ) );
}

