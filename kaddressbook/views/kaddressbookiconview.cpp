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

#include <qapplication.h>
#include <qiconview.h>
#include <qlayout.h>
#include <qstringlist.h>

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
    KAddressBookView *view( KAB::Core *core, QWidget *parent, const char *name )
    {
      return new KAddressBookIconView( core, parent, name );
    }

    QString type() const { return I18N_NOOP( "Icon" ); }

    QString description() const { return i18n( "Icons represent contacts. Very simple view." ); }
};

extern "C" {
  void *init_libkaddrbk_iconview()
  {
    return ( new IconViewFactory );
  }
}

AddresseeIconView::AddresseeIconView( QWidget *parent, const char *name )
  : KIconView( parent, name )
{
  setSelectionMode( QIconView::Extended );
  setResizeMode( QIconView::Adjust );
  setWordWrapIconText( true );
  setGridX( 100 );
  setItemsMovable( false );
  setSorting( true, true );
  setMode( KIconView::Select );

  connect( this, SIGNAL( dropped( QDropEvent*, const QValueList<QIconDragItem>& ) ),
           this, SLOT( itemDropped( QDropEvent*, const QValueList<QIconDragItem>& ) ) );
}

AddresseeIconView::~AddresseeIconView()
{
}

void AddresseeIconView::itemDropped( QDropEvent *event, const QValueList<QIconDragItem>& )
{
  emit addresseeDropped( event );
}

QDragObject *AddresseeIconView::dragObject()
{
  emit startAddresseeDrag();

  // We never want IconView to start the drag
  return 0;
}


class AddresseeIconViewItem : public KIconViewItem
{
  public:
    AddresseeIconViewItem( const KABC::Field::List&, KABC::AddressBook *doc,
                           const KABC::Addressee &addr, QIconView *parent )
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

      QPixmap icon;
      QPixmap defaultIcon( KGlobal::iconLoader()->loadIcon( "vcard", KIcon::Desktop ) );
      KABC::Picture pic = mAddressee.photo();
      if ( pic.data().isNull() )
        pic = mAddressee.logo();

      if ( pic.isIntern() && !pic.data().isNull() ) {
        QImage img = pic.data();
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
                                            QWidget *parent, const char *name)
  : KAddressBookView( core, parent, name )
{
  QVBoxLayout *layout = new QVBoxLayout( viewWidget() );

  mIconView = new AddresseeIconView( viewWidget(), "mIconView" );
  layout->addWidget( mIconView );

  // Connect up the signals
  connect( mIconView, SIGNAL( executed( QIconViewItem* ) ),
           this, SLOT( addresseeExecuted( QIconViewItem* ) ) );
  connect( mIconView, SIGNAL( selectionChanged() ),
           this, SLOT( addresseeSelected() ) );
  connect( mIconView, SIGNAL( addresseeDropped( QDropEvent* ) ),
           this, SIGNAL( dropped( QDropEvent* ) ) );
  connect( mIconView, SIGNAL( startAddresseeDrag() ),
           this, SIGNAL( startDrag() ) );
  connect( mIconView, SIGNAL( contextMenuRequested( QIconViewItem*, const QPoint& ) ),
           this, SLOT( rmbClicked( QIconViewItem*, const QPoint& ) ) );
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

  disconnect( mIconView, SIGNAL( executed( QIconViewItem* ) ),
              this, SLOT( addresseeExecuted( QIconViewItem* ) ) );

  if ( KABPrefs::instance()->honorSingleClick() )
    connect( mIconView, SIGNAL( executed( QIconViewItem* ) ),
             this, SLOT( addresseeExecuted( QIconViewItem* ) ) );
  else
    connect( mIconView, SIGNAL( doubleClicked( QIconViewItem* ) ),
             this, SLOT( addresseeExecuted( QIconViewItem* ) ) );
}

QStringList KAddressBookIconView::selectedUids()
{
  QStringList uidList;
  QIconViewItem *item;
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

void KAddressBookIconView::refresh( const QString &uid )
{
  QIconViewItem *item;
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

    refresh( QString::null );
  }
}

void KAddressBookIconView::setSelected( const QString &uid, bool selected )
{
  QIconViewItem *item;
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

void KAddressBookIconView::addresseeExecuted( QIconViewItem *item )
{
  AddresseeIconViewItem *aItem = dynamic_cast<AddresseeIconViewItem*>( item );

  if ( aItem )
    emit executed( aItem->addressee().uid() );
}

void KAddressBookIconView::addresseeSelected()
{
  QIconViewItem *item;
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
    emit selected( QString::null );
}

void KAddressBookIconView::rmbClicked( QIconViewItem*, const QPoint &point )
{
  popup( point );
}

void KAddressBookIconView::scrollUp()
{
  QApplication::postEvent( mIconView, new QKeyEvent( QEvent::KeyPress, Qt::Key_Up, 0, 0 ) );
}

void KAddressBookIconView::scrollDown()
{
  QApplication::postEvent( mIconView, new QKeyEvent( QEvent::KeyPress, Qt::Key_Down, 0, 0 ) );
}

#include "kaddressbookiconview.moc"
