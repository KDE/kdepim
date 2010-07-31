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

#include <kdialog.h>
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

#include "selectionpage.h"

SelectionPage::SelectionPage( TQWidget* parent, const char* name )
    : TQWidget( parent, name )
{
  setCaption( i18n( "Choose Which Contacts to Print" ) );

  TQVBoxLayout *topLayout = new TQVBoxLayout( this, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  TQLabel *label = new TQLabel( i18n( "Which contacts do you want to print?" ), this );
  topLayout->addWidget( label );

  mButtonGroup = new TQButtonGroup( this );
  mButtonGroup->setFrameShape( TQButtonGroup::NoFrame );
  mButtonGroup->setColumnLayout( 0, Qt::Vertical );
  mButtonGroup->layout()->setSpacing( KDialog::spacingHint() );
  mButtonGroup->layout()->setMargin( KDialog::marginHint() );

  TQGridLayout *groupLayout = new TQGridLayout( mButtonGroup->layout() );
  groupLayout->setAlignment( Qt::AlignTop );

  mUseWholeBook = new TQRadioButton( i18n( "&All contacts" ), mButtonGroup );
  mUseWholeBook->setChecked( true );
  TQWhatsThis::add( mUseWholeBook, i18n( "Print the entire address book" ) );
  groupLayout->addWidget( mUseWholeBook, 0, 0 );

  mUseSelection = new TQRadioButton( i18n( "&Selected contacts" ), mButtonGroup );
  TQWhatsThis::add( mUseSelection, i18n( "Only print contacts selected in KAddressBook.\n"
                                        "This option is disabled if no contacts are selected." ) );
  groupLayout->addWidget( mUseSelection, 1, 0 );

  mUseFilters = new TQRadioButton( i18n( "Contacts matching &filter" ), mButtonGroup );
  TQWhatsThis::add( mUseFilters, i18n( "Only print contacts matching the selected filter.\n"
                                     "This option is disabled if you have not defined any filters." ) );
  groupLayout->addWidget( mUseFilters, 2, 0 );

  mUseCategories = new TQRadioButton( i18n( "Category &members" ), mButtonGroup );
  TQWhatsThis::add( mUseCategories, i18n( "Only print contacts who are members of a category that is checked on the list to the left.\n"
                                       "This option is disabled if you have no categories." ) );
  groupLayout->addWidget( mUseCategories, 3, 0, Qt::AlignTop );

  mFiltersCombo = new TQComboBox( false, mButtonGroup );
  TQWhatsThis::add( mFiltersCombo, i18n( "Select a filter to decide which contacts to print." ) );
  groupLayout->addWidget( mFiltersCombo, 2, 1 );

  mCategoriesView = new TQListView( mButtonGroup );
  mCategoriesView->addColumn( "" );
  mCategoriesView->header()->hide();
  TQWhatsThis::add( mCategoriesView, i18n( "Check the categories whose members you want to print." ) );
  groupLayout->addWidget( mCategoriesView, 3, 1 );

  topLayout->addWidget( mButtonGroup );

  connect( mFiltersCombo, TQT_SIGNAL( activated(int) ), TQT_SLOT( filterChanged(int) ) );
  connect( mCategoriesView, TQT_SIGNAL( clicked(TQListViewItem*) ), TQT_SLOT( categoryClicked(TQListViewItem*) ) );
}

SelectionPage::~SelectionPage()
{
}

void SelectionPage::setFilters( const TQStringList& filters )
{
  mFiltersCombo->clear();
  mFiltersCombo->insertStringList( filters );

  mUseFilters->setEnabled( filters.count() > 0 );
}

TQString SelectionPage::filter() const
{
  return mFiltersCombo->currentText();
}

bool SelectionPage::useFilters() const
{
  return mUseFilters->isChecked();
}

void SelectionPage::setCategories( const TQStringList& list )
{
  TQStringList::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    new TQCheckListItem( mCategoriesView, *it, TQCheckListItem::CheckBox );

  mUseCategories->setEnabled( list.count() > 0 );
}

TQStringList SelectionPage::categories() const
{
  TQStringList list;

  TQListViewItemIterator it( mCategoriesView );
  for ( ; it.current(); ++it ) {
    TQCheckListItem *qcli = static_cast<TQCheckListItem*>(it.current());
    if ( qcli->isOn() )
      list.append( it.current()->text( 0 ) );
  }

  return list;
}

bool SelectionPage::useCategories()
{
  return mUseCategories->isChecked();
}

void SelectionPage::setUseSelection( bool value )
{
  mUseSelection->setEnabled( value );
}

bool SelectionPage::useSelection() const
{
  return mUseSelection->isChecked();
}

void SelectionPage::filterChanged( int )
{
  mUseFilters->setChecked( true );
}

void SelectionPage::categoryClicked( TQListViewItem *i )
{
  TQCheckListItem *qcli = static_cast<TQCheckListItem*>( i );
  if ( i && qcli->isOn() )
    mUseCategories->setChecked( true );
}

#include "selectionpage.moc"
