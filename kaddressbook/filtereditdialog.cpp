/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <tqbuttongroup.h>
#include <tqhbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqradiobutton.h>
#include <tqregexp.h>
#include <tqstring.h>
#include <tqtoolbutton.h>
#include <tqtooltip.h>
#include <tqwidget.h>

#include <kapplication.h>
#include <kbuttonbox.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klistbox.h>
#include <klistview.h>
#include <klocale.h>

#include "kabprefs.h"
#include "filtereditdialog.h"

FilterEditDialog::FilterEditDialog( TQWidget *parent, const char *name )
  : KDialogBase( Plain, i18n( "Edit Address Book Filter" ),
                 Help | Ok | Cancel, Ok, parent, name, false, true )
{
  initGUI();

  const TQStringList cats = KABPrefs::instance()->customCategories();

  TQStringList::ConstIterator it;
  for ( it = cats.begin(); it != cats.end(); ++it )
    mCategoriesView->insertItem( new TQCheckListItem( mCategoriesView, *it, TQCheckListItem::CheckBox ) );

  filterNameTextChanged( mNameEdit->text() );
}

FilterEditDialog::~FilterEditDialog()
{
}

void FilterEditDialog::setFilter( const Filter &filter )
{
  mNameEdit->setText( filter.name() );

  TQStringList categories = filter.categories();
  TQListViewItem *item = mCategoriesView->firstChild();
  while ( item != 0 ) {
    if ( categories.contains( item->text( 0 ) ) ) {
      TQCheckListItem *checkItem = static_cast<TQCheckListItem*>( item );
      checkItem->setOn( true );
    }

    item = item->nextSibling();
  }

  if ( filter.matchRule() == Filter::Matching )
    mMatchRuleGroup->setButton( 0 );
  else
    mMatchRuleGroup->setButton( 1 );
}

Filter FilterEditDialog::filter()
{
  Filter filter;

  filter.setName( mNameEdit->text() );

  TQStringList categories;
  TQListViewItem *item = mCategoriesView->firstChild();
  while ( item != 0 ) {
    TQCheckListItem *checkItem = static_cast<TQCheckListItem*>( item );
    if ( checkItem->isOn() )
      categories.append( item->text( 0 ) );

    item = item->nextSibling();
  }
  filter.setCategories( categories );

  if ( mMatchRuleGroup->find( 0 )->isOn() )
    filter.setMatchRule( Filter::Matching );
  else
    filter.setMatchRule( Filter::NotMatching );

  return filter;
}

void FilterEditDialog::initGUI()
{
  resize( 490, 300 );

  TQWidget *page = plainPage();
  TQLabel *label;

  TQGridLayout *topLayout = new TQGridLayout( page, 3, 2, 0, spacingHint() );

  label = new TQLabel( i18n( "Name:" ), page );
  mNameEdit = new KLineEdit( page );
  mNameEdit->setFocus();
  topLayout->addWidget( label, 0, 0 );
  topLayout->addWidget( mNameEdit, 0, 1 );
  connect( mNameEdit, TQT_SIGNAL( textChanged( const TQString& ) ),
           TQT_SLOT( filterNameTextChanged( const TQString&) ) );

  mCategoriesView = new KListView( page );
  mCategoriesView->addColumn( i18n( "Category" ) );
  mCategoriesView->setFullWidth( true );
  topLayout->addMultiCellWidget( mCategoriesView, 1, 1, 0, 1 );

  mMatchRuleGroup = new TQButtonGroup( page );
  mMatchRuleGroup->setExclusive( true );

  TQBoxLayout *gbLayout = new TQVBoxLayout( mMatchRuleGroup );
  gbLayout->setSpacing( KDialog::spacingHint() );
  gbLayout->setMargin( KDialog::marginHint() );

  TQRadioButton *radio = new TQRadioButton( i18n( "Show only contacts matching the selected categories" ), mMatchRuleGroup );
  radio->setChecked( true );
  mMatchRuleGroup->insert( radio );
  gbLayout->addWidget( radio );

  radio = new TQRadioButton( i18n( "Show all contacts except those matching the selected categories" ), mMatchRuleGroup );
  mMatchRuleGroup->insert( radio );
  gbLayout->addWidget( radio );

  topLayout->addMultiCellWidget( mMatchRuleGroup, 2, 2, 0, 1 );
}

