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

#include <QLayout>
#include <QPushButton>
#include <QTimer>
//Added by qt3to4:
#include <QGridLayout>

#include <kabc/resource.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kresources/configdialog.h>

#include "core.h"

#include "resourceselection.h"
#include <libkdepim/resourceabc.h>

class AddressBookWrapper : public KABC::AddressBook
{
  public:
    AddressBookWrapper( KABC::AddressBook* );

    KRES::Manager<KABC::Resource>* getResourceManager()
    {
      return resourceManager();
    }
};

class ResourceItem : public Q3CheckListItem
{
  public:
    ResourceItem( K3ListView *parent, KABC::Resource *resource )
      : Q3CheckListItem( parent, resource->resourceName(), CheckBox ),
        mResource( resource ), mChecked( false ),
        mIsSubresource( false ), mSubItemsCreated( false ),
        mResourceIdentifier()
    {
      setOn( resource->isActive() );
      setPixmap( 0, KIconLoader::global()->loadIcon( "help-contents", K3Icon::Small ) );
      mChecked = isOn();
    }

    ResourceItem( KPIM::ResourceABC *resourceABC, ResourceItem* parent,
                  const QString& resourceIdent )
      : Q3CheckListItem( parent, resourceABC->subresourceLabel( resourceIdent ), CheckBox ),
        mResource( resourceABC ), mChecked( false ),
        mIsSubresource( true ), mSubItemsCreated( false ),
        mResourceIdentifier( resourceIdent )
    {
      KPIM::ResourceABC* res = static_cast<KPIM::ResourceABC *>( mResource );
      setOn( res->subresourceActive( mResourceIdentifier ) );
      setPixmap( 0, KIconLoader::global()->loadIcon( "help-contents", K3Icon::Small ) );
      mChecked = isOn();
    }

    void createSubresourceItems();

    void setChecked( bool state ) { mChecked = state; }
    bool checked() const { return mChecked; }
    KABC::Resource *resource() const { return mResource; }
    QString resourceIdentifier() const { return mResourceIdentifier; }
    bool isSubResource() const { return mIsSubresource; }

    virtual void stateChange( bool active );

  private:
    KABC::Resource * const mResource;
    bool mChecked;
    const bool mIsSubresource;
    bool mSubItemsCreated;
    const QString mResourceIdentifier;
};

// Comes from korganizer/resourceview.cpp
void ResourceItem::createSubresourceItems()
{
  KPIM::ResourceABC* res = dynamic_cast<KPIM::ResourceABC *>( mResource );
  QStringList subresources;
  if ( res )
    subresources = res->subresources();
  if ( !subresources.isEmpty() ) {
    setOpen( true );
    setExpandable( true );
    // This resource has subresources
    QStringList::ConstIterator it;
    for ( it = subresources.begin(); it != subresources.end(); ++it ) {
      (void)new ResourceItem( res, this, *it );
    }
  }
  mSubItemsCreated = true;
}

// TODO: connect this to some signalResourceModified
// void ResourceItem::setGuiState()
// {
//   if ( mIsSubresource )
//     setOn( mResource->subresourceActive( mResourceIdentifier ) );
//   else
//     setOn( mResource->isActive() );
// }

void ResourceItem::stateChange( bool active )
{
  //kDebug(5720) << this << text( 0 ) <<" active=" << active;
  if ( active && !mIsSubresource ) {
    if ( !mSubItemsCreated )
      createSubresourceItems();
  }

  setOpen( active && childCount() > 0 );
}

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

  connect( mListView, SIGNAL( clicked( Q3ListViewItem* ) ),
           SLOT( currentChanged( Q3ListViewItem* ) ) );

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
  ResourceItem *item = selectedItem();
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
  ResourceItem *item = selectedItem();
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

