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

#include <tqlayout.h>
#include <tqpopupmenu.h>
#include <tqpushbutton.h>
#include <tqtimer.h>
#include <tqlabel.h>
#include <tqheader.h>
#include <tqtooltip.h>

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

class ResourceItem : public QCheckListItem
{
  public:
    ResourceItem( KListView *parent, KABC::Resource *resource )
      : TQCheckListItem( parent, resource->resourceName(), CheckBox ),
        mResource( resource ), mChecked( false ),
        mIsSubresource( false ), mSubItemsCreated( false ),
        mResourceIdentifier()
    {
      setOn( resource->isActive() );
      setPixmap( 0, KGlobal::iconLoader()->loadIcon( "contents", KIcon::Small ) );
      mChecked = isOn();
    }

    ResourceItem( KPIM::ResourceABC *resourceABC, ResourceItem* parent,
                  const TQString& resourceIdent )
      : TQCheckListItem( parent, resourceABC->subresourceLabel( resourceIdent ), CheckBox ),
        mResource( resourceABC ), mChecked( false ),
        mIsSubresource( true ), mSubItemsCreated( false ),
        mResourceIdentifier( resourceIdent )
    {
      KPIM::ResourceABC* res = dynamic_cast<KPIM::ResourceABC *>( mResource );
      setOn( res->subresourceActive( mResourceIdentifier ) );
      setPixmap( 0, KGlobal::iconLoader()->loadIcon( "contents", KIcon::Small ) );
      mChecked = isOn();
    }

    void createSubresourceItems();

    void setChecked( bool state ) {
        mChecked = state;
        setOn(state);
    }
    bool checked() const { return mChecked; }
    KABC::Resource *resource() const { return mResource; }
    TQString resourceIdentifier() const { return mResourceIdentifier; }
    bool isSubResource() const { return mIsSubresource; }

    virtual void stateChange( bool active );

  private:
    KABC::Resource * const mResource;
    bool mChecked;
    const bool mIsSubresource;
    bool mSubItemsCreated;
    const TQString mResourceIdentifier;
};

// Comes from korganizer/resourceview.cpp
void ResourceItem::createSubresourceItems()
{
  KPIM::ResourceABC* res = dynamic_cast<KPIM::ResourceABC *>( mResource );
  TQStringList subresources;
  if ( res )
    subresources = res->subresources();
  if ( !subresources.isEmpty() ) {
    setOpen( true );
    setExpandable( true );
    // This resource has subresources
    TQStringList::ConstIterator it;
    for ( it = subresources.begin(); it != subresources.end(); ++it ) {
      (void)new ResourceItem( res, this, *it );
    }
  }
  mSubItemsCreated = true;
}

void ResourceItem::stateChange( bool active )
{
  //kdDebug(5720) << k_funcinfo << this << " " << text( 0 ) << " active=" << active << endl;
  if ( active && !mIsSubresource ) {
    if ( !mSubItemsCreated )
      createSubresourceItems();
  }

  setOpen( active && childCount() > 0 );
}

////

ResourceSelection::ResourceSelection( KAB::Core *core, TQWidget *parent, const char *name )
  : KAB::ExtensionWidget( core, parent, name ), mManager( 0 )
{
  initGUI();

  AddressBookWrapper *wrapper = static_cast<AddressBookWrapper*>( core->addressBook() );
  mManager = wrapper->getResourceManager();

  connect( mAddButton, TQT_SIGNAL( clicked() ), TQT_SLOT( add() ) );
  connect( mEditButton, TQT_SIGNAL( clicked() ), TQT_SLOT( edit() ) );
  connect( mRemoveButton, TQT_SIGNAL( clicked() ), TQT_SLOT( remove() ) );

  connect( mListView, TQT_SIGNAL( clicked( TQListViewItem* ) ),
           TQT_SLOT( currentChanged( TQListViewItem* ) ) );

  connect( mListView, TQT_SIGNAL( contextMenuRequested ( TQListViewItem *,
                                                     const TQPoint &, int ) ),
           TQT_SLOT( contextMenuRequested( TQListViewItem *, const TQPoint &,
                                       int ) ) );

  TQTimer::singleShot( 0, this, TQT_SLOT( updateView() ) );
}

ResourceSelection::~ResourceSelection()
{
}

