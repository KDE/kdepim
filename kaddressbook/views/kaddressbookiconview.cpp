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
#include <q3iconview.h>
#include <QLayout>
#include <qstringlist.h>
//Added by qt3to4:
#include <QPixmap>
#include <QKeyEvent>
#include <QEvent>
#include <QDropEvent>
#include <QList>
#include <QVBoxLayout>

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
    KAddressBookView *view( KAB::Core *core, QWidget *parent )
    {
      return new KAddressBookIconView( core, parent );
    }

    QString type() const { return I18N_NOOP( "Icon" ); }

    QString description() const { return i18n( "Icons represent contacts. Very simple view." ); }
};

extern "C" {
  KDE_EXPORT void *init_libkaddrbk_iconview()
  {
    return ( new IconViewFactory );
  }
}

AddresseeIconView::AddresseeIconView( QWidget *parent, const char *name )
  : K3IconView( parent, name )
{
  setSelectionMode( Q3IconView::Extended );
  setResizeMode( Q3IconView::Adjust );
  setWordWrapIconText( true );
  setGridX( 100 );
  setItemsMovable( false );
  setSorting( true, true );
  setMode( K3IconView::Select );

  connect( this, SIGNAL( dropped( QDropEvent*, const QList<Q3IconDragItem>& ) ),
           this, SLOT( itemDropped( QDropEvent*, const QList<Q3IconDragItem>& ) ) );
}

AddresseeIconView::~AddresseeIconView()
{
}

void AddresseeIconView::itemDropped( QDropEvent *event, const QList<Q3IconDragItem>& )
{
  emit addresseeDropped( event );
}

Q3DragObject *AddresseeIconView::dragObject()
{
  emit startAddresseeDrag();

  // We never want IconView to start the drag
  return 0;
}


class AddresseeIconViewItem : public K3IconViewItem
{
  public:
    AddresseeIconViewItem( const KABC::Field::List&, KABC::AddressBook *doc,
                           const KABC::Addressee &addr, Q3IconView *parent )
      : K3IconViewItem( parent ), mDocument( doc ), mAddressee( addr )
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
      QPixmap defaultIcon( KGlobal::iconLoader()->loadIcon( "vcard", K3Icon::Desktop ) );
      KABC::Picture pic = mAddressee.photo();
      if ( pic.data().isNull() )
        pic = mAddressee.logo();

      if ( pic.isIntern() && !pic.data().isNull() ) {
        QImage img = pic.data();
        if ( img.width() > img.height() )
          icon = QPixmap::fromImage( img.scaleWidth( 32 ) );
        else
          icon = QPixmap::fromImage( img.scaleHeight( 32 ) );
      } else
        icon = defaultIcon;

      setPixmap( icon );
    }

  private:
    KABC::AddressBook *mDocument;
    KABC::Addressee mAddressee;
};


KAddressBookIconView::KAddressBookIconView( KAB::Core *core,
                                            QWidget *parent)
  : KAddressBookView( core, parent )
{
  QVBoxLayout *layout = new QVBoxLayout( viewWidget() );

  mIconView = new AddresseeIconView( viewWidget(), "mIconView" );
  layout->addWidget( mIconView );

  // Connect up the signals
  connect( mIconView, SIGNAL( executed( Q3IconViewItem* ) ),
           this, SLOT( addresseeExecuted( Q3IconViewItem* ) ) );
  connect( mIconView, SIGNAL( selectionChanged() ),
           this, SLOT( addresseeSelected() ) );
  connect( mIconView, SIGNAL( addresseeDropped( QDropEvent* ) ),
           this, SIGNAL( dropped( QDropEvent* ) ) );
  connect( mIconView, SIGNAL( startAddresseeDrag() ),
           this, SIGNAL( startDrag() ) );
  connect( mIconView, SIGNAL( contextMenuRequested( Q3IconViewItem*, const QPoint& ) ),
           this, SLOT( rmbClicked( Q3IconViewItem*, const QPoint& ) ) );
}

KAddressBookIconView::~KAddressBookIconView()
{
}

KABC::Field *KAddressBookIconView::sortField() const
{
  // we have hardcoded sorting, so we have to return a hardcoded field :(
  return KABC::Field::allFields()[ 2 ];
}

void KAddressBookIconView::readConfig( KConfigGroup &config )
{
  KAddressBookView::readConfig( config );

  disconnect( mIconView, SIGNAL( executed( Q3IconViewItem* ) ),
              this, SLOT( addresseeExecuted( Q3IconViewItem* ) ) );

  if ( KABPrefs::instance()->honorSingleClick() )
    connect( mIconView, SIGNAL( executed( Q3IconViewItem* ) ),
             this, SLOT( addresseeExecuted( Q3IconViewItem* ) ) );
  else
    connect( mIconView, SIGNAL( doubleClicked( Q3IconViewItem* ) ),
             this, SLOT( addresseeExecuted( Q3IconViewItem* ) ) );
}

QStringList KAddressBookIconView::selectedUids()
{
  QStringList uidList;
  Q3IconViewItem *item;
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
  Q3IconViewItem *item;
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

    refresh( QString() );
  }
}

void KAddressBookIconView::setSelected( const QString &uid, bool selected )
{
  Q3IconViewItem *item;
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

void KAddressBookIconView::addresseeExecuted( Q3IconViewItem *item )
{
  AddresseeIconViewItem *aItem = dynamic_cast<AddresseeIconViewItem*>( item );

  if ( aItem )
    emit executed( aItem->addressee().uid() );
}

void KAddressBookIconView::addresseeSelected()
{
  Q3IconViewItem *item;
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
    emit selected( QString() );
}

void KAddressBookIconView::rmbClicked( Q3IconViewItem*, const QPoint &point )
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