void ResourceSelection::currentChanged( Q3ListViewItem *item )
{
  ResourceItem *resItem = static_cast<ResourceItem*>( item );
  bool state = (resItem && !resItem->isSubResource() );

  mEditButton->setEnabled( state );
  mRemoveButton->setEnabled( state );

  if ( !resItem )
    return;

  KABC::Resource *resource = resItem->resource();

  if ( resItem->checked() != resItem->isOn() ) {
    resItem->setChecked( resItem->isOn() );
    if ( resItem->isSubResource() ) {
      KPIM::ResourceABC *res = static_cast<KPIM::ResourceABC *>( resource );
      res->setSubresourceActive( resItem->resourceIdentifier(), resItem->isOn() );
      mManager->change( resource );
    } else {
      resource->setActive( resItem->isOn() );
      mManager->change( resource );

      if ( resItem->checked() ) {
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
}

void ResourceSelection::updateView()
{
  if ( !mManager )
    return;

  mListView->clear();

  KRES::Manager<KABC::Resource>::Iterator it;
  for ( it = mManager->begin(); it != mManager->end(); ++it ) {

    new ResourceItem( mListView, *it );
    KPIM::ResourceABC* resource = dynamic_cast<KPIM::ResourceABC *>( *it );
    if ( resource ) {
      disconnect( resource, 0, this, 0 );
      connect( resource, SIGNAL( signalSubresourceAdded( KPIM::ResourceABC *,
                                                         const QString &, const QString & ) ),
               SLOT( slotSubresourceAdded( KPIM::ResourceABC *,
                                           const QString &, const QString & ) ) );

      connect( resource, SIGNAL( signalSubresourceRemoved( KPIM::ResourceABC *,
                                                           const QString &, const QString & ) ),
               SLOT( slotSubresourceRemoved( KPIM::ResourceABC *,
                                             const QString &, const QString & ) ) );
      //connect( resource, SIGNAL( resourceSaved( KPIM::ResourceABC * ) ),
      //         SLOT( closeResource( KPIM::ResourceABC * ) ) );
    }
  }

  Q3ListViewItemIterator itemIt( mListView );
  while ( itemIt.current() ) {
    ResourceItem *item = static_cast<ResourceItem*>( itemIt.current() );
    if ( item->resource()->identifier() == mLastResource ) {
      mListView->setSelected( item, true );
      mListView->ensureItemVisible( item );
      break;
    }
    ++itemIt;
  }

  core()->addressBook()->emitAddressBookChanged();
}


// Add a new entry
void ResourceSelection::slotSubresourceAdded( KPIM::ResourceABC *resource,
                                              const QString& /*type*/,
                                              const QString& subResource )
{
  kDebug(5720) << resource->resourceName() << subResource;
  Q3ListViewItem *i = mListView->findItem( resource->resourceName(), 0 );
  if ( !i )
    // Not found
    return;

  ResourceItem *item = static_cast<ResourceItem *>( i );
  (void)new ResourceItem( resource, item, subResource );
}

// Remove an entry
void ResourceSelection::slotSubresourceRemoved( KPIM::ResourceABC* resource,
                                                const QString& /*type*/,
                                                const QString& subResource )
{
  kDebug(5720) << resource->resourceName() << subResource;
  // TODO
  //delete findItemByIdentifier( resource );
  //emitResourcesChanged();
}

ResourceItem* ResourceSelection::selectedItem() const
{
  return static_cast<ResourceItem*>( mListView->selectedItem() );
}

void ResourceSelection::initGUI()
{
  QGridLayout *layout = new QGridLayout( this );
  layout->setSpacing( 5 );
  layout->setMargin( 2 );

  mListView = new K3ListView( this );
  mListView->addColumn( i18n( "Address Books" ) );
  mListView->setFullWidth( true );
  layout->addWidget( mListView, 0, 0, 1, 3 );

  mAddButton = new QPushButton( i18n( "Add..." ), this );
  mEditButton = new QPushButton( i18n( "Edit..." ), this );
  mEditButton->setEnabled( false );
  mRemoveButton = new QPushButton( i18n( "Remove" ), this );
  mRemoveButton->setEnabled( false );

  layout->addWidget( mAddButton, 1, 0 );
  layout->addWidget( mEditButton, 1, 1 );
  layout->addWidget( mRemoveButton, 1, 2 );
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

extern "C" {
  KDE_EXPORT void *init_libkaddrbk_resourceselection()
  {
    return ( new ResourceSelectionFactory );
  }
}

#include "resourceselection.moc"
