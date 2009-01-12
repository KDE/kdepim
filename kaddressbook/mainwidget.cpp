/*
    This file is part of KContactManager.

    Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "mainwidget.h"

#include <QDebug>

#include <QtGui/QAction>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QListView>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QSplitter>
#include <QtGui/QStackedWidget>

#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/collectionview.h>
#include <akonadi/itemview.h>

#include <kactioncollection.h>
#include <kabc/contactgroup.h>
#include <kabc/contactgroupbrowser.h>
#include <kabc/contactlineedit.h>
#include <kabc/kabcmodel.h>
#include <kabc/kabcitembrowser.h>
#include <kicon.h>
#include <klocale.h>
#include <kxmlguiclient.h>
#include <kaction.h>

#include "contacteditordialog.h"
#include "contactgroupeditordialog.h"

MainWidget::MainWidget( KXMLGUIClient *guiClient, QWidget *parent )
  : QWidget( parent ),
    mGuiClient( guiClient )
{
  mContactModel = new Akonadi::KABCModel( this );

  setupGui();
  setupActions();

  mCollectionModel = new Akonadi::CollectionModel( this );
  mCollectionModel->setHeaderData( 0, Qt::Horizontal, i18nc( "@title:column, contact groups", "Group" ) , Qt::EditRole );

  mCollectionFilterModel = new Akonadi::CollectionFilterProxyModel();
  mCollectionFilterModel->addMimeTypeFilter( "text/directory" );
  mCollectionFilterModel->addMimeTypeFilter( KABC::ContactGroup::mimeType() );
  mCollectionFilterModel->setSourceModel( mCollectionModel );

  // display collections sorted
  QSortFilterProxyModel *sortModel = new QSortFilterProxyModel( this );
  sortModel->setDynamicSortFilter( true );
  sortModel->setSortCaseSensitivity( Qt::CaseInsensitive );
  sortModel->setSourceModel( mCollectionFilterModel );

  mCollectionView->setModel( sortModel );
  mCollectionView->header()->setDefaultAlignment( Qt::AlignCenter );
  mCollectionView->header()->setSortIndicatorShown( false );

  mItemView->setModel( mContactModel );
  mItemView->header()->setDefaultAlignment( Qt::AlignCenter );
  for ( int column = 1; column < mContactModel->columnCount(); ++column )
    mItemView->setColumnHidden( column, true );

  connect( mCollectionView, SIGNAL( currentChanged( const Akonadi::Collection& ) ),
           mContactModel, SLOT( setCollection( const Akonadi::Collection& ) ) );
  connect( mItemView, SIGNAL( currentChanged( const Akonadi::Item& ) ),
           this, SLOT( itemSelected( const Akonadi::Item& ) ) );
  connect( mItemView, SIGNAL( doubleClicked( const Akonadi::Item& ) ),
           this, SLOT( editItem( const Akonadi::Item& ) ) );

  // show the contact details view as default
  mDetailsViewStack->setCurrentWidget( mContactDetails );
}

MainWidget::~MainWidget()
{
}

void MainWidget::setupGui()
{
  // the horizontal main layout
  QHBoxLayout *layout = new QHBoxLayout( this );

  // the splitter that contains the three main parts of the gui
  //   - collection view on the left
  //   - item view in the middle
  //   - details view on the right
  QSplitter *splitter = new QSplitter;
  layout->addWidget( splitter );

  // the collection view
  mCollectionView = new Akonadi::CollectionView();
  splitter->addWidget( mCollectionView );

  // the items view
  mItemView = new Akonadi::ItemView;
  splitter->addWidget( mItemView );

  // the details view stack
  mDetailsViewStack = new QStackedWidget();
  splitter->addWidget( mDetailsViewStack );

  // the details widget for contacts
  mContactDetails = new Akonadi::KABCItemBrowser( mDetailsViewStack );
  mDetailsViewStack->addWidget( mContactDetails );

  // the details widget for contact groups
  mContactGroupDetails = new Akonadi::ContactGroupBrowser( mDetailsViewStack );
  mDetailsViewStack->addWidget( mContactGroupDetails );
}

void MainWidget::setupActions()
{
  KAction *action = 0;
  KActionCollection *collection = mGuiClient->actionCollection();

  action = collection->addAction( "file_new_contact" );
  action->setIcon( KIcon( "contact-new" ) );
  action->setText( i18n( "&New Contact..." ) );
  connect( action, SIGNAL( triggered(bool) ), SLOT( newContact() ));
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_N ) );
  action->setWhatsThis( i18n( "Create a new contact<p>You will be presented with a dialog where you can add all data about a person, including addresses and phone numbers.</p>" ) );

  action = collection->addAction( "file_new_group" );
  action->setIcon( KIcon( "user-group-new" ) );
  action->setText( i18n( "&New Group..." ) );
  connect( action, SIGNAL( triggered(bool) ), SLOT( newGroup() ));
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_G ) );
  action->setWhatsThis( i18n( "Create a new group<p>You will be presented with a dialog where you can add a new group of contacts.</p>" ) );
}

void MainWidget::newContact()
{
  ContactEditorDialog dlg( ContactEditorDialog::CreateMode, mCollectionFilterModel, this );
  dlg.exec();
}

void MainWidget::newGroup()
{
  ContactGroupEditorDialog dlg( ContactGroupEditorDialog::CreateMode, mCollectionFilterModel, this );
  dlg.exec();
}

void MainWidget::editItem( const Akonadi::Item &reference )
{
  const QModelIndex index = mContactModel->indexForItem( reference, 0 );
  const Akonadi::Item item = mContactModel->itemForIndex( index );

  if ( item.mimeType() == "text/directory" || item.mimeType() == "text/vcard" ) {
    editContact( reference );
  } else if ( item.mimeType() == KABC::ContactGroup::mimeType() ) {
    editGroup( reference );
  }
}

/**
 * Depending on the mime type of the selected item, this method
 * brings up the right view on the detail view stack and sets the
 * selected item on it.
 */
void MainWidget::itemSelected( const Akonadi::Item &item )
{
  if ( item.mimeType() == QLatin1String( "text/directory" ) ) {
    mDetailsViewStack->setCurrentWidget( mContactDetails );
    mContactDetails->setItem( item );
  } else if ( item.mimeType() == KABC::ContactGroup::mimeType() ) {
    mDetailsViewStack->setCurrentWidget( mContactGroupDetails );
    mContactGroupDetails->setItem( item );
  }
}

void MainWidget::editContact( const Akonadi::Item &contact )
{
  ContactEditorDialog dlg( ContactEditorDialog::EditMode, mCollectionFilterModel, this );
  dlg.setContact( contact );
  dlg.exec();
}

void MainWidget::editGroup( const Akonadi::Item &group )
{
  ContactGroupEditorDialog dlg( ContactGroupEditorDialog::EditMode, mCollectionFilterModel, this );
  dlg.setContactGroup( group );
  dlg.exec();
}

#include "mainwidget.moc"
