/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#include <qheader.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qwidget.h>

#include <kgenericfactory.h>
#include <klocale.h>
#include <libkdepim/kpimprefs.h>

#include "addressbooksyncee.h"

#include "addressbookfilter.h"

using namespace KSync;

K_EXPORT_KS_FILTER( libksfilter_addressbook, AddressBookFilter )

AddressBookFilter::AddressBookFilter( QObject *parent )
  : Filter( parent, "AddressBookFilter" )
{
  setName( i18n( "Address Book Filter" ) );
}

AddressBookFilter::~AddressBookFilter()
{
}

bool AddressBookFilter::supports( Syncee *syncee )
{
  return (dynamic_cast<AddressBookSyncee*>( syncee ) != 0);
}

QWidget *AddressBookFilter::configWidget( QWidget *parent )
{
  AddressBookConfigWidget *wdg = new AddressBookConfigWidget( parent, "AddressBookConfigWidget" );

  KPimPrefs prefs;
  prefs.usrReadConfig();
  wdg->setCategories( prefs.mCustomCategories );
  wdg->setSelectedCategories( mSelectedCategories );

  return wdg;
}

void AddressBookFilter::configWidgetClosed( QWidget *widget )
{
  AddressBookConfigWidget *wdg = static_cast<AddressBookConfigWidget*>( widget );
  mSelectedCategories = wdg->selectedCategories();
}

void AddressBookFilter::convert( Syncee *syncee )
{
  filterSyncee( dynamic_cast<AddressBookSyncee*>( syncee ), mSelectedCategories );
}

void AddressBookFilter::reconvert( Syncee *syncee )
{
  unfilterSyncee( dynamic_cast<AddressBookSyncee*>( syncee ) );
}

void AddressBookFilter::doLoad()
{
  mSelectedCategories = config()->readListEntry( "SelectedCategories" );
}

void AddressBookFilter::doSave()
{
  config()->writeEntry( "SelectedCategories", mSelectedCategories );
}

void AddressBookFilter::filterSyncee( AddressBookSyncee *syncee, const QStringList &categories )
{
  mFilteredEntries.clear();

  if ( categories.isEmpty() ) // do not filter
    return;

  QStringList::ConstIterator it;
  
  AddressBookSyncEntry *entry;
  for ( entry = syncee->firstEntry(); entry; entry = syncee->nextEntry() ) {
    bool found = false;
    for ( it = categories.begin(); it != categories.end(); ++it )
      if ( entry->addressee().categories().contains( *it ) ) {
        found = true;
        break;
      }

    if ( !found )
      mFilteredEntries.append( entry );
  }

  QPtrListIterator<AddressBookSyncEntry> entryIt( mFilteredEntries );
  while ( entryIt.current() ) {
    syncee->removeEntry( entryIt.current() );
    ++entryIt;
  }
}

void AddressBookFilter::unfilterSyncee( AddressBookSyncee *syncee )
{
  QPtrListIterator<AddressBookSyncEntry> entryIt( mFilteredEntries );
  while ( entryIt.current() ) {
    syncee->addEntry( entryIt.current() );
    ++entryIt;
  }
}



AddressBookConfigWidget::AddressBookConfigWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  mView = new QListView( this );
  mView->addColumn( "" );
  mView->header()->hide();

  layout->addWidget( mView );
}

void AddressBookConfigWidget::setCategories( const QStringList &categories )
{
  mView->clear();

  QStringList::ConstIterator it;
  for ( it = categories.begin(); it != categories.end(); ++it )
    new QCheckListItem( mView, *it, QCheckListItem::CheckBox );
}

void AddressBookConfigWidget::setSelectedCategories( const QStringList &categories )
{
  QListViewItemIterator itemIt( mView );
  QStringList::ConstIterator it;

  while ( itemIt.current() ) {
    bool found = false;
    for ( it = categories.begin(); it != categories.end(); ++it ) {
      if ( itemIt.current()->text( 0 ) == *it ) {
        found = true;
        break;
      }
    }

    QCheckListItem *item = static_cast<QCheckListItem*>( itemIt.current() );
    item->setOn( found );

    ++itemIt;
  }
}

QStringList AddressBookConfigWidget::selectedCategories() const
{
  QStringList categories;

  QListViewItemIterator itemIt( mView, QListViewItemIterator::Checked );
  while ( itemIt.current() )
    categories.append( itemIt.current()->text( 0 ) );

  return categories;
}