void ResourceSelection::contextMenuRequested ( TQListViewItem *i,
                                          const TQPoint &pos, int )
{
  ResourceItem *item = static_cast<ResourceItem *>( i );

  TQPopupMenu *menu = new TQPopupMenu( this );
  connect( menu, TQT_SIGNAL( aboutToHide() ), menu, TQT_SLOT( deleteLater() ) );
  if ( item ) {
    int reloadId = menu->insertItem( i18n("Re&load"), this,
                                     TQT_SLOT( reloadResource() ) );
    menu->setItemEnabled( reloadId, item->resource()->isActive() );
    int saveId = menu->insertItem( i18n("&Save"), this,
                                   TQT_SLOT( saveResource() ) );
    menu->setItemEnabled( saveId, item->resource()->isActive() );
    menu->insertSeparator();

//     menu->insertItem( i18n("Show &Info"), this, TQT_SLOT( showInfo() ) );

    menu->insertItem( i18n("&Edit..."), this, TQT_SLOT( edit() ) );
    menu->insertItem( i18n("&Remove"), this, TQT_SLOT( remove() ) );

    menu->insertSeparator();
 }
  menu->insertItem( i18n("&Add..."), this, TQT_SLOT( add() ) );

  menu->popup( pos );
}

void ResourceSelection::reloadResource()
{
  ResourceItem *item = selectedItem();
  if ( !item ) return;

  KABC::Resource *r = item->resource();
  r->load();
}

void ResourceSelection::saveResource()
{
  ResourceItem *item = selectedItem();
  if ( !item ) return;

  KABC::Resource *r = item->resource();
  KABC::Ticket *ticket = core()->addressBook()->requestSaveTicket( r );
  if ( ticket ) {
      r->save( ticket );
  }
}

void ResourceSelection::showInfo()
{
  ResourceItem *item = selectedItem();
  if ( !item ) return;

//   TQString txt = "<qt>" + item->resource()->infoText() + "</qt>";
//   KMessageBox::information( this, txt );
}

TQString ResourceSelection::title() const
{
  return i18n( "Address Books" );
}

TQString ResourceSelection::identifier() const
{
  return "resourceselection";
}

void ResourceSelection::add()
{
  TQStringList types = mManager->resourceTypeNames();
  TQStringList descs = mManager->resourceTypeDescriptions();

  bool ok = false;
  TQString desc = KInputDialog::getItem( i18n( "Add Address Book" ),
                                        i18n( "Please select type of the new address book:" ),
                                        descs, 0, false, &ok, this );
  if ( !ok )
    return;

  TQString type = types[ descs.findIndex( desc ) ];

  // Create new resource
  KABC::Resource *resource = mManager->createResource( type );
  if ( !resource ) {
    KMessageBox::error( this, i18n("<qt>Unable to create an address book of type <b>%1</b>.</qt>")
                              .arg( type ) );
    return;
  }

  resource->setAddressBook(core()->addressBook());

  KRES::ConfigDialog dlg( this, TQString( "contact" ), resource );

  if ( dlg.exec() ) {
    core()->addressBook()->addResource( resource );
    resource->asyncLoad();

    mLastResource = resource->identifier();
    updateView();
    currentChanged(mListView->currentItem() );
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

  // view items can change during "edit", e.g. sub resources being removed ->
  // sub resource item removed
  // thus keep their data rather than their pointer
  KABC::Resource *resource = item->resource();

  KRES::ConfigDialog dlg( this, TQString( "contact" ), resource );

  if ( dlg.exec() ) {
    mManager->change( resource );
    resource->asyncLoad();

    mLastResource = resource->identifier();
    updateView();
  }
}

void ResourceSelection::remove()
{
  ResourceItem *item = selectedItem();
  if ( !item )
    return;

  int result = KMessageBox::warningContinueCancel( this,
        i18n( "<qt>Do you really want to remove the address book <b>%1</b>?</qt>" )
        .arg( item->resource()->resourceName() ), "",
        KGuiItem( i18n( "&Remove" ), "editdelete" ) );
  if ( result == KMessageBox::Cancel )
    return;

  mLastResource = item->resource()->identifier();

  core()->addressBook()->removeResource( item->resource() );
  core()->addressBook()->emitAddressBookChanged();

  updateView();
  currentChanged(mListView->currentItem() );
}

void ResourceSelection::currentChanged( TQListViewItem *item )
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
      KPIM::ResourceABC *res = dynamic_cast<KPIM::ResourceABC *>( resource );
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

    ResourceItem *item = new ResourceItem( mListView, *it );
    KPIM::ResourceABC* resource = dynamic_cast<KPIM::ResourceABC *>( *it );
    if ( resource ) {
      disconnect( resource, 0, this, 0 );
      connect( resource, TQT_SIGNAL( signalSubresourceAdded( KPIM::ResourceABC *,
                                                         const TQString &, const TQString & ) ),
               TQT_SLOT( slotSubresourceAdded( KPIM::ResourceABC *,
                                           const TQString &, const TQString & ) ) );

      connect( resource, TQT_SIGNAL( signalSubresourceRemoved( KPIM::ResourceABC *,
                                                           const TQString &, const TQString & ) ),
               TQT_SLOT( slotSubresourceRemoved( KPIM::ResourceABC *,
                                             const TQString &, const TQString & ) ) );

      connect( resource, TQT_SIGNAL( signalSubresourceChanged( KPIM::ResourceABC *,
                                                           const TQString &, const TQString & ) ),
               TQT_SLOT( slotSubresourceChanged( KPIM::ResourceABC *,
                                             const TQString &, const TQString & ) ) );

      //connect( resource, TQT_SIGNAL( resourceSaved( KPIM::ResourceABC * ) ),
      //         TQT_SLOT( closeResource( KPIM::ResourceABC * ) ) );
      item->createSubresourceItems();
    }
  }

  TQListViewItemIterator itemIt( mListView );
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
                                              const TQString& /*type*/,
                                              const TQString& subResource )
{
  kdDebug(5720) << k_funcinfo << resource->resourceName() << " " << subResource << endl;
  TQListViewItem *i = mListView->findItem( resource->resourceName(), 0 );
  if ( !i )
    // Not found
    return;

  ResourceItem *item = static_cast<ResourceItem *>( i );
  // Make sure all other sub items have already been created
  item->createSubresourceItems();

  // check if we already have an item for it
  if ( !findSubResourceItem( resource, subResource ) ) {
      (void)new ResourceItem( resource, item, subResource );
  }
}

