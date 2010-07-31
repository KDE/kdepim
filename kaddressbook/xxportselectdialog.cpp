/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>
                       Tobias Koenig <tokoe@kde.org>

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

#include <kabc/addressbook.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <klocale.h>

#include <tqbuttongroup.h>
#include <tqcombobox.h>
#include <tqheader.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlistview.h>
#include <tqpushbutton.h>
#include <tqradiobutton.h>
#include <tqstringlist.h>
#include <tqwhatsthis.h>

#include "core.h"
#include "kabprefs.h"

#include "xxportselectdialog.h"

XXPortSelectDialog::XXPortSelectDialog( KAB::Core *core, bool sort,
                                        TQWidget* parent, const char* name )
    : KDialogBase( Plain, i18n( "Choose Which Contacts to Export" ), Help | Ok | Cancel,
                   Ok, parent, name, true, true ), mCore( core ),
      mUseSorting( sort )
{
  initGUI();

  connect( mFiltersCombo, TQT_SIGNAL( activated( int ) ),
           TQT_SLOT( filterChanged( int ) ) );
  connect( mCategoriesView, TQT_SIGNAL( clicked( TQListViewItem* ) ),
           TQT_SLOT( categoryClicked( TQListViewItem* ) ) );

  // setup filters
  mFilters = Filter::restore( kapp->config(), "Filter" );
  Filter::List::ConstIterator filterIt;
  TQStringList filters;
  for ( filterIt = mFilters.begin(); filterIt != mFilters.end(); ++filterIt )
    filters.append( (*filterIt).name() );

  mFiltersCombo->insertStringList( filters );
  mUseFilters->setEnabled( filters.count() > 0 );

  // setup categories
  const TQStringList categories =  KABPrefs::instance()->customCategories();
  TQStringList::ConstIterator it;
  for ( it = categories.begin(); it != categories.end(); ++it )
    new TQCheckListItem( mCategoriesView, *it, TQCheckListItem::CheckBox );
  mUseCategories->setEnabled( categories.count() > 0 );

  int count = mCore->selectedUIDs().count();
  mUseSelection->setEnabled( count != 0 );
  mUseSelection->setChecked( count  > 0 );

  mSortTypeCombo->insertItem( i18n( "Ascending" ) );
  mSortTypeCombo->insertItem( i18n( "Descending" ) );

  mFields = mCore->addressBook()->fields( KABC::Field::All );
  KABC::Field::List::ConstIterator fieldIt;
  for ( fieldIt = mFields.begin(); fieldIt != mFields.end(); ++fieldIt )
    mFieldCombo->insertItem( (*fieldIt)->label() );
}

KABC::AddresseeList XXPortSelectDialog::contacts()
{
  const TQStringList selection = mCore->selectedUIDs();

  KABC::AddresseeList list;
  if ( mUseSelection->isChecked() ) {
    TQStringList::ConstIterator it;
    for ( it = selection.begin(); it != selection.end(); ++it ) {
      KABC::Addressee addr = mCore->addressBook()->findByUid( *it );
      if ( !addr.isEmpty() )
        list.append( addr );
    }
  } else if ( mUseFilters->isChecked() ) {
    // find contacts that can pass selected filter
    Filter::List::ConstIterator filterIt;
    for ( filterIt = mFilters.begin(); filterIt != mFilters.end(); ++filterIt )
      if ( (*filterIt).name() == mFiltersCombo->currentText() )
        break;

    KABC::AddressBook::Iterator it;
    for ( it = mCore->addressBook()->begin(); it != mCore->addressBook()->end(); ++it ) {
      if ( (*filterIt).filterAddressee( *it ) )
        list.append( *it );
    }
  } else if ( mUseCategories->isChecked() ) {
    const TQStringList categorieList = categories();

    KABC::AddressBook::ConstIterator it;
    KABC::AddressBook::ConstIterator addressBookEnd( mCore->addressBook()->end() );
    for ( it = mCore->addressBook()->begin(); it != addressBookEnd; ++it ) {
      const TQStringList tmp( (*it).categories() );
      TQStringList::ConstIterator tmpIt;
      for ( tmpIt = tmp.begin(); tmpIt != tmp.end(); ++tmpIt )
        if ( categorieList.contains( *tmpIt ) ) {
          list.append( *it );
          break;
        }
    }
  } else {
    // create a string list of all entries:
    KABC::AddressBook::ConstIterator it;
    for ( it = mCore->addressBook()->begin(); it != mCore->addressBook()->end(); ++it )
      list.append( *it );
  }

  if ( mUseSorting ) {
    list.setReverseSorting( mSortTypeCombo->currentItem() == 1 );
    uint pos = mFieldCombo->currentItem();
    if ( pos < mFields.count() )
      list.sortByField( mFields[ pos ] );
  }

  return list;
}