void FilterEditDialog::filterNameTextChanged( const TQString &text )
{
  enableButtonOK( !text.isEmpty() );
}

void FilterEditDialog::slotHelp()
{
  kapp->invokeHelp( "using-filters" );
}

FilterDialog::FilterDialog( TQWidget *parent, const char *name )
  : KDialogBase( Plain, i18n( "Edit Address Book Filters" ),
                 Ok | Cancel, Ok, parent, name, false, true )
{
  initGUI();
}

FilterDialog::~FilterDialog()
{
}

void FilterDialog::setFilters( const Filter::List &list )
{
  mFilterList.clear();
  mInternalFilterList.clear();

  Filter::List::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    if ( (*it).isInternal() )
      mInternalFilterList.append( *it );
    else
      mFilterList.append( *it );
  }

  refresh();
}

Filter::List FilterDialog::filters() const
{
  Filter::List list = mFilterList + mInternalFilterList;
  return list;
}

void FilterDialog::add()
{
  FilterEditDialog dlg( this );

  if ( dlg.exec() )
    mFilterList.append( dlg.filter() );

  refresh();

  mFilterListBox->setCurrentItem( mFilterListBox->count() - 1 );
}

void FilterDialog::edit()
{
  FilterEditDialog dlg( this );

  uint pos = mFilterListBox->currentItem();

  dlg.setFilter( mFilterList[ pos ] );

  if ( dlg.exec() ) {
    mFilterList.remove( mFilterList.at( pos ) );
    mFilterList.insert( mFilterList.at( pos ), dlg.filter() );
  }

  refresh();

  mFilterListBox->setCurrentItem( pos );
}

void FilterDialog::remove()
{
  mFilterList.remove( mFilterList.at( mFilterListBox->currentItem() ) );

  selectionChanged( 0 );

  refresh();
}

void FilterDialog::refresh()
{
  mFilterListBox->clear();

  Filter::List::ConstIterator it;
  for ( it = mFilterList.begin(); it != mFilterList.end(); ++it )
    mFilterListBox->insertItem( (*it).name() );
}

void FilterDialog::selectionChanged( TQListBoxItem *item )
{
  bool state = ( item != 0 );

  mEditButton->setEnabled( state );
  mRemoveButton->setEnabled( state );
}

void FilterDialog::initGUI()
{
  resize( 330, 200 );

  TQWidget *page = plainPage();

  TQGridLayout *topLayout = new TQGridLayout( page, 1, 2, 0, spacingHint() );

  mFilterListBox = new KListBox( page );
  topLayout->addWidget( mFilterListBox, 0, 0 );
  connect( mFilterListBox, TQT_SIGNAL( selectionChanged( TQListBoxItem * ) ),
           TQT_SLOT( selectionChanged( TQListBoxItem * ) ) );
  connect( mFilterListBox, TQT_SIGNAL( doubleClicked ( TQListBoxItem * ) ),
           TQT_SLOT( edit() ) );

  KButtonBox *buttonBox = new KButtonBox( page, Vertical );
  buttonBox->addButton( i18n( "&Add..." ), this, TQT_SLOT( add() ) );
  mEditButton = buttonBox->addButton( i18n( "&Edit..." ), this, TQT_SLOT( edit() ) );
  mEditButton->setEnabled( false );
  mRemoveButton = buttonBox->addButton( i18n( "&Remove" ), this, TQT_SLOT( remove() ) );
  mRemoveButton->setEnabled( false );

  buttonBox->layout();
  topLayout->addWidget( buttonBox, 0, 1 );
}

#include "filtereditdialog.moc"
