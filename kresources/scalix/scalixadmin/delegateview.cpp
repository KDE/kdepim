/*
 *   This file is part of ScalixAdmin.
 *
 *   Copyright (C) 2007 Trolltech ASA. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <klocale.h>

#include "delegatemanager.h"

#include "delegateview.h"

class DelegateItem : public QListViewItem
{
  public:
    DelegateItem( QListView *parent, const Scalix::Delegate &delegate )
      : QListViewItem( parent ), mDelegate( delegate )
    {
      setText( 0, mDelegate.email() );
      setText( 1, Scalix::Delegate::rightsAsString( mDelegate.rights() ) );
    }

    Scalix::Delegate delegate() const { return mDelegate; }

  private:
    Scalix::Delegate mDelegate;
};

DelegateView::DelegateView( DelegateManager *manager, QWidget *parent )
  : KListView( parent ), mManager( manager )
{
  addColumn( i18n( "Delegate" ) );
  addColumn( i18n( "Rights" ) );
  setFullWidth( true );
  setAllColumnsShowFocus( true );

  connect( mManager, SIGNAL( changed() ), SLOT( delegateChanged() ) );

  delegateChanged();
}

Scalix::Delegate DelegateView::selectedDelegate() const
{
  DelegateItem *item = dynamic_cast<DelegateItem*>( selectedItem() );
  if ( item )
    return item->delegate();

  return Scalix::Delegate();
}

void DelegateView::delegateChanged()
{
  clear();

  const Scalix::Delegate::List delegates = mManager->delegates();
  for ( uint i = 0; i < delegates.count(); ++i )
    new DelegateItem( this, delegates[ i ] );
}

#include "delegateview.moc"
