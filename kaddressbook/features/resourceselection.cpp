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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <qlayout.h>
#include <qpushbutton.h>
#include <qtimer.h>

#include <kabc/resource.h>
#include <kdialog.h>
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

class ResourceItem : public QCheckListItem
{
  public:
    ResourceItem( KListView *parent, KABC::Resource *resource )
      : QCheckListItem( parent, "", CheckBox ),
        mResource( resource ), mChecked( false )
    {
      setOn( resource->isActive() );
      mChecked = resource->isActive();
    }

    virtual QString text( int column ) const
    {
      if ( column == 0 )
        return mResource->resourceName();
      else
        return QString::null;
    }

    void setChecked( bool state )
    {
      mChecked = state;
    }

    bool checked() const { return mChecked; }
    KABC::Resource *resource() const { return mResource; }

  private:
    KABC::Resource *mResource;
    bool mChecked;
};

ResourceSelection::ResourceSelection( KAB::Core *core, QWidget *parent, const char *name )
  : KAB::ExtensionWidget( core, parent, name ), mManager( 0 )
{
  initGUI();

  AddressBookWrapper *wrapper = static_cast<AddressBookWrapper*>( core->addressBook() );
  mManager = wrapper->getResourceManager();

  connect( mAddButton, SIGNAL( clicked() ), SLOT( add() ) );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( edit() ) );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( remove() ) );

  connect( mView, SIGNAL( clicked( QListViewItem* ) ),
           SLOT( currentChanged( QListViewItem* ) ) );

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

  QString type = types[ descs.findIndex( desc ) ];

  // Create new resource
  KABC::Resource *resource = mManager->createResource( type );
  if( !resource ) {
    KMessageBox::error( this, i18n("<qt>Unable to create an address book of type <b>%1</b>.</qt>")
                              .arg( type ) );
    return;
  }

  resource->setResourceName( i18n( "%1 address book" ).arg( type ) );

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
        i18n( "<qt>Do you really want to remove the address book <b>%1</b>?</qt>" )
        .arg( item->resource()->resourceName() ), "",
        KGuiItem( i18n( "&Remove" ), "editdelete" ) );
  if ( result == KMessageBox::Cancel )
    return;

  mLastResource = item->resource()->identifier();

  core()->addressBook()->removeResource( item->resource() );
  core()->addressBook()->emitAddressBookChanged();

  updateView();
}

void ResourceSelection::currentChanged( QListViewItem *item )
{
  bool state = (item != 0);

  mEditButton->setEnabled( state );
  mRemoveButton->setEnabled( state );

  ResourceItem *resItem = static_cast<ResourceItem*>( item );
  if ( !resItem )
    return;

  KABC::Resource *resource = resItem->resource();

  if ( resItem->checked() != resItem->isOn() ) {
    resItem->setChecked( resItem->isOn() );
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

    mLastResource = resource->identifier();
    updateView();
  }
}

void ResourceSelection::updateView()
{
  if ( !mManager )
    return;

  mView->clear();

  KRES::Manager<KABC::Resource>::Iterator it;
  for ( it = mManager->begin(); it != mManager->end(); ++it )
    new ResourceItem( mView, *it );

  QListViewItemIterator itemIt( mView );
  while ( itemIt.current() ) {
    ResourceItem *item = static_cast<ResourceItem*>( itemIt.current() );
    if ( item->resource()->identifier() == mLastResource ) {
      mView->setSelected( item, true );
      mView->ensureItemVisible( item );
      break;
    }
    ++itemIt;
  }

  core()->addressBook()->emitAddressBookChanged();
}

ResourceItem* ResourceSelection::selectedItem() const
{
  return static_cast<ResourceItem*>( mView->selectedItem() );
}

void ResourceSelection::initGUI()
{
  QGridLayout *layout = new QGridLayout( this, 2, 3, 2, 5 );

  mView = new KListView( this );
  mView->addColumn( i18n( "Address Books" ) );
  mView->setFullWidth( true );
  layout->addMultiCellWidget( mView, 0, 0, 0, 2 );

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
    KAB::ExtensionWidget *extension( KAB::Core *core, QWidget *parent, const char *name )
    {
      return new ResourceSelection( core, parent, name );
    }

    QString identifier() const
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
