/*
    This file is part of KAddressBook.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#include <QtCore/QTimer>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#include <QtGui/QToolTip>

#include <kabc/resource.h>
#include <kabc/resourceabc.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kresources/configdialog.h>

#include "core.h"
#include "resourceselection.h"

class AddressBookWrapper : public KABC::AddressBook
{
  public:
    AddressBookWrapper( KABC::AddressBook* );

    KRES::Manager<KABC::Resource>* getResourceManager()
    {
      return resourceManager();
    }
};

class KABCResourceItem : public QTreeWidgetItem
{
  public:
    KABCResourceItem( QTreeWidget *parent, KABC::Resource *resource )
      : QTreeWidgetItem( parent, QStringList( resource->resourceName() ) ),
        mResource( resource ),
        mIsSubresource( false ), mSubItemsCreated( false ),
        mResourceIdentifier()
    {
      setFlags( flags() | Qt::ItemIsUserCheckable );
      setCheckState( 0, resource->isActive() ? Qt::Checked : Qt::Unchecked );
      setIcon( 0, KIcon( "x-office-address-book" ) );
    }

    KABCResourceItem( KABC::ResourceABC *resourceABC, KABCResourceItem* parent,
                  const QString& resourceIdent )
      : QTreeWidgetItem( parent, QStringList( resourceABC->subresourceLabel( resourceIdent ) ) ),
        mResource( resourceABC ),
        mIsSubresource( true ), mSubItemsCreated( false ),
        mResourceIdentifier( resourceIdent )
    {
      setFlags( flags() | Qt::ItemIsUserCheckable );
      KABC::ResourceABC* res = static_cast<KABC::ResourceABC *>( mResource );
      setCheckState( 0, res->subresourceActive( mResourceIdentifier ) ? Qt::Checked : Qt::Unchecked );
      setIcon( 0, KIcon( "x-office-address-book" ) );

      treeWidget()->setRootIsDecorated( true );
    }

    ~KABCResourceItem()
    {
        qDebug( ) << "Deleting Item";
    }

    void createSubresourceItems();
    KABC::Resource *resource() const { return mResource; }
    QString resourceIdentifier() const { return mResourceIdentifier; }
    bool isSubResource() const { return mIsSubresource; }

  private:
    KABC::Resource * const mResource;
    const bool mIsSubresource;
    bool mSubItemsCreated;
    const QString mResourceIdentifier;
};

// Comes from korganizer/resourceview.cpp
void KABCResourceItem::createSubresourceItems()
{
  if ( !mIsSubresource && !mSubItemsCreated ) {
    KABC::ResourceABC* res = dynamic_cast<KABC::ResourceABC *>( mResource );
    QStringList subresources;
    if ( res )
      subresources = res->subresources();
    if ( !subresources.isEmpty() ) {
      setExpanded( true );

      // This resource has subresources
      QStringList::ConstIterator it;
      for ( it = subresources.begin(); it != subresources.end(); ++it ) {
        (void)new KABCResourceItem( res, this, *it );
      }
    }
    mSubItemsCreated = true;

    setExpanded( childCount() > 0 );
  }
}

// TODO: connect this to some signalResourceModified
// void KABCResourceItem::setGuiState()
// {
//   if ( mIsSubresource )
//     setOn( mResource->subresourceActive( mResourceIdentifier ) );
//   else
//     setOn( mResource->isActive() );
// }

////

ResourceSelection::ResourceSelection( KAB::Core *core, QWidget *parent )
  : KAB::ExtensionWidget( core, parent ), mManager( 0 )
{
  initGUI();

  AddressBookWrapper *wrapper = static_cast<AddressBookWrapper*>( core->addressBook() );
  mManager = wrapper->getResourceManager();

  connect( mAddButton, SIGNAL( clicked() ), SLOT( add() ) );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( edit() ) );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( remove() ) );

  connect( mListView, SIGNAL( itemClicked( QTreeWidgetItem *, int ) ),
           SLOT( currentChanged( QTreeWidgetItem * ) ) );

  QTimer::singleShot( 0, this, SLOT( updateView() ) );
}

ResourceSelection::~ResourceSelection()
{
}

QString ResourceSelection::title() const
{
  return i18n( "Address Books" );
}

QString ResourceSelection::identifier() const
{
  return "resourceselection";
}

void ResourceSelection::add()
{
  QStringList types = mManager->resourceTypeNames();
  QStringList descs = mManager->resourceTypeDescriptions();

  bool ok = false;
  QString desc = KInputDialog::getItem( i18n( "Add Address Book" ),
                                        i18n( "Please select type of the new address book:" ),
                                        descs, 0, false, &ok, this );
  if ( !ok )
    return;

  QString type = types[ descs.indexOf( desc ) ];

  // Create new resource
  KABC::Resource *resource = mManager->createResource( type );
  if ( !resource ) {
    KMessageBox::error( this, i18n("<qt>Unable to create an address book of type <b>%1</b>.</qt>",
                                type ) );
    return;
  }

  resource->setResourceName( i18n( "%1 address book", type ) );
  resource->setAddressBook( core()->addressBook() );

  KRES::ConfigDialog dlg( this, QString( "contact" ), resource );

  if ( dlg.exec() ) {
    core()->addressBook()->addResource( resource );
    resource->asyncLoad();

    mLastResource = resource->identifier();
    updateView();
  } else {
    delete resource;
    resource = 0;
  }
}

void ResourceSelection::edit()
{
  KABCResourceItem *item = selectedItem();
  if ( !item )
    return;

  KRES::ConfigDialog dlg( this, QString( "contact" ), item->resource() );

  if ( dlg.exec() ) {
    mManager->change( item->resource() );
    item->resource()->asyncLoad();

    mLastResource = item->resource()->identifier();
    updateView();
  }
}

void ResourceSelection::remove()
{
  KABCResourceItem *item = selectedItem();
  if ( !item )
    return;

  int result = KMessageBox::warningContinueCancel( this,
        i18n( "<qt>Do you really want to remove the address book <b>%1</b>?</qt>" ,
          item->resource()->resourceName() ), "",
        KGuiItem( i18n( "&Remove" ), "edit-delete" ) );
  if ( result == KMessageBox::Cancel )
    return;

  mLastResource = item->resource()->identifier();

  core()->addressBook()->removeResource( item->resource() );
  core()->addressBook()->emitAddressBookChanged();

  updateView();
}

void ResourceSelection::currentChanged( QTreeWidgetItem *item )
{
  KABCResourceItem *resItem = static_cast<KABCResourceItem*>( item );
  bool state = (resItem && !resItem->isSubResource() );

  mEditButton->setEnabled( state );
  mRemoveButton->setEnabled( state );

  if ( !resItem )
    return;

  KABC::Resource *resource = resItem->resource();

  resItem->createSubresourceItems();

  if ( resItem->isSubResource() ) {
    KABC::ResourceABC *res = static_cast<KABC::ResourceABC *>( resource );
    res->setSubresourceActive( resItem->resourceIdentifier(), resItem->checkState( 0 ) == Qt::Checked );
    mManager->change( resource );
  } else {
    resource->setActive( resItem->checkState( 0 ) == Qt::Checked );
    mManager->change( resource );

    if ( resItem->checkState( 0 ) == Qt::Checked ) {
      if ( !resource->addressBook() )
        resource->setAddressBook( core()->addressBook() );

      if ( !resource->isOpen() )
        resource->open();

      resource->asyncLoad();
    } else {
      resource->close();
    }
  }

  mLastResource = resource->identifier();
  core()->addressBook()->emitAddressBookChanged();
  //updateView();
}

void ResourceSelection::updateView()
{
  if ( !mManager )
    return;

  mListView->clear();

  KRES::Manager<KABC::Resource>::Iterator it;
  for ( it = mManager->begin(); it != mManager->end(); ++it ) {

    new KABCResourceItem( mListView, *it );
    KABC::ResourceABC* resource = dynamic_cast<KABC::ResourceABC *>( *it );
    if ( resource ) {
      disconnect( resource, 0, this, 0 );
      connect( resource, SIGNAL( signalSubresourceAdded( KABC::ResourceABC *,
                                                         const QString &, const QString & ) ),
               SLOT( slotSubresourceAdded( KABC::ResourceABC *,
                                           const QString &, const QString & ) ) );

      connect( resource, SIGNAL( signalSubresourceRemoved( KABC::ResourceABC *,
                                                           const QString &, const QString & ) ),
               SLOT( slotSubresourceRemoved( KABC::ResourceABC *,
                                             const QString &, const QString & ) ) );
      //connect( resource, SIGNAL( resourceSaved( KABC::ResourceABC * ) ),
      //         SLOT( closeResource( KABC::ResourceABC * ) ) );
    }
  }

  QTreeWidgetItemIterator iterator( mListView );
  while ( *iterator ) {
    KABCResourceItem *item = static_cast< KABCResourceItem * >( *iterator );
    if ( item->resource()->identifier() == mLastResource ) {
      item->setSelected( true );
      break;
    }
    ++iterator;
  }

  core()->addressBook()->emitAddressBookChanged();
}


// Add a new entry
void ResourceSelection::slotSubresourceAdded( KABC::ResourceABC *resource,
                                              const QString& /*type*/,
                                              const QString& subResource )
{
  kDebug(5720) << resource->resourceName() << subResource;

  QList< QTreeWidgetItem * > foundItems =
      mListView->findItems( resource->resourceName(), Qt::MatchExactly );

  if ( foundItems.size() == 0 )
    // Not found
    return;

  KABCResourceItem *item = static_cast<KABCResourceItem *>( foundItems[0] );
  (void)new KABCResourceItem( resource, item, subResource );
}

