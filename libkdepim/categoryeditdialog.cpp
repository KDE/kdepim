/*
    This file is part of libkdepim.

    Copyright (c) 2000, 2001, 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qstringlist.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qheader.h>
#include <qpushbutton.h>
#include <klocale.h>

#include "kpimprefs.h"

#include "categoryeditdialog.h"

using namespace KPIM;

class CategoryEditDialog::Private
{
  public:
    QListView *mView;
    QPushButton *mAddButton;
    QPushButton *mEditButton;
    QPushButton *mDeleteButton;
};

class CategoryListViewItem : public QListViewItem
{
  public:
    CategoryListViewItem( QListView *view, const QString &text ) :
      QListViewItem( view, text )
    {
    }

    void okRename ( int col ) // we need that public to explicitly accept renaming when closing the dialog
    {
      QListViewItem::okRename( col );
    }
};

CategoryEditDialog::CategoryEditDialog( KPimPrefs *prefs, QWidget* parent,
                                        const char* name, bool modal )
  : KDialogBase::KDialogBase( parent, name, modal,
    i18n("Edit Categories"), Ok|Apply|Cancel|Help, Ok, true ),
    mPrefs( prefs ), d( new Private )
{
  QWidget *widget = new QWidget( this );
  setMainWidget( widget );

  QGridLayout *layout = new QGridLayout( widget, 4, 2, marginHint(), spacingHint() );

  d->mView = new QListView( widget );
  d->mView->addColumn( "" );
  d->mView->header()->hide();
  d->mView->setDefaultRenameAction( QListView::Accept );

  layout->addMultiCellWidget( d->mView, 0, 3, 0, 0 );

  d->mAddButton = new QPushButton( i18n( "Add" ), widget );
  layout->addWidget( d->mAddButton, 0, 1 );

  d->mEditButton = new QPushButton( i18n( "Edit" ), widget );
  layout->addWidget( d->mEditButton, 1, 1 );

  d->mDeleteButton = new QPushButton( i18n( "Remove" ), widget );
  layout->addWidget( d->mDeleteButton, 2, 1 );


  fillList();

  connect( d->mAddButton, SIGNAL( clicked() ), this, SLOT( add() ) );
  connect( d->mEditButton, SIGNAL( clicked() ), this, SLOT( edit() ) );
  connect( d->mDeleteButton, SIGNAL( clicked() ), this, SLOT( remove() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
CategoryEditDialog::~CategoryEditDialog()
{
  delete d;
}

void CategoryEditDialog::fillList()
{
  d->mView->clear();
  QStringList::Iterator it;
  bool categoriesExist=false;
  for ( it = mPrefs->mCustomCategories.begin();
        it != mPrefs->mCustomCategories.end(); ++it ) {

    QListViewItem *item = new CategoryListViewItem( d->mView, *it );
    item->setRenameEnabled( 0, true );

    categoriesExist = true;
  }

  d->mEditButton->setEnabled( categoriesExist );
  d->mDeleteButton->setEnabled( categoriesExist );
  d->mView->setSelected( d->mView->firstChild(), true );
}

void CategoryEditDialog::add()
{
  if ( d->mView->firstChild() )
    d->mView->setCurrentItem( d->mView->firstChild() );

  QListViewItem *item = new CategoryListViewItem( d->mView, i18n( "New category" ) );
  item->setRenameEnabled( 0, true );

  d->mView->setSelected( item, true );
  d->mView->ensureItemVisible( item );
  item->startRename( 0 );

  bool itemCount = d->mView->childCount() > 0;
  d->mEditButton->setEnabled( itemCount );
  d->mDeleteButton->setEnabled( itemCount );
}

void CategoryEditDialog::edit()
{
  if ( d->mView->currentItem() )
    d->mView->currentItem()->startRename( 0 );
}

void CategoryEditDialog::remove()
{
  if ( d->mView->currentItem() ) {
    delete d->mView->currentItem();

    d->mView->setSelected( d->mView->currentItem(), true );

    bool itemCount = d->mView->childCount() > 0;
    d->mEditButton->setEnabled( itemCount );
    d->mDeleteButton->setEnabled( itemCount );
  }
}

void CategoryEditDialog::slotOk()
{
  // accept the currently ongoing rename
  if ( d->mView->selectedItem() )
    static_cast<CategoryListViewItem*>( d->mView->selectedItem() )->okRename( 0 );
  slotApply();
  accept();
}

void CategoryEditDialog::slotApply()
{
  mPrefs->mCustomCategories.clear();

  QListViewItem *item = d->mView->firstChild();
  while ( item ) {
    if ( !item->text( 0 ).isEmpty() )
      mPrefs->mCustomCategories.append( item->text( 0 ) );
    item = item->nextSibling();
  }
  mPrefs->writeConfig();

  emit categoryConfigChanged();
}

void CategoryEditDialog::slotCancel()
{
  reload();
  KDialogBase::slotCancel();
}

void CategoryEditDialog::reload()
{
  fillList();
}

#include "categoryeditdialog.moc"