TQStringList XXPortSelectDialog::categories() const
{
  TQStringList list;

  TQListViewItemIterator it( mCategoriesView );
  for ( ; it.current(); ++it ) {
    TQCheckListItem* qcli = static_cast<TQCheckListItem*>(it.current());
    if ( qcli->isOn() )
      list.append( it.current()->text( 0 ) );
  }

  return list;
}

void XXPortSelectDialog::filterChanged( int )
{
  mUseFilters->setChecked( true );
}

void XXPortSelectDialog::categoryClicked( TQListViewItem *i )
{
  if ( !i )
    return;

  TQCheckListItem *qcli = static_cast<TQCheckListItem*>( i );
  if ( qcli->isOn() )
    mUseCategories->setChecked( true );
}

void XXPortSelectDialog::slotHelp()
{
  kapp->invokeHelp( "import-and-export" );
}

void XXPortSelectDialog::initGUI()
{
  TQFrame *page = plainPage();

  TQVBoxLayout *topLayout = new TQVBoxLayout( page, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  TQLabel *label = new TQLabel( i18n( "Which contacts do you want to export?" ), page );
  topLayout->addWidget( label );

  mButtonGroup = new TQButtonGroup( i18n( "Selection" ), page );
  mButtonGroup->setColumnLayout( 0, Qt::Vertical );
  mButtonGroup->layout()->setSpacing( KDialog::spacingHint() );
  mButtonGroup->layout()->setMargin( KDialog::marginHint() );

  TQGridLayout *groupLayout = new TQGridLayout( mButtonGroup->layout() );
  groupLayout->setAlignment( Qt::AlignTop );

  mUseWholeBook = new TQRadioButton( i18n( "&All contacts" ), mButtonGroup );
  mUseWholeBook->setChecked( true );
  TQWhatsThis::add( mUseWholeBook, i18n( "Export the entire address book" ) );
  groupLayout->addWidget( mUseWholeBook, 0, 0 );
  mUseSelection = new TQRadioButton( i18n("&Selected contact", "&Selected contacts (%n selected)", mCore->selectedUIDs().count() ), mButtonGroup );
  TQWhatsThis::add( mUseSelection, i18n( "Only export contacts selected in KAddressBook.\n"
                                        "This option is disabled if no contacts are selected." ) );
  groupLayout->addWidget( mUseSelection, 1, 0 );

  mUseFilters = new TQRadioButton( i18n( "Contacts matching &filter" ), mButtonGroup );
  TQWhatsThis::add( mUseFilters, i18n( "Only export contacts matching the selected filter.\n"
                                     "This option is disabled if you have not defined any filters" ) );
  groupLayout->addWidget( mUseFilters, 2, 0 );

  mUseCategories = new TQRadioButton( i18n( "Category &members" ), mButtonGroup );
  TQWhatsThis::add( mUseCategories, i18n( "Only export contacts who are members of a category that is checked on the list to the left.\n"
                                       "This option is disabled if you have no categories." ) );
  groupLayout->addWidget( mUseCategories, 3, 0, Qt::AlignTop );

  mFiltersCombo = new TQComboBox( false, mButtonGroup );
  TQWhatsThis::add( mFiltersCombo, i18n( "Select a filter to decide which contacts to export." ) );
  groupLayout->addWidget( mFiltersCombo, 2, 1 );

  mCategoriesView = new TQListView( mButtonGroup );
  mCategoriesView->addColumn( "" );
  mCategoriesView->header()->hide();
  TQWhatsThis::add( mCategoriesView, i18n( "Check the categories whose members you want to export." ) );
  groupLayout->addWidget( mCategoriesView, 3, 1 );

  topLayout->addWidget( mButtonGroup );

  TQButtonGroup *sortingGroup = new TQButtonGroup( i18n( "Sorting" ), page );
  sortingGroup->setColumnLayout( 0, Qt::Vertical );
  TQGridLayout *sortLayout = new TQGridLayout( sortingGroup->layout(), 2, 2,
                                             KDialog::spacingHint() );
  sortLayout->setAlignment( Qt::AlignTop );

  label = new TQLabel( i18n( "Criterion:" ), sortingGroup );
  sortLayout->addWidget( label, 0, 0 );

  mFieldCombo = new KComboBox( false, sortingGroup );
  sortLayout->addWidget( mFieldCombo, 0, 1 );

  label = new TQLabel( i18n( "Order:" ), sortingGroup );
  sortLayout->addWidget( label, 1, 0 );

  mSortTypeCombo = new KComboBox( false, sortingGroup );
  sortLayout->addWidget( mSortTypeCombo, 1, 1 );

  topLayout->addWidget( sortingGroup );

  if ( !mUseSorting )
    sortingGroup->hide();
}

#include "xxportselectdialog.moc"
