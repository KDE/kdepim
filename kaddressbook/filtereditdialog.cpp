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

#include <q3buttongroup.h>
#include <q3hbox.h>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QRegExp>
#include <QString>
#include <QToolButton>
#include <QToolTip>
#include <QWidget>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QGridLayout>
#include <QBoxLayout>

#include <kapplication.h>
#include <kbuttonbox.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <k3listbox.h>
#include <k3listview.h>
#include <klocale.h>
#include <ktoolinvocation.h>

#include "kabprefs.h"
#include "filtereditdialog.h"

FilterEditDialog::FilterEditDialog( QWidget *parent, const char *name )
  : KDialog( parent)
{
  setCaption( i18n( "Edit Address Book Filter" ) );
  setButtons( Help | Ok | Cancel );
  setDefaultButton ( Ok );
  showButtonSeparator( true );
  setModal( false );
  initGUI();

  const QStringList cats = KABPrefs::instance()->customCategories();

  QStringList::ConstIterator it;
  for ( it = cats.begin(); it != cats.end(); ++it )
    mCategoriesView->insertItem( new Q3CheckListItem( mCategoriesView, *it, Q3CheckListItem::CheckBox ) );

  filterNameTextChanged( mNameEdit->text() );
}

FilterEditDialog::~FilterEditDialog()
{
}

void FilterEditDialog::setFilter( const Filter &filter )
{
  mNameEdit->setText( filter.name() );

  QStringList categories = filter.categories();
  Q3ListViewItem *item = mCategoriesView->firstChild();
  while ( item != 0 ) {
    if ( categories.contains( item->text( 0 ) ) ) {
      Q3CheckListItem *checkItem = static_cast<Q3CheckListItem*>( item );
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

  QStringList categories;
  Q3ListViewItem *item = mCategoriesView->firstChild();
  while ( item != 0 ) {
    Q3CheckListItem *checkItem = static_cast<Q3CheckListItem*>( item );
    if ( checkItem->isOn() )
      categories.append( item->text( 0 ) );

    item = item->nextSibling();
  }
  filter.setCategories( categories );

  if ( mMatchRuleGroup->find( 0 )->isChecked() )
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

  mCategoriesView = new K3ListView( page );
  mCategoriesView->addColumn( i18n( "Category" ) );
  mCategoriesView->setFullWidth( true );
  topLayout->addWidget( mCategoriesView, 1, 0, 1, 2 );

  mMatchRuleGroup = new Q3ButtonGroup( page );
  mMatchRuleGroup->setExclusive( true );

  QBoxLayout *gbLayout = new QVBoxLayout( mMatchRuleGroup );
  gbLayout->setSpacing( KDialog::spacingHint() );
  gbLayout->setMargin( KDialog::marginHint() );

  QRadioButton *radio = new QRadioButton( i18n( "Show only contacts matching the selected categories" ), mMatchRuleGroup );
  radio->setChecked( true );
  mMatchRuleGroup->insert( radio );
  gbLayout->addWidget( radio );

  radio = new QRadioButton( i18n( "Show all contacts except those matching the selected categories" ), mMatchRuleGroup );
  mMatchRuleGroup->insert( radio );
  gbLayout->addWidget( radio );

  topLayout->addWidget( mMatchRuleGroup, 2, 0, 1, 2 );
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

FilterDialog::FilterDialog( QWidget *parent, const char *name )
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

  mFilterListBox->setCurrentItem( mFilterListBox->count() - 1 );
}

void FilterDialog::edit()
{
  FilterEditDialog dlg( this );

  uint pos = mFilterListBox->currentItem();

  dlg.setFilter( mFilterList[ pos ] );

  if ( dlg.exec() ) {
    mFilterList.removeAt(  pos  );
#ifdef __GNUC__
#warning "kde4: correct ?"
#endif
    mFilterList.insert( pos , dlg.filter() );
  }

  refresh();

  mFilterListBox->setCurrentItem( pos );
}

void FilterDialog::remove()
{
#ifdef __GNUC__
#warning "kde4: correct ?"
#endif
  mFilterList.removeAt( mFilterListBox->currentItem()  );

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

void FilterDialog::selectionChanged( Q3ListBoxItem *item )
{
  bool state = ( item != 0 );

  mEditButton->setEnabled( state );
  mRemoveButton->setEnabled( state );
}

void FilterDialog::initGUI()
{
  resize( 330, 200 );
  QWidget *page = new QWidget( this );
  setMainWidget( page );

  QGridLayout *topLayout = new QGridLayout( page );
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( 0 );

  mFilterListBox = new K3ListBox( page );
  topLayout->addWidget( mFilterListBox, 0, 0 );
  connect( mFilterListBox, SIGNAL( selectionChanged( Q3ListBoxItem * ) ),
           SLOT( selectionChanged( Q3ListBoxItem * ) ) );
  connect( mFilterListBox, SIGNAL( doubleClicked ( Q3ListBoxItem * ) ),
           SLOT( edit() ) );

  KButtonBox *buttonBox = new KButtonBox( page, Qt::Vertical );
  buttonBox->addButton( i18n( "&Add..." ), this, SLOT( add() ) );
  mEditButton = buttonBox->addButton( i18n( "&Edit..." ), this, SLOT( edit() ) );
  mEditButton->setEnabled( false );
  mRemoveButton = buttonBox->addButton( i18n( "&Remove" ), this, SLOT( remove() ) );
  mRemoveButton->setEnabled( false );

  buttonBox->layout();
  topLayout->addWidget( buttonBox, 0, 1 );
}

#include "filtereditdialog.moc"