// Remove an entry
void ResourceSelection::slotSubresourceRemoved( KABC::ResourceABC* resource,
                                                const QString& /*type*/,
                                                const QString& subResource )
{
  kDebug(5720) << resource->resourceName() << subResource;
  // TODO
  //delete findItemByIdentifier( resource );
  //emitResourcesChanged();
}

KABCResourceItem* ResourceSelection::selectedItem() const
{
  return static_cast<KABCResourceItem*>( mListView->currentItem() );
}

void ResourceSelection::initGUI()
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  QBoxLayout *buttonLayout = new QHBoxLayout();
  buttonLayout->setSpacing( KDialog::spacingHint() );
  topLayout->addLayout( buttonLayout );

  QLabel *abLabel = new QLabel( i18n( "Address Books" ), this );
  buttonLayout->addWidget( abLabel );
  buttonLayout->addStretch( 1 );

  mAddButton = new QToolButton( this );
  mAddButton->setIcon( KIcon( "list-add" ) );
  mAddButton->setToolTip( i18n( "Add addressbook" ) );
  buttonLayout->addWidget( mAddButton );
  mEditButton = new QToolButton( this );
  mEditButton->setIcon( KIcon( "document-properties" ) );
  mEditButton->setEnabled( false );
  mEditButton->setToolTip( i18n( "Edit addressbook settings" ) );
  buttonLayout->addWidget( mEditButton );
  mRemoveButton = new QToolButton( this );
  mRemoveButton->setIcon( KIcon( "edit-delete" ) );
  mRemoveButton->setEnabled( false );
  mRemoveButton->setToolTip( i18n( "Remove addressbook" ) );
  buttonLayout->addWidget( mRemoveButton );

  mListView = new QTreeWidget( this );
  mListView->setRootIsDecorated( false );
  mListView->setHeaderLabel( i18n( "Address Books" ) );
  mListView->header()->hide();
  topLayout->addWidget( mListView );
}

class ResourceSelectionFactory : public KAB::ExtensionFactory
{
  public:
    KAB::ExtensionWidget *extension( KAB::Core *core, QWidget *parent )
    {
      return new ResourceSelection( core, parent );
    }

    QString identifier() const
    {
      return "resourceselection";
    }
};

K_EXPORT_PLUGIN(ResourceSelectionFactory)

#include "resourceselection.moc"
