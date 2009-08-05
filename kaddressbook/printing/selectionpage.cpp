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

#include "selectionpage.h"

#include <QtCore/QStringList>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QVBoxLayout>

#include <kcombobox.h>
#include <kdialog.h>
#include <klocale.h>

#include <libkdepim/categoryselectdialog.h>
#include <libkdepim/autochecktreewidget.h>


SelectionPage::SelectionPage( QWidget* parent, const char* name )
    : QWidget( parent )
{
  setObjectName( name );
  setWindowTitle( i18n( "Choose Which Contacts to Print" ) );

  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( KDialog::marginHint() );

  QLabel *label = new QLabel( i18n( "Which contacts do you want to print?" ), this );
  topLayout->addWidget( label );

  mButtonGroup = new QGroupBox( this );
  //mButtonGroup->setFrameShape( QFrame::NoFrame );
 // mButtonGroup->setColumnLayout( 0, Qt::Vertical );
  QGridLayout *groupLayout = new QGridLayout();
  mButtonGroup->setLayout( groupLayout );
  mButtonGroup->layout()->setSpacing( KDialog::spacingHint() );
  mButtonGroup->layout()->setMargin( KDialog::marginHint() );
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
mUseFilters->setEnabled(false); //sebsauer
  mUseFilters->setWhatsThis( i18n( "Only print contacts matching the selected filter.\n"
                                     "This option is disabled if you have not defined any filters." ) );
  groupLayout->addWidget( mUseFilters, 2, 0 );

  mUseCategories = new QRadioButton( i18n( "Category &members" ), mButtonGroup );
mUseCategories->setEnabled(false); //sebsauer
  mUseCategories->setWhatsThis( i18n( "Only print contacts who are members of a category that is checked on the list to the left.\n"
                                       "This option is disabled if you have no categories." ) );
  groupLayout->addWidget( mUseCategories, 3, 0, Qt::AlignTop );

  mFiltersCombo = new KComboBox( mButtonGroup );
  mFiltersCombo->setEditable( false );
  mFiltersCombo->setWhatsThis( i18n( "Select a filter to decide which contacts to print." ) );
  groupLayout->addWidget( mFiltersCombo, 2, 1 );
#if 0
  mCategoriesView = new KPIM::CategorySelectWidget( mButtonGroup, KABPrefs::instance() );
  mCategoriesView->hideButton();
  mCategoriesView->layout()->setMargin( 0 );
  mCategoriesView->setWhatsThis( i18n( "Check the categories whose members you want to print." ) );
  groupLayout->addWidget( mCategoriesView, 3, 1 );
#endif
  topLayout->addWidget( mButtonGroup );
#if 0
  connect( mFiltersCombo, SIGNAL( activated(int) ), SLOT( filterChanged() ) );
  connect( mCategoriesView->listView(), 
           SIGNAL( itemClicked( QTreeWidgetItem *, int ) ), 
           SLOT( categoryChanged() ) );
#endif
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
  mCategoriesView->setCategories( list );
  mUseCategories->setEnabled( list.count() > 0 );
}

QStringList SelectionPage::categories() const
{
  QString lst;
  return mCategoriesView->selectedCategories( lst );
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

void SelectionPage::filterChanged()
{
  mUseFilters->setChecked( true );
}

void SelectionPage::categoryChanged()
{
  mUseCategories->setChecked( true );
}

#include "selectionpage.moc"
