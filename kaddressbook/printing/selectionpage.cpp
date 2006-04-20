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

#include <q3buttongroup.h>
#include <qcombobox.h>
#include <q3header.h>
#include <qlabel.h>
#include <qlayout.h>
#include <q3listview.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qstringlist.h>

//Added by qt3to4:
#include <QVBoxLayout>
#include <QGridLayout>

#include "selectionpage.h"

SelectionPage::SelectionPage( QWidget* parent, const char* name )
    : QWidget( parent, name )
{
  setCaption( i18n( "Choose Which Contacts to Print" ) );

  QVBoxLayout *topLayout = new QVBoxLayout( this, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "Which contacts do you want to print?" ), this );
  topLayout->addWidget( label );

  mButtonGroup = new QGroupBox( this );
  //mButtonGroup->setFrameShape( QFrame::NoFrame );
 // mButtonGroup->setColumnLayout( 0, Qt::Vertical );
  mButtonGroup->layout()->setSpacing( KDialog::spacingHint() );
  mButtonGroup->layout()->setMargin( KDialog::marginHint() );

  QGridLayout *groupLayout = new QGridLayout( mButtonGroup->layout() );
  groupLayout->setAlignment( Qt::AlignTop );

  mUseWholeBook = new QRadioButton( i18n( "&All contacts" ), mButtonGroup );
  mUseWholeBook->setChecked( true );
  mUseWholeBook->setWhatsThis( i18n( "Print the entire address book" ) );
  groupLayout->addWidget( mUseWholeBook, 0, 0 );

  mUseSelection = new QRadioButton( i18n( "&Selected contacts" ), mButtonGroup );
  mUseSelection->setWhatsThis( i18n( "Only print contacts selected in KAddressBook.\n"
                                        "This option is disabled if no contacts are selected." ) );
  groupLayout->addWidget( mUseSelection, 1, 0 );

  mUseFilters = new QRadioButton( i18n( "Contacts matching &filter" ), mButtonGroup );
  mUseFilters->setWhatsThis( i18n( "Only print contacts matching the selected filter.\n"
                                     "This option is disabled if you have not defined any filters." ) );
  groupLayout->addWidget( mUseFilters, 2, 0 );

  mUseCategories = new QRadioButton( i18n( "Category &members" ), mButtonGroup );
  mUseCategories->setWhatsThis( i18n( "Only print contacts who are members of a category that is checked on the list to the left.\n"
                                       "This option is disabled if you have no categories." ) );
  groupLayout->addWidget( mUseCategories, 3, 0, Qt::AlignTop );

  mFiltersCombo = new QComboBox( mButtonGroup );
  mFiltersCombo->setEditable( false );
  mFiltersCombo->setWhatsThis( i18n( "Select a filter to decide which contacts to print." ) );
  groupLayout->addWidget( mFiltersCombo, 2, 1 );

  mCategoriesView = new Q3ListView( mButtonGroup );
  mCategoriesView->addColumn( "" );
  mCategoriesView->header()->hide();
  mCategoriesView->setWhatsThis( i18n( "Check the categories whose members you want to print." ) );
  groupLayout->addWidget( mCategoriesView, 3, 1 );

  topLayout->addWidget( mButtonGroup );

  connect( mFiltersCombo, SIGNAL( activated(int) ), SLOT( filterChanged(int) ) );
  connect( mCategoriesView, SIGNAL( clicked(Q3ListViewItem*) ), SLOT( categoryClicked(Q3ListViewItem*) ) );
}

SelectionPage::~SelectionPage()
{
}

void SelectionPage::setFilters( const QStringList& filters )
{
  mFiltersCombo->clear();
  mFiltersCombo->addItems( filters );

  mUseFilters->setEnabled( filters.count() > 0 );
}

QString SelectionPage::filter() const
{
  return mFiltersCombo->currentText();
}

bool SelectionPage::useFilters() const
{
  return mUseFilters->isChecked();
}

void SelectionPage::setCategories( const QStringList& list )
{
  QStringList::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    new Q3CheckListItem( mCategoriesView, *it, Q3CheckListItem::CheckBox );

  mUseCategories->setEnabled( list.count() > 0 );
}

QStringList SelectionPage::categories() const
{
  QStringList list;

  Q3ListViewItemIterator it( mCategoriesView );
  for ( ; it.current(); ++it ) {
    Q3CheckListItem *qcli = static_cast<Q3CheckListItem*>(it.current());
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

void SelectionPage::categoryClicked( Q3ListViewItem *i )
{
  Q3CheckListItem *qcli = static_cast<Q3CheckListItem*>( i );
  if ( i && qcli->isOn() )
    mUseCategories->setChecked( true );
}

#include "selectionpage.moc"
