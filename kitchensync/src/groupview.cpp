/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

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
*/

#include <qlayout.h>

#include "aboutpage.h"
#include "groupitem.h"
#include "syncprocessmanager.h"

#include "groupview.h"

GroupView::GroupView( QWidget *parent )
  : QWidget( parent ), mAboutPage( 0 )
{
  mLayout = new QVBoxLayout( this );

  mWidgetList = new KWidgetList( this );

  mLayout->addWidget( mWidgetList );
}

SyncProcess* GroupView::selectedSyncProcess() const
{
  GroupItem *item = static_cast<GroupItem*>( mWidgetList->selectedItem() );
  if ( item )
    return item->syncProcess();
  else
    return 0;
}

void GroupView::clear()
{
  mWidgetList->clear();
}

void GroupView::updateView()
{
  clear();

  if ( SyncProcessManager::self()->count() == 0 ) {
    mWidgetList->hide();

    if ( !mAboutPage ) {
      mAboutPage = new AboutPage( this );
      mLayout->addWidget( mAboutPage );

      connect( mAboutPage, SIGNAL( addGroup() ), SIGNAL( addGroup() ) );
    }

    mAboutPage->show();

  } else {
    if ( mAboutPage )
      mAboutPage->hide();
    mWidgetList->show();
  }

  for ( int i = 0; i < SyncProcessManager::self()->count(); ++i ) {
    SyncProcess *process = SyncProcessManager::self()->at( i );

    GroupItem *item = new GroupItem( mWidgetList, process );
    connect( item, SIGNAL( synchronizeGroup( SyncProcess* ) ),
             SIGNAL( synchronizeGroup( SyncProcess* ) ) );
    connect( item, SIGNAL( abortSynchronizeGroup( SyncProcess* ) ),
             SIGNAL( abortSynchronizeGroup( SyncProcess* ) ) );
    connect( item, SIGNAL( configureGroup( SyncProcess* ) ),
             SIGNAL( configureGroup( SyncProcess* ) ) );

    mWidgetList->appendItem( item );
  }
}

void GroupView::updateSyncProcess( SyncProcess *syncProcess )
{
  for ( int i = 0; i < (int)mWidgetList->count(); ++i ) {
    GroupItem *item = static_cast<GroupItem*>( mWidgetList->item( i ) );
    if ( item && item->syncProcess() == syncProcess )
      item->update();
  }
}

#include "groupview.moc"
