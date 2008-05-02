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

#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QString>

#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QBoxLayout>

#include <kapplication.h>
#include <KDialogButtonBox>
#include <kdebug.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <KListWidget>
#include <klocale.h>
#include <ktoolinvocation.h>
#include <libkdepim/categoryselectdialog.h>

#include "kabprefs.h"
#include "filtereditdialog.h"

FilterEditDialog::FilterEditDialog( QWidget *parent )
  : KDialog( parent)
{
  setCaption( i18n( "Edit Address Book Filter" ) );
  setButtons( Help | Ok | Cancel );
  setDefaultButton ( Ok );
  showButtonSeparator( true );
  setModal( false );
  initGUI();

  filterNameTextChanged( mNameEdit->text() );
}

FilterEditDialog::~FilterEditDialog()
{
}

void FilterEditDialog::setFilter( const Filter &filter )
{
  mNameEdit->setText( filter.name() );

  mCategoriesView->setSelected(filter.categories());

  if ( filter.matchRule() == Filter::Matching )
    mMatchRuleGroup->button( 0 )->setChecked( true );
  else
    mMatchRuleGroup->button( 1 )->setChecked( true );
}

Filter FilterEditDialog::filter()
{
  Filter filter;

  filter.setName( mNameEdit->text() );

  QString lst;
  filter.setCategories( mCategoriesView->selectedCategories(lst));

  if ( mMatchRuleGroup->button( 0 )->isChecked() )
    filter.setMatchRule( Filter::Matching );
  else
    filter.setMatchRule( Filter::NotMatching );

  return filter;
}

void FilterEditDialog::initGUI()
{
  resize( 490, 300 );

  QWidget *page = new QWidget( this );
  setMainWidget( page );
  QLabel *label;

  QGridLayout *topLayout = new QGridLayout( page );
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( 0 );

  label = new QLabel( i18n( "Name:" ), page );
  mNameEdit = new KLineEdit( page );
  mNameEdit->setFocus();
  topLayout->addWidget( label, 0, 0 );
  topLayout->addWidget( mNameEdit, 0, 1 );
  connect( mNameEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( filterNameTextChanged( const QString&) ) );
  mCategoriesView = new KPIM::CategorySelectWidget(page,KABPrefs::instance());
  mCategoriesView->setCategories(KABPrefs::instance()->customCategories());
  mCategoriesView->hideButton();
  mCategoriesView->layout()->setMargin( 0 );
  topLayout->addWidget( mCategoriesView, 1, 0, 1, 2 );

  mMatchRuleGroup = new QButtonGroup;
  mMatchRuleGroup->setExclusive( true );

  QGroupBox *group = new QGroupBox( page );

  QBoxLayout *gbLayout = new QVBoxLayout;
  gbLayout->setSpacing( KDialog::spacingHint() );
  gbLayout->setMargin( KDialog::marginHint() );
  group->setLayout( gbLayout );

  QRadioButton *radio = new QRadioButton( i18n( "Show only contacts matching the selected categories" ), group );
  mMatchRuleGroup->addButton( radio, 0 );
  radio->setChecked( true );
  gbLayout->addWidget( radio );

  radio = new QRadioButton( i18n( "Show all contacts except those matching the selected categories" ), group );
  mMatchRuleGroup->addButton( radio, 1 );
  gbLayout->addWidget( radio );

  topLayout->addWidget( group, 2, 0, 1, 2 );
  connect(this, SIGNAL(helpClicked()),this,SLOT(slotHelp()));
}

void FilterEditDialog::filterNameTextChanged( const QString &text )
{
  enableButtonOk( !text.isEmpty() );
}

void FilterEditDialog::slotHelp()
{
  KToolInvocation::invokeHelp( "using-filters" );
}

FilterDialog::FilterDialog( QWidget *parent )
  : KDialog( parent)
{
  setButtons( Ok | Cancel );
  setDefaultButton ( Ok );
  setCaption( i18n( "Edit Address Book Filters" ) );
  showButtonSeparator( true );
  setModal( false );
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

  mFilterListBox->setCurrentRow( mFilterListBox->count() - 1 );
}

void FilterDialog::edit()
{
  FilterEditDialog dlg( this );

  int pos = mFilterListBox->currentRow();

  dlg.setFilter( mFilterList[ pos ] );

  if ( dlg.exec() )
    mFilterList.replace( pos , dlg.filter() );

  refresh();

  mFilterListBox->setCurrentRow( pos );
}

void FilterDialog::remove()
{
  mFilterList.removeAt( mFilterListBox->currentRow()  );

  selectionChanged();

  refresh();
}

void FilterDialog::refresh()
{
  mFilterListBox->clear();

  Filter::List::ConstIterator it;
  for ( it = mFilterList.begin(); it != mFilterList.end(); ++it )
    mFilterListBox->addItem( new QListWidgetItem( (*it).name() ) );
}

void FilterDialog::selectionChanged()
{
  QListWidgetItem *item = mFilterListBox->currentItem();

  mEditButton->setEnabled( item );
  mRemoveButton->setEnabled( item );
}

void FilterDialog::initGUI()
{
  resize( 330, 200 );
  QWidget *page = new QWidget( this );
  setMainWidget( page );

  QGridLayout *topLayout = new QGridLayout( page );
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( 0 );

  mFilterListBox = new KListWidget( page );
  mFilterListBox->setSelectionMode( QAbstractItemView::SingleSelection );
  topLayout->addWidget( mFilterListBox, 0, 0 );
  connect( mFilterListBox, SIGNAL( currentItemChanged( QListWidgetItem *, QListWidgetItem * ) ),
           SLOT( selectionChanged() ) );
  connect( mFilterListBox, SIGNAL( itemDoubleClicked( QListWidgetItem * ) ),
           SLOT( edit() ) );

  KDialogButtonBox *buttonBox = new KDialogButtonBox( page, Qt::Vertical );
  buttonBox->addButton( i18n( "&Add..." ),QDialogButtonBox::ActionRole, this, SLOT( add() ) );
  mEditButton = buttonBox->addButton( i18n( "&Edit..." ),QDialogButtonBox::ActionRole, this, SLOT( edit() ) );
  mEditButton->setEnabled( false );
  mRemoveButton = buttonBox->addButton( i18n( "&Remove" ),QDialogButtonBox::ActionRole, this, SLOT( remove() ) );
  mRemoveButton->setEnabled( false );

  buttonBox->layout();
  topLayout->addWidget( buttonBox, 0, 1 );
}

#include "filtereditdialog.moc"
