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

#include <tqapplication.h>
#include <tqiconview.h>
#include <tqlayout.h>
#include <tqstringlist.h>

#include <kabc/addressbook.h>
#include <kabc/addressee.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>

#include "core.h"
#include "kabprefs.h"

#include "kaddressbookiconview.h"

class IconViewFactory : public ViewFactory
{
  public:
    KAddressBookView *view( KAB::Core *core, TQWidget *parent, const char *name )
    {
      return new KAddressBookIconView( core, parent, name );
    }

    TQString type() const { return I18N_NOOP( "Icon" ); }

    TQString description() const { return i18n( "Icons represent contacts. Very simple view." ); }
};

extern "C" {
  void *init_libkaddrbk_iconview()
  {
    return ( new IconViewFactory );
  }
}

AddresseeIconView::AddresseeIconView( TQWidget *parent, const char *name )
  : KIconView( parent, name )
{
  setSelectionMode( TQIconView::Extended );
  setResizeMode( TQIconView::Adjust );
  setWordWrapIconText( true );
  setGridX( 100 );
  setItemsMovable( false );
  setSorting( true, true );
  setMode( KIconView::Select );

  connect( this, TQT_SIGNAL( dropped( TQDropEvent*, const TQValueList<TQIconDragItem>& ) ),
           this, TQT_SLOT( itemDropped( TQDropEvent*, const TQValueList<TQIconDragItem>& ) ) );
}

AddresseeIconView::~AddresseeIconView()
{
}

void AddresseeIconView::itemDropped( TQDropEvent *event, const TQValueList<TQIconDragItem>& )
{
  emit addresseeDropped( event );
}

TQDragObject *AddresseeIconView::dragObject()
{
  emit startAddresseeDrag();

  // We never want IconView to start the drag
  return 0;
}


class AddresseeIconViewItem : public KIconViewItem
{
  public:
    AddresseeIconViewItem( const KABC::Field::List&, KABC::AddressBook *doc,
                           const KABC::Addressee &addr, TQIconView *parent )
      : KIconViewItem( parent ), mDocument( doc ), mAddressee( addr )
      {
        refresh();
      }

    const KABC::Addressee &addressee() const { return mAddressee; }

    void refresh()
    {
      mAddressee = mDocument->findByUid( mAddressee.uid() );

      if ( !mAddressee.isEmpty() )
        setText( mAddressee.givenName() + " " + mAddressee.familyName() );

      TQPixmap icon;
      TQPixmap defaultIcon( KGlobal::iconLoader()->loadIcon( "vcard", KIcon::Desktop ) );
      KABC::Picture pic = mAddressee.photo();
      if ( pic.data().isNull() )
        pic = mAddressee.logo();

      if ( pic.isIntern() && !pic.data().isNull() ) {
        TQImage img = pic.data();
        if ( img.width() > img.height() )
          icon = img.scaleWidth( 32 );
        else
          icon = img.scaleHeight( 32 );
      } else
        icon = defaultIcon;

      setPixmap( icon );
    }

  private:
    KABC::AddressBook *mDocument;
    KABC::Addressee mAddressee;
};


KAddressBookIconView::KAddressBookIconView( KAB::Core *core,
                                            TQWidget *parent, const char *name)
  : KAddressBookView( core, parent, name )
{
  TQVBoxLayout *layout = new TQVBoxLayout( viewWidget() );

  mIconView = new AddresseeIconView( viewWidget(), "mIconView" );
  layout->addWidget( mIconView );

  // Connect up the signals
  connect( mIconView, TQT_SIGNAL( executed( TQIconViewItem* ) ),
           this, TQT_SLOT( addresseeExecuted( TQIconViewItem* ) ) );
  connect( mIconView, TQT_SIGNAL( selectionChanged() ),
           this, TQT_SLOT( addresseeSelected() ) );
  connect( mIconView, TQT_SIGNAL( addresseeDropped( TQDropEvent* ) ),
           this, TQT_SIGNAL( dropped( TQDropEvent* ) ) );
  connect( mIconView, TQT_SIGNAL( startAddresseeDrag() ),
           this, TQT_SIGNAL( startDrag() ) );
  connect( mIconView, TQT_SIGNAL( contextMenuRequested( TQIconViewItem*, const TQPoint& ) ),
           this, TQT_SLOT( rmbClicked( TQIconViewItem*, const TQPoint& ) ) );
}

KAddressBookIconView::~KAddressBookIconView()
{
}

KABC::Field *KAddressBookIconView::sortField() const
{
  // we have hardcoded sorting, so we have to return a hardcoded field :(
  return KABC::Field::allFields()[ 2 ];
}

