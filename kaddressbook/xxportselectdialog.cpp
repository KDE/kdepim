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

#include <q3buttongroup.h>
#include <QComboBox>
#include <q3header.h>
#include <QLabel>
#include <QLayout>
#include <q3listview.h>
#include <QPushButton>
#include <QRadioButton>
#include <QStringList>

//Added by qt3to4:
#include <QVBoxLayout>
#include <QFrame>
#include <QGridLayout>
#include <ktoolinvocation.h>
#include <kglobal.h>

#include "core.h"
#include "kabprefs.h"

#include "xxportselectdialog.h"

XXPortSelectDialog::XXPortSelectDialog( KAB::Core *core, bool sort,
                                        QWidget* parent )
    : KDialog( parent ), mCore( core ),
      mUseSorting( sort )
{
  setCaption( i18n( "Choose Which Contacts to Export" ) );
  setButtons( Help | Ok | Cancel );
  setDefaultButton( Ok );
  setModal( true );
  showButtonSeparator( true );

  initGUI();

  connect( mFiltersCombo, SIGNAL( activated( int ) ),
           SLOT( filterChanged( int ) ) );
  connect( mCategoriesView, SIGNAL( clicked( Q3ListViewItem* ) ),
           SLOT( categoryClicked( Q3ListViewItem* ) ) );

  // setup filters
  mFilters = Filter::restore( KGlobal::config().data(), "Filter" );
  Filter::List::ConstIterator filterIt;
  QStringList filters;
  for ( filterIt = mFilters.begin(); filterIt != mFilters.end(); ++filterIt )
    filters.append( (*filterIt).name() );

  mFiltersCombo->addItems( filters );
  mUseFilters->setEnabled( filters.count() > 0 );

  // setup categories
  const QStringList categories =  KABPrefs::instance()->customCategories();
  QStringList::ConstIterator it;
  for ( it = categories.begin(); it != categories.end(); ++it )
    new Q3CheckListItem( mCategoriesView, *it, Q3CheckListItem::CheckBox );
  mUseCategories->setEnabled( categories.count() > 0 );

  int count = mCore->selectedUIDs().count();
  mUseSelection->setEnabled( count != 0 );
  mUseSelection->setChecked( count  > 0 );

  mSortTypeCombo->addItem( i18n( "Ascending" ) );
  mSortTypeCombo->addItem( i18n( "Descending" ) );

  mFields = mCore->addressBook()->fields( KABC::Field::All );
  KABC::Field::List::ConstIterator fieldIt;
  for ( fieldIt = mFields.begin(); fieldIt != mFields.end(); ++fieldIt )
    mFieldCombo->addItem( (*fieldIt)->label() );
  connect(this,SIGNAL(helpClicked()),this,SLOT(slotHelp()));
}