// Remove an entry
void ResourceSelection::slotSubresourceRemoved( KPIM::ResourceABC* resource,
                                                const TQString& /*type*/,
                                                const TQString& subResource )
{
  ResourceItem *item = findSubResourceItem( resource, subResource );
  delete item;
  core()->addressBook()->emitAddressBookChanged();
  updateView();
}

// change an entry
void ResourceSelection::slotSubresourceChanged( KPIM::ResourceABC* resource,
                                                const TQString& type,
                                                const TQString& subResource )
{
  kdDebug(5720) << resource->resourceName() << subResource;

  ResourceItem *item = findSubResourceItem( resource, subResource );
  if ( item == 0 ) {
      kdWarning(5720) << "Changed before it was added?";
      slotSubresourceAdded( resource, type, subResource );
      return;
  }

  item->setText( 0, resource->subresourceLabel( subResource ) );
  item->setChecked( resource->subresourceActive( subResource ) ? true : false );
  // TODO
  //emitResourcesChanged();
  core()->addressBook()->emitAddressBookChanged();
  updateView();
}

ResourceItem* ResourceSelection::selectedItem() const
{
  return static_cast<ResourceItem*>( mListView->selectedItem() );
}

ResourceItem* ResourceSelection::findSubResourceItem( KPIM::ResourceABC *resource,
                                                const TQString &subResource )
{
    TQListViewItemIterator parentIt( mListView );
    for ( ; *parentIt; ++parentIt ) {
        if ( static_cast<ResourceItem*>(*parentIt)->resource() != resource )
            continue;

        TQListViewItemIterator childIt( *parentIt );
        for ( ; *childIt; ++childIt ) {
            ResourceItem *item = static_cast<ResourceItem*>(*childIt);
            if ( item->resourceIdentifier() == subResource )
                return item;
        }
    }

    return 0;
}

void ResourceSelection::initGUI()
{
  TQBoxLayout *topLayout = new TQVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  TQBoxLayout *buttonLayout = new TQHBoxLayout();
  buttonLayout->setSpacing( KDialog::spacingHint() );
  topLayout->addLayout( buttonLayout );

  TQLabel *abLabel = new TQLabel( i18n( "Address Books" ), this );
  buttonLayout->addWidget( abLabel );
  buttonLayout->addStretch( 1 );

  mAddButton = new TQPushButton( this );
  mAddButton->setIconSet( SmallIconSet( "add" ) );
  TQToolTip::add( mAddButton, i18n( "Add addressbook" ) );
  buttonLayout->addWidget( mAddButton );
  mEditButton = new TQPushButton( this );
  mEditButton->setIconSet( SmallIconSet( "edit" ) );
  mEditButton->setEnabled( false );
  TQToolTip::add( mEditButton, i18n( "Edit addressbook settings" ) );
  buttonLayout->addWidget( mEditButton );
  mRemoveButton = new TQPushButton( this );
  mRemoveButton->setIconSet( SmallIconSet( "remove" ) );
  mRemoveButton->setEnabled( false );
  TQToolTip::add( mRemoveButton, i18n( "Remove addressbook" ) );
  buttonLayout->addWidget( mRemoveButton );

  mListView = new KListView( this );
  mListView->header()->hide();
  mListView->addColumn( i18n( "Address Books" ) );
  mListView->setFullWidth( true );
  topLayout->addWidget( mListView );
}

class ResourceSelectionFactory : public KAB::ExtensionFactory
{
  public:
    KAB::ExtensionWidget *extension( KAB::Core *core, TQWidget *parent, const char *name )
    {
      return new ResourceSelection( core, parent, name );
    }

    TQString identifier() const
    {
      return "resourceselection";
    }
};

extern "C" {
  void *init_libkaddrbk_resourceselection()
  {
    return ( new ResourceSelectionFactory );
  }
}

#include "resourceselection.moc"
