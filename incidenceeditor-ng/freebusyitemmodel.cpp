/*
  Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
  Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "freebusyitemmodel.h"

#include <Akonadi/Calendar/freebusymanager.h>

#include <KDebug>
#include <KGlobal>
#include <KLocalizedString>
#include <KLocale>

#include <QTimerEvent>

using namespace IncidenceEditorNG;

namespace IncidenceEditorNG {

class ItemPrivateData
{
  public:
    ItemPrivateData( ItemPrivateData *parent ) : parentItem( parent )
    {
    }

    ~ItemPrivateData()
    {
      qDeleteAll( childItems );
    }

    ItemPrivateData *child( int row )
    {
      return childItems.value( row );
    }

    void appendChild( ItemPrivateData *item )
    {
      childItems.append(item);
    }

    ItemPrivateData *removeChild( int row )
    {
      return childItems.takeAt( row );
    }

    int childCount() const
    {
      return childItems.count();
    }

    int row() const
    {
      if ( parentItem ) {
        return parentItem->childItems.indexOf( const_cast<ItemPrivateData*>(this) );
      }
      return 0;
    }

    ItemPrivateData *parent()
    {
      return parentItem;
    }

  private:
    QList<ItemPrivateData*> childItems;
    ItemPrivateData *parentItem;
};

}

FreeBusyItemModel::FreeBusyItemModel( QObject *parent )
  : QAbstractItemModel( parent ), mForceDownload( false )
{
  qRegisterMetaType<KCalCore::Attendee::Ptr>( "KCalCore::Attendee::Ptr" );
  qRegisterMetaType<KCalCore::FreeBusy::Ptr>( "KCalCore::FreeBusy::Ptr" );
  qRegisterMetaType<KCalCore::Period>( "KCalCore::Period" );

  Akonadi::FreeBusyManager *m = Akonadi::FreeBusyManager::self();
  connect( m, SIGNAL(freeBusyRetrieved(KCalCore::FreeBusy::Ptr,QString)),
           SLOT(slotInsertFreeBusy(KCalCore::FreeBusy::Ptr,QString)) );

  connect( &mReloadTimer, SIGNAL(timeout()), SLOT(autoReload()) );
  mReloadTimer.setSingleShot( true );

  mRootData = new ItemPrivateData( 0 );
}

FreeBusyItemModel::~FreeBusyItemModel()
{
  delete mRootData;
}

QVariant FreeBusyItemModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() ) {
    return QVariant();
  }

  ItemPrivateData *data = (ItemPrivateData*) index.internalPointer();

  if ( data->parent() == mRootData ) {
    int row = index.row();
    if ( row >= mFreeBusyItems.size() ) {
      return QVariant();
    }

    switch( role ) {
    case Qt::DisplayRole:
      return mFreeBusyItems.at( row )->attendee()->fullName();
    case FreeBusyItemModel::AttendeeRole:
      return QVariant::fromValue( mFreeBusyItems.at( row )->attendee() );
    case FreeBusyItemModel::FreeBusyRole:
      if ( mFreeBusyItems.at( row )->freeBusy() ) {
        return QVariant::fromValue( mFreeBusyItems.at( row )->freeBusy() );
      } else {
        return QVariant();
      }
    default:
      return QVariant();
    }
  }

  FreeBusyItem::Ptr fbitem = mFreeBusyItems.at( data->parent()->row() );
  if ( !fbitem->freeBusy() || index.row() >= fbitem->freeBusy()->busyPeriods().size() ) {
    return QVariant();
  }

  KCalCore::FreeBusyPeriod period = fbitem->freeBusy()->fullBusyPeriods().at( index.row() );
  switch( role ) {
  case Qt::DisplayRole: // return something to make modeltest happy
    return QString( "%1 - %2" ).
      arg( KGlobal::locale()->formatDateTime( period.start() ) ).
      arg( KGlobal::locale()->formatDateTime( period.end() ) );
  case FreeBusyItemModel::FreeBusyPeriodRole:
    return QVariant::fromValue( period );
  default:
    return QVariant();
  }
}

int FreeBusyItemModel::rowCount( const QModelIndex &parent ) const
{
  ItemPrivateData *parentData;
  if ( parent.column() > 0 ) {
    return 0;
  }

  if ( !parent.isValid() ) {
    parentData = mRootData;
  } else {
    parentData = static_cast<ItemPrivateData*>( parent.internalPointer() );
  }

  return parentData->childCount();
}

int FreeBusyItemModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 1;
}

QModelIndex FreeBusyItemModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) ) {
    return QModelIndex();
  }

  ItemPrivateData *parentData;
  if ( !parent.isValid() ) {
    parentData = mRootData;
  } else {
    parentData = static_cast<ItemPrivateData*>( parent.internalPointer() );
  }

  ItemPrivateData *childData = parentData->child( row );
  if ( childData ) {
    return createIndex( row, column, childData );
  } else {
    return QModelIndex();
  }
//  FreeBusyItem::Ptr item = mFreeBusyItems.at( parent.row() );
//  KCalCore::FreeBusy::Ptr fb = item->freeBusy();
//  if( !fb )
//    return QModelIndex();
//
//  QList<KCalCore::FreeBusyPeriod> busyPeriods = fb->fullBusyPeriods();
//  if( row < busyPeriods.size() )
//    return createIndex( row, column, new ItemPrivateData( parent.row() ) );
//  else
//    return QModelIndex();
//  }
}

QModelIndex FreeBusyItemModel::parent( const QModelIndex &child ) const
{
  if ( !child.isValid() ) {
    return QModelIndex();
  }

  ItemPrivateData *childData = static_cast<ItemPrivateData*>( child.internalPointer() );
  ItemPrivateData *parentData = childData->parent();
  if ( parentData == mRootData ) {
    return QModelIndex();
  }

  return createIndex( parentData->row(), 0, parentData );
}

QVariant FreeBusyItemModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role == Qt::DisplayRole && orientation == Qt::Horizontal && section == 0 ) {
    return i18n( "Attendee" );
  }
  return QVariant();
}

void FreeBusyItemModel::addItem( const IncidenceEditorNG::FreeBusyItem::Ptr &freebusy )
{
  kDebug() << freebusy->attendee()->fullName();
  int row = mFreeBusyItems.size();
  beginInsertRows( QModelIndex(), row, row );
  mFreeBusyItems.append( freebusy );
  ItemPrivateData *data = new ItemPrivateData( mRootData );
  mRootData->appendChild( data );
  endInsertRows();

  if ( freebusy->freeBusy() && freebusy->freeBusy()->fullBusyPeriods().size() > 0 ) {
    QModelIndex parent = index( row, 0 );
    setFreeBusyPeriods( parent, freebusy->freeBusy()->fullBusyPeriods() );
  }
  updateFreeBusyData( freebusy );
}

void FreeBusyItemModel::setFreeBusyPeriods( const QModelIndex &parent,
                                            const KCalCore::FreeBusyPeriod::List &list )
{
  if(!parent.isValid())
    return;

  ItemPrivateData *parentData = static_cast<ItemPrivateData*>( parent.internalPointer() );
  QModelIndex first = index( 0, 0, parent );
  QModelIndex last = index( parentData->childCount() - 1, 0, parent );

  if ( parentData->childCount() > 0 ) {
    beginRemoveRows( parent, 0, parentData->childCount() - 1 );
    for ( int i = parentData->childCount() - 1; i >= 0; --i ) {
      delete parentData->removeChild( i );
    }
    endRemoveRows();
  }

  int fb_count = list.size();
  beginInsertRows( parent, 0, fb_count - 1 );
  for ( int i=0; i < fb_count; ++i ) {
    ItemPrivateData *childData= new ItemPrivateData( parentData );
    parentData->appendChild( childData );
  }
  endInsertRows();
  emit dataChanged( first, last );
}

void FreeBusyItemModel::clear()
{
  mFreeBusyItems.clear();
  delete mRootData;
  mRootData = new ItemPrivateData( 0 );
  reset();
}

void IncidenceEditorNG::FreeBusyItemModel::removeRow( int row )
{
  beginRemoveRows( QModelIndex(), row, row );
  mFreeBusyItems.removeAt( row );
  ItemPrivateData *data = mRootData->removeChild( row );
  delete data;
  endRemoveRows();
}

void FreeBusyItemModel::removeItem( const IncidenceEditorNG::FreeBusyItem::Ptr &freebusy )
{
  int row = mFreeBusyItems.indexOf( freebusy );
  if( row >= 0 ) {
    removeRow( row );
  }
}

void FreeBusyItemModel::removeAttendee( const KCalCore::Attendee::Ptr &attendee )
{
  FreeBusyItem::Ptr anItem;
  for ( int i = 0; i < mFreeBusyItems.count(); ++i ) {
    anItem = mFreeBusyItems[i];
    if ( *anItem->attendee() == *attendee ) {
      if ( anItem->updateTimerID() != 0 ) {
        killTimer( anItem->updateTimerID() );
      }
      removeRow( i );
      break;
    }
  }
}

bool FreeBusyItemModel::containsAttendee( const KCalCore::Attendee::Ptr &attendee )
{
  FreeBusyItem::Ptr anItem;
  for ( int i = 0; i < mFreeBusyItems.count(); ++i ) {
    anItem = mFreeBusyItems[i];
    if ( *anItem->attendee() == *attendee ) {
      return true;
    }
  }
  return false;
}

void FreeBusyItemModel::updateFreeBusyData( const FreeBusyItem::Ptr &item )
{
  if ( item->isDownloading() ) {
    // This item is already in the process of fetching the FB list
    return;
  }

  if ( item->updateTimerID() != 0 ) {
    // An update timer is already running. Reset it
    killTimer( item->updateTimerID() );
  }

  // This item does not have a download running, and no timer is set
  // Do the download in one second
  item->setUpdateTimerID( startTimer( 1000 ) );
}

void FreeBusyItemModel::timerEvent( QTimerEvent *event )
{
  killTimer( event->timerId() );
  Q_FOREACH ( FreeBusyItem::Ptr item, mFreeBusyItems ) {
    if ( item->updateTimerID() == event->timerId() ) {
      item->setUpdateTimerID( 0 );
      item->startDownload( mForceDownload );
      return;
    }
  }
}

void FreeBusyItemModel::slotInsertFreeBusy( const KCalCore::FreeBusy::Ptr &fb,
                                            const QString &email )
{
  if ( !fb ) {
    return;
  }

  if ( fb->fullBusyPeriods().isEmpty() ) {
    return;
  }

  fb->sortList();

  Q_FOREACH ( FreeBusyItem::Ptr item, mFreeBusyItems ) {
    if ( item->email() == email ) {
      item->setFreeBusy( fb );
      const int row = mFreeBusyItems.indexOf( item );
      const QModelIndex parent = index( row, 0 );
      setFreeBusyPeriods( parent, fb->fullBusyPeriods() );
    }
  }
}

void FreeBusyItemModel::autoReload()
{
  mForceDownload = false;
  reload();
}

void FreeBusyItemModel::reload()
{
  Q_FOREACH ( FreeBusyItem::Ptr item, mFreeBusyItems ) {
    if ( mForceDownload ) {
      item->startDownload( mForceDownload );
    } else {
      updateFreeBusyData( item );
    }
  }
}

void FreeBusyItemModel::triggerReload()
{
  mReloadTimer.start( 1000 );
}

void FreeBusyItemModel::cancelReload()
{
  mReloadTimer.stop();
}

void FreeBusyItemModel::manualReload()
{
  mForceDownload = true;
  reload();
}