KABC::AddresseeList XXPortSelectDialog::contacts()
{
  const QStringList selection = mCore->selectedUIDs();

  KABC::AddresseeList list;
  if ( mUseSelection->isChecked() ) {
    QStringList::ConstIterator it;
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
    const QStringList categorieList = categories();

    KABC::AddressBook::ConstIterator it;
    KABC::AddressBook::ConstIterator addressBookEnd( mCore->addressBook()->end() );
    for ( it = mCore->addressBook()->begin(); it != addressBookEnd; ++it ) {
      const QStringList tmp( (*it).categories() );
      QStringList::ConstIterator tmpIt;
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
    list.setReverseSorting( mSortTypeCombo->currentIndex() == 1 );
    int pos = mFieldCombo->currentIndex();
    if ( pos < mFields.count() )
      list.sortByField( mFields[ pos ] );
  }

  return list;
}

QStringList XXPortSelectDialog::categories() const
{
  QStringList list;

  Q3ListViewItemIterator it( mCategoriesView );
  for ( ; it.current(); ++it ) {
    Q3CheckListItem* qcli = static_cast<Q3CheckListItem*>(it.current());
    if ( qcli->isOn() )
      list.append( it.current()->text( 0 ) );
  }

  return list;
}

void XXPortSelectDialog::filterChanged( int )
{
  mUseFilters->setChecked( true );
}

void XXPortSelectDialog::categoryClicked( Q3ListViewItem *i )
{
  if ( !i )
    return;

  Q3CheckListItem *qcli = static_cast<Q3CheckListItem*>( i );
  if ( qcli->isOn() )
    mUseCategories->setChecked( true );
}

void XXPortSelectDialog::slotHelp()
{
  KToolInvocation::invokeHelp( "import-and-export" );
}

void XXPortSelectDialog::initGUI()
{
  QFrame *page = new QFrame( this );
  setMainWidget( page );

  QVBoxLayout *topLayout = new QVBoxLayout( page );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( KDialog::marginHint() );

  QLabel *label = new QLabel( i18n( "Which contacts do you want to export?" ), page );
  topLayout->addWidget( label );

  mButtonGroup = new Q3ButtonGroup( i18n( "Selection" ), page );
  mButtonGroup->setColumnLayout( 0, Qt::Vertical );
  mButtonGroup->layout()->setSpacing( KDialog::spacingHint() );
  mButtonGroup->layout()->setMargin( KDialog::marginHint() );

  QGridLayout *groupLayout = new QGridLayout();
  mButtonGroup->layout()->addItem( groupLayout );
  groupLayout->setAlignment( Qt::AlignTop );

  mUseWholeBook = new QRadioButton( i18n( "&All contacts" ), mButtonGroup );
  mUseWholeBook->setChecked( true );
  mUseWholeBook->setWhatsThis( i18n( "Export the entire address book" ) );
  groupLayout->addWidget( mUseWholeBook, 0, 0 );
  mUseSelection = new QRadioButton( i18np( "&Selected contact", "&Selected contacts (%1 selected)", mCore->selectedUIDs().count() ), mButtonGroup );
  mUseSelection->setWhatsThis( i18n( "Only export contacts selected in KAddressBook.\n"
                                        "This option is disabled if no contacts are selected." ) );
  groupLayout->addWidget( mUseSelection, 1, 0 );

  mUseFilters = new QRadioButton( i18n( "Contacts matching &filter" ), mButtonGroup );
  mUseFilters->setWhatsThis( i18n( "Only export contacts matching the selected filter.\n"
                                     "This option is disabled if you have not defined any filters" ) );
  groupLayout->addWidget( mUseFilters, 2, 0 );

  mUseCategories = new QRadioButton( i18n( "Category &members" ), mButtonGroup );
  mUseCategories->setWhatsThis( i18n( "Only export contacts who are members of a category that is checked on the list to the left.\n"
                                       "This option is disabled if you have no categories." ) );
  groupLayout->addWidget( mUseCategories, 3, 0, Qt::AlignTop );

  mFiltersCombo = new QComboBox( mButtonGroup );
  mFiltersCombo->setEditable( false );
  mFiltersCombo->setWhatsThis( i18n( "Select a filter to decide which contacts to export." ) );
  groupLayout->addWidget( mFiltersCombo, 2, 1 );

  mCategoriesView = new Q3ListView( mButtonGroup );
  mCategoriesView->addColumn( "" );
  mCategoriesView->header()->hide();
  mCategoriesView->setWhatsThis( i18n( "Check the categories whose members you want to export." ) );
  groupLayout->addWidget( mCategoriesView, 3, 1 );

  topLayout->addWidget( mButtonGroup );

  Q3ButtonGroup *sortingGroup = new Q3ButtonGroup( i18n( "Sorting" ), page );
  sortingGroup->setColumnLayout( 0, Qt::Vertical );
  QGridLayout *sortLayout = new QGridLayout();
  sortingGroup->layout()->addItem( sortLayout );
  sortLayout->setSpacing( KDialog::spacingHint() );
  sortLayout->setAlignment( Qt::AlignTop );

  label = new QLabel( i18n( "Criterion:" ), sortingGroup );
  sortLayout->addWidget( label, 0, 0 );

  mFieldCombo = new KComboBox( false, sortingGroup );
  sortLayout->addWidget( mFieldCombo, 0, 1 );

  label = new QLabel( i18n( "Order:" ), sortingGroup );
  sortLayout->addWidget( label, 1, 0 );

  mSortTypeCombo = new KComboBox( false, sortingGroup );
  sortLayout->addWidget( mSortTypeCombo, 1, 1 );

  topLayout->addWidget( sortingGroup );

  if ( !mUseSorting )
    sortingGroup->hide();
}

#include "xxportselectdialog.moc"
