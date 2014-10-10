/*
  Copyright (c) 2000, 2001, 2002 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2012 Allen Winter <winter@kde.org>

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

#include "categoryeditdialog.h"

#include "ui_categoryeditdialog_base.h"

#include <calendarsupport/categoryhierarchyreader.h>
#include <calendarsupport/categoryconfig.h>

#include <KLocalizedString>
#include <QIcon>

#include <QHeaderView>
#include <QList>
#include <QStringList>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace IncidenceEditorNG;
using namespace CalendarSupport;

CategoryEditDialog::CategoryEditDialog( CategoryConfig *categoryConfig,
                                        QWidget *parent )
  : QDialog( parent ), mCategoryConfig( categoryConfig )
{
  setWindowTitle( i18n( "Edit Categories" ) );
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Help);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  mWidgets = new Ui::CategoryEditDialog_base();
  QWidget *widget = new QWidget( this );
  widget->setObjectName( "CategoryEdit" );
  mWidgets->setupUi( widget );

  mWidgets->mCategories->header()->hide();
  mWidgets->mButtonAdd->setIcon( QIcon::fromTheme( "list-add" ) );

  mWidgets->mButtonAddSubcategory->setEnabled( false );
  mWidgets->mButtonAddSubcategory->setIcon( QIcon::fromTheme( "list-add" ) );

  mWidgets->mButtonRemove->setEnabled( false );
  mWidgets->mButtonRemove->setIcon( QIcon::fromTheme( "list-remove" ) );

#ifndef KDEPIM_MOBILE_UI
  mWidgets->mCategories->setDragDropMode( QAbstractItemView::InternalMove );
#endif

  // unfortunately, kde-core-devel will not allow this code in QDialog
  // because these button's functionality cannot be easily generalized.
  okButton->setToolTip(i18n( "Apply changes and close"  ));
  okButton->setWhatsThis( i18n( "When clicking <b>Ok</b>, "
                                "the settings will be handed over to the "
                                "program and the dialog will be closed." ) );
  buttonBox->button(QDialogButtonBox::Cancel)->setToolTip(i18n( "Cancel changes and close"  ));
  buttonBox->button(QDialogButtonBox::Cancel)->setWhatsThis(i18n( "When clicking <b>Cancel</b>, "
                                    "the settings will be discarded and the "
                                    "dialog will be closed." ) );

  buttonBox->button(QDialogButtonBox::Help)->setWhatsThis(i18n( "When clicking <b>Help</b>, "
                                  "a separate KHelpCenter window will open "
                                  "providing more information about the settings." ) );

  mainLayout->addWidget(widget);
  mainLayout->addWidget(buttonBox);

  fillList();

  mWidgets->mCategories->setFocus();

  connect( mWidgets->mCategories, SIGNAL(itemSelectionChanged()),
           SLOT(editItem()) );
  connect( mWidgets->mCategories, SIGNAL(itemSelectionChanged()),
           SLOT(slotSelectionChanged()) );
  connect( mWidgets->mCategories, SIGNAL(itemCollapsed(QTreeWidgetItem*)),
           SLOT(expandIfToplevel(QTreeWidgetItem*)) );
  connect( mWidgets->mEdit, SIGNAL(textChanged(QString)),
           this, SLOT(slotTextChanged(QString)) );
  connect( mWidgets->mButtonAdd, SIGNAL(clicked()),
           this, SLOT(add()) );
  connect( mWidgets->mButtonAddSubcategory, SIGNAL(clicked()),
           this, SLOT(addSubcategory()) );
  connect( mWidgets->mButtonRemove, SIGNAL(clicked()),
           this, SLOT(remove()) );
  connect(okButton, &QPushButton::clicked, this, &CategoryEditDialog::slotOk);
  connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &CategoryEditDialog::slotCancel);
  //connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(slotApply()) );
}

CategoryEditDialog::~CategoryEditDialog()
{
  delete mWidgets;
}

void CategoryEditDialog::fillList()
{
  CalendarSupport::CategoryHierarchyReaderQTreeWidget(
    mWidgets->mCategories ).read( mCategoryConfig->customCategories() );
}

void CategoryEditDialog::slotTextChanged( const QString &text )
{
  QTreeWidgetItem *item = mWidgets->mCategories->currentItem();
  if ( item ) {
    item->setText( 0, text );
  }
}

void CategoryEditDialog::slotSelectionChanged()
{
  bool enable = ( mWidgets->mCategories->selectedItems().count() > 0 );
  mWidgets->mButtonAddSubcategory->setEnabled( enable );
  mWidgets->mButtonRemove->setEnabled( enable );
}

void CategoryEditDialog::add()
{
  QTreeWidgetItem *newItem =
    new QTreeWidgetItem( mWidgets->mCategories,
                         QStringList( i18n( "New category" ) ) );
  newItem->setExpanded( true );

  mWidgets->mCategories->setCurrentItem( newItem );
  mWidgets->mCategories->clearSelection();
  newItem->setSelected( true );
  mWidgets->mCategories->scrollToItem( newItem );
  mWidgets->mEdit->setFocus();
  mWidgets->mEdit->selectAll();
}

void CategoryEditDialog::addSubcategory()
{
  if ( !mWidgets->mEdit->text().isEmpty() ) {
    QTreeWidgetItem *newItem =
      new QTreeWidgetItem( mWidgets->mCategories->currentItem(),
                           QStringList( i18n( "New subcategory" ) ) );
    newItem->setExpanded( true );

    mWidgets->mCategories->setCurrentItem( newItem );
    mWidgets->mCategories->clearSelection();
    newItem->setSelected( true );
    mWidgets->mCategories->scrollToItem( newItem );
    mWidgets->mEdit->setFocus();
    mWidgets->mEdit->selectAll();
  }
}

void CategoryEditDialog::remove()
{
  QList<QTreeWidgetItem*> to_remove = mWidgets->mCategories->selectedItems();
  while ( !to_remove.isEmpty() ) {
    deleteItem( to_remove.takeFirst(), to_remove );
  }

  if ( mWidgets->mCategories->currentItem() ) {
    mWidgets->mCategories->currentItem()->setSelected( true );
  }
}

void CategoryEditDialog::deleteItem( QTreeWidgetItem *item, QList<QTreeWidgetItem *> &to_remove )
{
  if ( !item ) {
    return;
  }

  for ( int i = item->childCount() - 1; i >= 0; i-- ) {
    QTreeWidgetItem *child = item->child( i );
    to_remove.removeAll( child );
    deleteItem( child, to_remove );
  }
  delete item;
}

void CategoryEditDialog::slotOk()
{
  slotApply();
  accept();
}

void CategoryEditDialog::slotApply()
{
  QStringList l;

  QStringList path;
  QTreeWidgetItemIterator it( mWidgets->mCategories );
  while ( *it ) {
    path = mWidgets->mCategories->pathByItem( *it++ );
    path.replaceInStrings(
      CategoryConfig::categorySeparator,
      QString( "\\" ) + CategoryConfig::categorySeparator );
    l.append( path.join( CategoryConfig::categorySeparator ) );
  }
  mCategoryConfig->setCustomCategories( l );
  mCategoryConfig->writeConfig();

  emit categoryConfigChanged();
}

void CategoryEditDialog::slotCancel()
{
  reload();
}

void CategoryEditDialog::editItem()
{
  QList<QTreeWidgetItem*> to_edit = mWidgets->mCategories->selectedItems();
  if ( !to_edit.isEmpty() ) {
    QTreeWidgetItem *item = to_edit.first();
    if ( item ) {
      mWidgets->mEdit->setText( item->text( 0 ) );
    }
  }
}

void CategoryEditDialog::reload()
{
  fillList();
}

void CategoryEditDialog::show()
{
/*
  QTreeWidgetItem *first = 0;
  if ( mWidgets->mCategories->topLevelItemCount() ) {
    first = mWidgets->mCategories->topLevelItem( 0 );
    mWidgets->mCategories->setCurrentItem( first );
  }
  mWidgets->mCategories->clearSelection();
  if ( first ) {
    first->setSelected( true );
    editItem( first );
  }
*/
  QDialog::show();
}

void CategoryEditDialog::expandIfToplevel( QTreeWidgetItem *item )
{
  if ( !item->parent() ) {
    item->setExpanded( true );
  }
}