void KAddressBookIconView::readConfig( KConfig *config )
{
  KAddressBookView::readConfig( config );

  disconnect( mIconView, TQT_SIGNAL( executed( TQIconViewItem* ) ),
              this, TQT_SLOT( addresseeExecuted( TQIconViewItem* ) ) );

  if ( KABPrefs::instance()->honorSingleClick() )
    connect( mIconView, TQT_SIGNAL( executed( TQIconViewItem* ) ),
             this, TQT_SLOT( addresseeExecuted( TQIconViewItem* ) ) );
  else
    connect( mIconView, TQT_SIGNAL( doubleClicked( TQIconViewItem* ) ),
             this, TQT_SLOT( addresseeExecuted( TQIconViewItem* ) ) );
}

TQStringList KAddressBookIconView::selectedUids()
{
  TQStringList uidList;
  TQIconViewItem *item;
  AddresseeIconViewItem *aItem;

  for ( item = mIconView->firstItem(); item; item = item->nextItem() ) {
    if ( item->isSelected() ) {
      aItem = dynamic_cast<AddresseeIconViewItem*>( item );
      if ( aItem )
        uidList << aItem->addressee().uid();
    }
  }

  return uidList;
}

void KAddressBookIconView::refresh( const TQString &uid )
{
  TQIconViewItem *item;
  AddresseeIconViewItem *aItem;

  if ( uid.isEmpty() ) {
    // Rebuild the view
    mIconView->clear();
    mIconList.clear();

    const KABC::Addressee::List addresseeList( addressees() );
    KABC::Addressee::List::ConstIterator it( addresseeList.begin() );
    const KABC::Addressee::List::ConstIterator endIt( addresseeList.end() );
    for ( ; it != endIt; ++it )
      aItem = new AddresseeIconViewItem( fields(), core()->addressBook(), *it, mIconView );

    mIconView->arrangeItemsInGrid( true );

    for ( item = mIconView->firstItem(); item; item = item->nextItem() ) {
      AddresseeIconViewItem* aivi = dynamic_cast<AddresseeIconViewItem*>( item );
      mIconList.append( aivi );
    }

  } else {
    // Try to find the one to refresh
    for ( item = mIconView->firstItem(); item; item = item->nextItem() ) {
      aItem = dynamic_cast<AddresseeIconViewItem*>( item );
      if ( aItem && (aItem->addressee().uid() == uid) ) {
        aItem->refresh();
        mIconView->arrangeItemsInGrid( true );
        return;
      }
    }

    refresh( TQString::null );
  }
}

void KAddressBookIconView::setSelected( const TQString &uid, bool selected )
{
  TQIconViewItem *item;
  AddresseeIconViewItem *aItem;

  if ( uid.isEmpty() ) {
    mIconView->selectAll( selected );
  } else {
    bool found = false;
    for ( item = mIconView->firstItem(); item && !found; item = item->nextItem() ) {

      aItem = dynamic_cast<AddresseeIconViewItem*>( item );
      if ( aItem && (aItem->addressee().uid() == uid) ) {
        mIconView->setSelected( aItem, selected );
        mIconView->ensureItemVisible( aItem );
        found = true;
      }
    }
  }
}

void KAddressBookIconView::setFirstSelected( bool selected )
{
  if ( mIconView->firstItem() ) {
    mIconView->setSelected( mIconView->firstItem(), selected );
    mIconView->ensureItemVisible( mIconView->firstItem() );
  }
}

void KAddressBookIconView::addresseeExecuted( TQIconViewItem *item )
{
  AddresseeIconViewItem *aItem = dynamic_cast<AddresseeIconViewItem*>( item );

  if ( aItem )
    emit executed( aItem->addressee().uid() );
}

void KAddressBookIconView::addresseeSelected()
{
  TQIconViewItem *item;
  AddresseeIconViewItem *aItem;

  bool found = false;
  for ( item = mIconView->firstItem(); item && !found; item = item->nextItem() ) {
    if ( item->isSelected() ) {
      aItem = dynamic_cast<AddresseeIconViewItem*>( item );
      if ( aItem ) {
        emit selected( aItem->addressee().uid() );
        found = true;
      }
    }
  }

  if ( !found )
    emit selected( TQString::null );
}

void KAddressBookIconView::rmbClicked( TQIconViewItem*, const TQPoint &point )
{
  popup( point );
}

void KAddressBookIconView::scrollUp()
{
  TQApplication::postEvent( mIconView, new TQKeyEvent( TQEvent::KeyPress, Qt::Key_Up, 0, 0 ) );
}

void KAddressBookIconView::scrollDown()
{
  TQApplication::postEvent( mIconView, new TQKeyEvent( TQEvent::KeyPress, Qt::Key_Down, 0, 0 ) );
}

#include "kaddressbookiconview.moc"
