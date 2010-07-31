/*
    This file is part of libkdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#include <tqheader.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqsignalmapper.h>
#include <tqtoolbutton.h>

#include <kabc/stdaddressbook.h>
#include <kcombobox.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klistview.h>
#include <klocale.h>

#include "addresseeselector.h"

using namespace KPIM;

class AddresseeSelector::AddressBookManager
{
  public:
    TQStringList titles() const;

    void addResource( KABC::Resource* );
    void addAddressBook( const TQString &title, SelectionItem::List &list );

    void clear();
    bool contains( uint index, const SelectionItem& );

  private:
    struct AddressBookEntry {
      TQString title;
      SelectionItem::List list;
    };

    TQValueList<KABC::Resource*> mResources;
    TQValueList<AddressBookEntry> mAddressBooks;
};

TQStringList AddresseeSelector::AddressBookManager::titles() const
{
  TQStringList titles;

  // we've always an 'all' entry
  titles.append( i18n( "All" ) );

  TQValueList<KABC::Resource*>::ConstIterator resIt;
  for ( resIt = mResources.begin(); resIt != mResources.end(); ++resIt )
    titles.append( (*resIt)->resourceName() );

  TQValueList<AddressBookEntry>::ConstIterator abIt;
  for ( abIt = mAddressBooks.begin(); abIt != mAddressBooks.end(); ++abIt )
    titles.append( (*abIt).title );

  return titles;
}

void AddresseeSelector::AddressBookManager::addResource( KABC::Resource *resource )
{
  if ( mResources.find( resource ) == mResources.end() )
    mResources.append( resource );
}

void AddresseeSelector::AddressBookManager::addAddressBook( const TQString &title,
                                                            SelectionItem::List &list  )
{
  AddressBookEntry entry;
  entry.title = title;
  entry.list = list;


  // TODO: check for duplicates
  mAddressBooks.append( entry );
}

void AddresseeSelector::AddressBookManager::clear()
{
  mResources.clear();
  mAddressBooks.clear();
}

bool AddresseeSelector::AddressBookManager::contains( uint index, const SelectionItem &item )
{
  if ( index == 0 ) // the 'all' entry
    return true;

  if ( mResources.count() > 0 ) {
    if ( index <= mResources.count() ) {
      index--;
      if ( item.addressee().resource() == mResources[ index ] )
        return true;
      else
        return false;
    }
  }

  index = index - mResources.count();

  if ( mAddressBooks.count() > 0 ) {
    if ( index <= mAddressBooks.count() ) {
      index--;
      AddressBookEntry entry = mAddressBooks[ index ];
      SelectionItem::List::ConstIterator it;
      for ( it = entry.list.begin(); it != entry.list.end(); ++it )
        if ( (*it).addressee() == item.addressee() )
          return true;

      return false;
    }
  }

  return false;
}


SelectionItem::SelectionItem( const KABC::Addressee &addressee, uint index )
  : mAddressee( addressee ), mDistributionList( 0 ), mIndex( index )
{
  mField.fill( false, 10 );
}

SelectionItem::SelectionItem( KABC::DistributionList *list, uint index )
  : mDistributionList( list ), mIndex( index )
{
  mField.fill( false, 10 );
}

SelectionItem::SelectionItem()
  : mDistributionList( 0 ), mIndex( 0 )
{
  mField.fill( false, 10 );
}

void SelectionItem::addToField( int index )
{
  mField.setBit( index );
}

void SelectionItem::removeFromField( int index )
{
  mField.clearBit( index );
}

bool SelectionItem::isInField( int index )
{
  return mField.testBit( index );
}

KABC::Addressee SelectionItem::addressee() const
{
  return mAddressee;
}

KABC::DistributionList* SelectionItem::distributionList() const
{
  return mDistributionList;
}

uint SelectionItem::index() const
{
  return mIndex;
}


class SelectionViewItem : public QListViewItem
{
  public:
    SelectionViewItem( TQListView *parent, Selection *selection,
                       SelectionItem *item )
      : TQListViewItem( parent, "" ), mSelection( selection ), mItem( item )
    {
      if ( mItem->distributionList() == 0 )
        mIcon = mSelection->itemIcon( mItem->addressee(), mItem->index() );
      else
        mIcon = mSelection->distributionListIcon( mItem->distributionList() );
    }

    TQString text( int column ) const
    {
      if ( column == 0 ) {
        if ( mItem->distributionList() == 0 )
          return mSelection->itemText( mItem->addressee(), mItem->index() );
        else
          return mSelection->distributionListText( mItem->distributionList() );
      } else
        return TQString::null;
    }

    const TQPixmap* pixmap( int column ) const
    {
      if ( column == 0 ) {
        return &mIcon;
      } else
        return 0;
    }

    SelectionItem* item() const { return mItem; }

  private:
    Selection *mSelection;
    SelectionItem *mItem;
    TQPixmap mIcon;
};

AddresseeSelector::AddresseeSelector( Selection *selection, TQWidget *parent, const char *name )
  : TQWidget( parent, name ), mSelection( selection ), mManager( 0 )
{
  mMoveMapper = new TQSignalMapper( this );
  mRemoveMapper = new TQSignalMapper( this );

  mAddressBookManager = new AddressBookManager();

  initGUI();

  init();

  mSelection->setSelector( this );
}

AddresseeSelector::~AddresseeSelector()
{
  delete mManager;
  mManager = 0;

  delete mAddressBookManager;
  mAddressBookManager = 0;
}

void AddresseeSelector::init()
{
  connect( KABC::StdAddressBook::self( true ), TQT_SIGNAL( addressBookChanged( AddressBook* ) ),
           this, TQT_SLOT( reloadAddressBook() ) );
  connect( mAddresseeFilter, TQT_SIGNAL( textChanged( const TQString& ) ),
           this, TQT_SLOT( updateAddresseeView() ) );
  connect( mAddressBookCombo, TQT_SIGNAL( activated( int ) ),
           this, TQT_SLOT( updateAddresseeView() ) );

  connect( mMoveMapper, TQT_SIGNAL( mapped( int ) ),
           this, TQT_SLOT( move( int ) ) );
  connect( mRemoveMapper, TQT_SIGNAL( mapped( int ) ),
           this, TQT_SLOT( remove( int ) ) );

  reloadAddressBook();
}

void AddresseeSelector::initGUI()
{
  TQGridLayout *layout = new TQGridLayout( this, 2, 3, KDialog::marginHint(), KDialog::spacingHint() );
  TQGridLayout *topLayout = new TQGridLayout( this, 2, 2, KDialog::marginHint() );

  TQLabel *label = new TQLabel( i18n( "Address book:" ), this );
  mAddressBookCombo = new KComboBox( false, this );

  topLayout->addWidget( label, 0, 0 );
  topLayout->addWidget( mAddressBookCombo, 0, 1 );

  label = new TQLabel( i18n( "Search:" ), this );
  mAddresseeFilter = new KLineEdit( this );

  topLayout->addWidget( label, 1, 0 );
  topLayout->addWidget( mAddresseeFilter, 1, 1 );

  topLayout->setColStretch( 1, 1 );

  layout->addMultiCellLayout( topLayout, 0, 0, 0, 2 );

  int row = 1;

  TQIconSet moveSet = KGlobal::iconLoader()->loadIconSet( "next", KIcon::Small );
  TQIconSet removeSet = KGlobal::iconLoader()->loadIconSet( "previous", KIcon::Small );

  uint count = mSelection->fieldCount();
  for ( uint i = 0; i < count; ++i, ++row ) {
    KListView *listView = new KListView( this );
    listView->addColumn( mSelection->fieldTitle( i ) );
    listView->setFullWidth( true );
    mSelectionViews.append( listView );

    connect( listView, TQT_SIGNAL( doubleClicked( TQListViewItem*, const TQPoint&, int ) ),
             mRemoveMapper, TQT_SLOT( map() ) );
    mRemoveMapper->setMapping( listView, i );

    TQVBoxLayout *buttonLayout = new TQVBoxLayout( this );
    buttonLayout->setAlignment( Qt::AlignBottom );
    layout->addLayout( buttonLayout, row, 1 );

    // move button
    TQToolButton *moveButton = new TQToolButton( this );
    moveButton->setIconSet( moveSet );
    moveButton->setFixedSize( 30, 30 );

    connect( moveButton, TQT_SIGNAL( clicked() ),
             mMoveMapper, TQT_SLOT( map() ) );
    mMoveMapper->setMapping( moveButton, i );

    // remove button
    TQToolButton *removeButton = new TQToolButton( this );
    removeButton->setIconSet( removeSet );
    removeButton->setFixedSize( 30, 30 );

    connect( removeButton, TQT_SIGNAL( clicked() ),
             mRemoveMapper, TQT_SLOT( map() ) );
    mRemoveMapper->setMapping( removeButton, i );

    buttonLayout->addWidget( moveButton );
    buttonLayout->addWidget( removeButton );

    layout->addWidget( listView, row, 2 );
  }

  mAddresseeView = new KListView( this );
  mAddresseeView->addColumn( "" );
  mAddresseeView->header()->hide();
  mAddresseeView->setFullWidth( true );

  layout->addMultiCellWidget( mAddresseeView, 1, row, 0, 0 );
}

void AddresseeSelector::finish()
{
  SelectionItem::List::Iterator it;

  for ( uint field = 0; field < mSelection->fieldCount(); ++field ) {
    for ( it = mSelectionItems.begin(); it != mSelectionItems.end(); ++it ) {
      if ( (*it).isInField( field ) ) {
        if ( (*it).distributionList() == 0 )
          mSelection->addSelectedAddressees( field, (*it).addressee(), (*it).index() );
        else
          mSelection->addSelectedDistributionList( field, (*it).distributionList() );
      }
    }
  }
}

void AddresseeSelector::updateAddresseeView()
{
  mAddresseeView->clear();

  int addressBookIndex = mAddressBookCombo->currentItem();

  SelectionItem::List::Iterator it;
  for ( it = mSelectionItems.begin(); it != mSelectionItems.end(); ++it ) {
    if ( mAddressBookManager->contains( addressBookIndex, *it ) ) {
      if ( (*it).distributionList() == 0 ) {
        if ( mAddresseeFilter->text().isEmpty() ||
             mSelection->itemMatches( (*it).addressee(), (*it).index(),
                                      mAddresseeFilter->text() ) )
          new SelectionViewItem( mAddresseeView, mSelection, &(*it) );
      } else {
        if ( mAddresseeFilter->text().isEmpty() ||
             mSelection->distributionListMatches( (*it).distributionList(),
                                                  mAddresseeFilter->text() ) )
          new SelectionViewItem( mAddresseeView, mSelection, &(*it) );
      }
    }
  }

  updateSelectionViews();
}

void AddresseeSelector::move( int index )
{
  SelectionViewItem *item = dynamic_cast<SelectionViewItem*>( mAddresseeView->selectedItem() );
  if ( item ) {
    item->item()->addToField( index );
    updateSelectionView( index );
  }
}

void AddresseeSelector::remove( int index )
{
  KListView *view = mSelectionViews[ index ];

  SelectionViewItem *item = dynamic_cast<SelectionViewItem*>( view->selectedItem() );
  if ( item ) {
    item->item()->removeFromField( index );
    updateSelectionView( index );
  }
}

void AddresseeSelector::setItemSelected( uint fieldIndex, const KABC::Addressee &addr, uint itemIndex )
{
  bool found = false;

  SelectionItem::List::Iterator it;
  for ( it = mSelectionItems.begin(); it != mSelectionItems.end(); ++it ) {
    if ( (*it).addressee() == addr && (*it).index() == itemIndex ) {
      (*it).addToField( fieldIndex );
      found = true;
    }
  }

  if ( !found ) {
    SelectionItem item( addr, itemIndex );
    item.addToField( fieldIndex );

    mSelectionItems.append( item );
  }

  updateSelectionView( fieldIndex );
}

void AddresseeSelector::setItemSelected( uint fieldIndex, const KABC::Addressee &addr,
                                         uint itemIndex, const TQString &text )
{
  bool found = false;

  SelectionItem::List::Iterator it;
  for ( it = mSelectionItems.begin(); it != mSelectionItems.end(); ++it ) {
    if ( mSelection->itemEquals( (*it).addressee(), (*it).index(), text ) ) {
      (*it).addToField( fieldIndex );
      found = true;
    }
  }

  if ( !found ) {
    SelectionItem item( addr, itemIndex );
    item.addToField( fieldIndex );

    mSelectionItems.append( item );
  }

  updateSelectionView( fieldIndex );
}

void AddresseeSelector::updateSelectionView( int index )
{
  KListView *view = mSelectionViews[ index ];
  view->clear();

  SelectionItem::List::Iterator it;
  for ( it = mSelectionItems.begin(); it != mSelectionItems.end(); ++it ) {
    if ( (*it).isInField( index ) )
      new SelectionViewItem( view, mSelection, &(*it) );
  }
}

void AddresseeSelector::updateSelectionViews()
{
  for ( uint i = 0; i < mSelection->fieldCount(); ++i )
    updateSelectionView( i );
}

void AddresseeSelector::reloadAddressBook()
{
  // load contacts
  KABC::Addressee::List list = KABC::StdAddressBook::self( true )->allAddressees();
  KABC::Addressee::List::Iterator it;

  SelectionItem::List selectedItems;

  SelectionItem::List::Iterator itemIt;
  for ( itemIt = mSelectionItems.begin(); itemIt != mSelectionItems.end(); ++itemIt ) {
    bool isSelected = false;
    for ( uint i = 0; i < mSelection->fieldCount(); ++i ) {
      if ( (*itemIt).isInField( i ) ) {
        isSelected = true;
        break;
      }
    }

    // we don't save distribution lists, since this leads to crashes
    if ( isSelected && (*itemIt).distributionList() == 0 ) {
      selectedItems.append( *itemIt );
    }
  }

  mSelectionItems.clear();
  mSelectionItems = selectedItems;

  for ( it = list.begin(); it != list.end(); ++it ) {
    uint itemCount = mSelection->itemCount( *it );
    for ( uint index = 0; index < itemCount; ++index ) {
      bool available = false;
      for ( itemIt = mSelectionItems.begin(); itemIt != mSelectionItems.end(); ++itemIt ) {
        if ( (*itemIt).addressee() == (*it) && (*itemIt).index() == index ) {
          available = true;
          break;
        }
      }

      if ( !available ) {
        SelectionItem item( *it, index );
        mSelectionItems.append( item );
      }
    }
  }

  // load distribution lists
  delete mManager;
  mManager = new KABC::DistributionListManager( KABC::StdAddressBook::self( true ) );

  mManager->load();

  TQStringList lists = mManager->listNames();

  TQStringList::Iterator listIt;
  for ( listIt = lists.begin(); listIt != lists.end(); ++listIt ) {
    KABC::DistributionList *list = mManager->list( *listIt );
    SelectionItem item( list, 0 );
    mSelectionItems.append( item );
  }

  mAddressBookManager->clear();

  // update address book combo
  mAddressBookCombo->clear();

  TQPtrList<KABC::Resource> resources = KABC::StdAddressBook::self( true )->resources();
  TQPtrListIterator<KABC::Resource> resIt( resources );
  while ( resIt.current() ) {
    if ( resIt.current()->isActive() )
      mAddressBookManager->addResource( resIt );

    ++resIt;
  }

  for ( uint i = 0; i < mSelection->addressBookCount(); ++i ) {
    SelectionItem::List itemList;

    KABC::Addressee::List addrList = mSelection->addressBookContent( i );
    for ( it = addrList.begin(); it != addrList.end(); ++it ) {
      uint itemCount = mSelection->itemCount( *it );
      for ( uint index = 0; index < itemCount; ++index ) {
        SelectionItem item( *it, index );
        mSelectionItems.append( item );
        itemList.append( item );
      }
    }

    mAddressBookManager->addAddressBook( mSelection->addressBookTitle( i ),
                                         itemList );
  }

  mAddressBookCombo->insertStringList( mAddressBookManager->titles() );

  updateAddresseeView();
}


AddresseeSelectorDialog::AddresseeSelectorDialog( Selection *selection,
                                                  TQWidget *parent, const char *name )
  : KDialogBase( Plain, "", Ok | Cancel, Ok, parent, name, true )
{
  TQFrame *frame = plainPage();
  TQVBoxLayout *layout = new TQVBoxLayout( frame );
  mSelector = new KPIM::AddresseeSelector( selection, frame );
  layout->addWidget( mSelector );

  resize( 500, 490 );
}

void AddresseeSelectorDialog::accept()
{
  mSelector->finish();
  TQDialog::accept();
}

#include "addresseeselector.moc"
