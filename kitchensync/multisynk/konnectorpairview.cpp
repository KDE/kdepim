/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qtimer.h>
#include <klocale.h>

#include "konnectorpairmanager.h"

#include "konnectorpairview.h"

KonnectorPairItem::KonnectorPairItem( KonnectorPair *pair, KListView *parent )
  : QObject( 0 ), QListViewItem( parent ), mPair( pair )
{
  connect( pair->manager(), SIGNAL( synceesRead( KSync::Konnector* ) ),
           this, SLOT( synceesRead( KSync::Konnector* ) ) );
  connect( pair->manager(), SIGNAL( synceeReadError( KSync::Konnector* ) ),
           this, SLOT( synceeReadError( KSync::Konnector* ) ) );
  connect( pair->manager(), SIGNAL( synceesWritten( KSync::Konnector* ) ),
           this, SLOT( synceesWritten( KSync::Konnector* ) ) );
  connect( pair->manager(), SIGNAL( synceeWriteError( KSync::Konnector* ) ),
           this, SLOT( synceeWriteError( KSync::Konnector* ) ) );
  connect( pair->manager(), SIGNAL( syncFinished() ),
           this, SLOT( syncFinished() ) );

  initialState();
}

QString KonnectorPairItem::text( int column ) const
{
  switch ( column ) {
    case 0:
      return i18n( "Yes" );
      break;
    case 1:
      return mPair->name();
      break;
    case 2:
      return mStatusMsg;
      break;
    default:
      return QString::null;
  }
}

QString KonnectorPairItem::uid() const
{
  return mPair->uid();
}

void KonnectorPairItem::initialState()
{
  mStatusMsg = i18n( "Press \"Sync\" to synchronize" );
  repaint();
}

void KonnectorPairItem::synceesRead( Konnector *konnector )
{
  mStatusMsg = i18n( "Retrieve data from %1..." ).arg( konnector->resourceName() );
  repaint();
}

void KonnectorPairItem::synceeReadError( Konnector *konnector )
{
  mStatusMsg = i18n( "Couldn't retrieve data from %1..." ).arg( konnector->resourceName() );
  repaint();
}

void KonnectorPairItem::synceesWritten( Konnector *konnector )
{
  mStatusMsg = i18n( "Write back data to %1..." ).arg( konnector->resourceName() );
  repaint();
}

void KonnectorPairItem::synceeWriteError( Konnector *konnector )
{
  mStatusMsg = i18n( "Couldn't write back data to %1..." ).arg( konnector->resourceName() );
  repaint();
}

void KonnectorPairItem::syncFinished()
{
  mStatusMsg = i18n( "Synchronization finished" );
  repaint();

  QTimer::singleShot( 2, this, SLOT( initialState() ) );
}



KonnectorPairView::KonnectorPairView( KonnectorPairManager* manager, QWidget *parent )
  : KListView( parent ), mManager( manager )
{
  addColumn( i18n( "Enabled" ) );
  addColumn( i18n( "Name" ) );
  addColumn( i18n( "State" ) );

  setAllColumnsShowFocus( true );
  setFullWidth( true );

  connect( manager, SIGNAL( changed() ),
           this, SLOT( refreshView() ) );
  connect( this, SIGNAL( selectionChanged() ),
           this, SLOT( slotSelectionChanged() ) );
  connect( this, SIGNAL( currentChanged( QListViewItem* ) ),
           this, SLOT( slotSelectionChanged() ) );
}

KonnectorPairView::~KonnectorPairView()
{
}

QString KonnectorPairView::selectedPair() const
{
  KonnectorPairItem *item = static_cast<KonnectorPairItem*>( selectedItem() );
  if ( item )
    return item->uid();
  else
    return QString::null;
}

void KonnectorPairView::refresh()
{
  refreshView();
}

void KonnectorPairView::refreshView()
{
  clear();

  KonnectorPair::List pairs = mManager->pairs();
  KonnectorPair::List::Iterator it;
  for ( it = pairs.begin(); it != pairs.end(); ++it )
    new KonnectorPairItem( *it, this );

  setSelected( firstChild(), true );
}

void KonnectorPairView::slotSelectionChanged()
{
  emit konnectorPairSelected( selectedItem() != 0 );
}

#include "konnectorpairview.moc"
