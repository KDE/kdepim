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
#include <tqdragobject.h>
#include <tqevent.h>
#include <tqiconview.h>
#include <tqlayout.h>
#include <tqstringlist.h>

#include <kabc/addressbook.h>
#include <kabc/addressee.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include "core.h"
#include "configurecardviewdialog.h"
#include "kabprefs.h"

#include "kaddressbookcardview.h"

class CardViewFactory : public ViewFactory
{
  public:
    KAddressBookView *view( KAB::Core *core, TQWidget *parent, const char *name )
    {
      return new KAddressBookCardView( core, parent, name );
    }

    TQString type() const { return I18N_NOOP("Card"); }

    TQString description() const { return i18n( "Rolodex style cards represent contacts." ); }

    ViewConfigureWidget *configureWidget( KABC::AddressBook *ab, TQWidget *parent,
                                          const char *name = 0 )
    {
      return new ConfigureCardViewWidget( ab, parent, name );
    }
};

extern "C" {
  void *init_libkaddrbk_cardview()
  {
    return ( new CardViewFactory );
  }
}

class AddresseeCardViewItem : public CardViewItem
{
  public:
    AddresseeCardViewItem( const KABC::Field::List &fields,
                           bool showEmptyFields,
                           KABC::AddressBook *doc, const KABC::Addressee &addr,
                           CardView *parent )
      : CardViewItem( parent, addr.realName() ),
        mFields( fields ), mShowEmptyFields( showEmptyFields ),
        mDocument( doc ), mAddressee( addr )
      {
        if ( mFields.isEmpty() )
          mFields = KABC::Field::defaultFields();

        refresh();
      }

    const KABC::Addressee &addressee() const { return mAddressee; }

    void refresh()
    {
      mAddressee = mDocument->findByUid( mAddressee.uid() );

      if ( !mAddressee.isEmpty() ) {
        clearFields();

        KABC::Field::List::ConstIterator it( mFields.begin() );
        const KABC::Field::List::ConstIterator endIt( mFields.end() );
        for ( ; it != endIt; ++it ) {
          // insert empty fields or not? not doing so saves a bit of memory and CPU
          // (during geometry calculations), but prevents having equally
          // wide label columns in all cards, unless CardViewItem/CardView search
          // globally for the widest label. (anders)

          // if ( mShowEmptyFields || !(*it)->value( mAddressee ).isEmpty() )
          insertField( (*it)->label(), (*it)->value( mAddressee ) );
        }

        setCaption( mAddressee.realName() );
      }
    }

  private:
    KABC::Field::List mFields;
    bool mShowEmptyFields;
    KABC::AddressBook *mDocument;
    KABC::Addressee mAddressee;
};


AddresseeCardView::AddresseeCardView( TQWidget *parent, const char *name )
  : CardView( parent, name )
{
  setAcceptDrops( true );
}

AddresseeCardView::~AddresseeCardView()
{
}

void AddresseeCardView::dragEnterEvent( TQDragEnterEvent *event )
{
  if ( TQTextDrag::canDecode( event ) )
    event->accept();
}

void AddresseeCardView::dropEvent( TQDropEvent *event )
{
  emit addresseeDropped( event );
}

void AddresseeCardView::startDrag()
{
  emit startAddresseeDrag();
}


KAddressBookCardView::KAddressBookCardView( KAB::Core *core,
                                            TQWidget *parent, const char *name )
    : KAddressBookView( core, parent, name )
{
  mShowEmptyFields = false;

  TQVBoxLayout *layout = new TQVBoxLayout( viewWidget() );

  mCardView = new AddresseeCardView( viewWidget(), "mCardView" );
  mCardView->setSelectionMode( CardView::Extended );
  layout->addWidget( mCardView );

  // Connect up the signals
  connect( mCardView, TQT_SIGNAL( executed( CardViewItem* ) ),
           this, TQT_SLOT( addresseeExecuted( CardViewItem* ) ) );
  connect( mCardView, TQT_SIGNAL( selectionChanged() ),
           this, TQT_SLOT( addresseeSelected() ) );
  connect( mCardView, TQT_SIGNAL( addresseeDropped( TQDropEvent* ) ),
           this, TQT_SIGNAL( dropped( TQDropEvent* ) ) );
  connect( mCardView, TQT_SIGNAL( startAddresseeDrag() ),
           this, TQT_SIGNAL( startDrag() ) );
  connect( mCardView, TQT_SIGNAL( contextMenuRequested( CardViewItem*, const TQPoint& ) ),
           this, TQT_SLOT( rmbClicked( CardViewItem*, const TQPoint& ) ) );
}

KAddressBookCardView::~KAddressBookCardView()
{
}

KABC::Field *KAddressBookCardView::sortField() const
{
  // we have hardcoded sorting, so we have to return a hardcoded field :(
  return KABC::Field::allFields()[ 0 ];
}

void KAddressBookCardView::readConfig( KConfig *config )
{
  KAddressBookView::readConfig( config );

  // costum colors?
  if ( config->readBoolEntry( "EnableCustomColors", false ) ) {
    TQPalette p( mCardView->palette() );
    TQColor c = p.color( TQPalette::Normal, TQColorGroup::Base );
    p.setColor( TQPalette::Normal, TQColorGroup::Base, config->readColorEntry( "BackgroundColor", &c ) );
    c = p.color( TQPalette::Normal, TQColorGroup::Text );
    p.setColor( TQPalette::Normal, TQColorGroup::Text, config->readColorEntry( "TextColor", &c ) );
    c = p.color( TQPalette::Normal, TQColorGroup::Button );
    p.setColor( TQPalette::Normal, TQColorGroup::Button, config->readColorEntry( "HeaderColor", &c ) );
    c = p.color( TQPalette::Normal, TQColorGroup::ButtonText );
    p.setColor( TQPalette::Normal, TQColorGroup::ButtonText, config->readColorEntry( "HeaderTextColor", &c ) );
    c = p.color( TQPalette::Normal, TQColorGroup::Highlight );
    p.setColor( TQPalette::Normal, TQColorGroup::Highlight, config->readColorEntry( "HighlightColor", &c ) );
    c = p.color( TQPalette::Normal, TQColorGroup::HighlightedText );
    p.setColor( TQPalette::Normal, TQColorGroup::HighlightedText, config->readColorEntry( "HighlightedTextColor", &c ) );
    mCardView->viewport()->setPalette( p );
  } else {
    // needed if turned off during a session.
    mCardView->viewport()->setPalette( mCardView->palette() );
  }

  //custom fonts?
  TQFont f( font() );
  if ( config->readBoolEntry( "EnableCustomFonts", false ) ) {
    mCardView->setFont( config->readFontEntry( "TextFont", &f ) );
    f.setBold( true );
    mCardView->setHeaderFont( config->readFontEntry( "HeaderFont", &f ) );
  } else {
    mCardView->setFont( f );
    f.setBold( true );
    mCardView->setHeaderFont( f );
  }

  mCardView->setDrawCardBorder( config->readBoolEntry( "DrawBorder", true ) );
  mCardView->setDrawColSeparators( config->readBoolEntry( "DrawSeparators", true ) );
  mCardView->setDrawFieldLabels( config->readBoolEntry( "DrawFieldLabels", false ) );
  mShowEmptyFields = config->readBoolEntry( "ShowEmptyFields", false );

  mCardView->setShowEmptyFields( mShowEmptyFields );

  mCardView->setItemWidth( config->readNumEntry( "ItemWidth", 200 ) );
  mCardView->setItemMargin( config->readNumEntry( "ItemMargin", 0 ) );
  mCardView->setItemSpacing( config->readNumEntry( "ItemSpacing", 10 ) );
  mCardView->setSeparatorWidth( config->readNumEntry( "SeparatorWidth", 2 ) );

  disconnect( mCardView, TQT_SIGNAL( executed( CardViewItem* ) ),
              this, TQT_SLOT( addresseeExecuted( CardViewItem* ) ) );

  if ( KABPrefs::instance()->honorSingleClick() )
    connect( mCardView, TQT_SIGNAL( executed( CardViewItem* ) ),
             this, TQT_SLOT( addresseeExecuted( CardViewItem* ) ) );
  else
    connect( mCardView, TQT_SIGNAL( doubleClicked( CardViewItem* ) ),
             this, TQT_SLOT( addresseeExecuted( CardViewItem* ) ) );
}

void KAddressBookCardView::writeConfig( KConfig *config )
{
  config->writeEntry( "ItemWidth", mCardView->itemWidth() );
  KAddressBookView::writeConfig( config );
}

TQStringList KAddressBookCardView::selectedUids()
{
  TQStringList uidList;
  CardViewItem *item;
  AddresseeCardViewItem *aItem;

  for ( item = mCardView->firstItem(); item; item = item->nextItem() ) {
    if ( item->isSelected() ) {
      aItem = dynamic_cast<AddresseeCardViewItem*>( item );
      if ( aItem )
        uidList << aItem->addressee().uid();
    }
  }

  return uidList;
}

void KAddressBookCardView::refresh( const TQString &uid )
{
  CardViewItem *item;
  AddresseeCardViewItem *aItem;

  if ( uid.isEmpty() ) {
    // Rebuild the view
    mCardView->viewport()->setUpdatesEnabled( false );
    mCardView->clear();

    const KABC::Addressee::List addresseeList( addressees() );
    KABC::Addressee::List::ConstIterator it( addresseeList.begin() );
    const KABC::Addressee::List::ConstIterator endIt( addresseeList.end() );
    for ( ; it != endIt; ++it ) {
      aItem = new AddresseeCardViewItem( fields(), mShowEmptyFields,
                                         core()->addressBook(), *it, mCardView );
    }
    mCardView->viewport()->setUpdatesEnabled( true );
    mCardView->viewport()->update();

    // by default nothing is selected
    emit selected( TQString::null );
  } else {
    // Try to find the one to refresh
    bool found = false;
    for ( item = mCardView->firstItem(); item && !found; item = item->nextItem() ) {
      aItem = dynamic_cast<AddresseeCardViewItem*>( item );
      if ( aItem && (aItem->addressee().uid() == uid) ) {
        aItem->refresh();
        found = true;
      }
    }
  }
}

void KAddressBookCardView::setSelected( const TQString &uid, bool selected )
{
  CardViewItem *item;
  AddresseeCardViewItem *aItem;

  if ( uid.isEmpty() ) {
    mCardView->selectAll( selected );
  } else {
    bool found = false;
    for ( item = mCardView->firstItem(); item && !found; item = item->nextItem() ) {
      aItem = dynamic_cast<AddresseeCardViewItem*>( item );

      if ( aItem && (aItem->addressee().uid() == uid) ) {
        mCardView->setSelected( aItem, selected );
        mCardView->ensureItemVisible( item );
        found = true;
      }
    }
  }
}

void KAddressBookCardView::setFirstSelected( bool selected )
{
  if ( mCardView->firstItem() ) {
    mCardView->setSelected( mCardView->firstItem(), selected );
    mCardView->ensureItemVisible( mCardView->firstItem() );
  }
}

void KAddressBookCardView::addresseeExecuted( CardViewItem *item )
{
  AddresseeCardViewItem *aItem = dynamic_cast<AddresseeCardViewItem*>( item );
  if ( aItem )
    emit executed( aItem->addressee().uid() );
}

void KAddressBookCardView::addresseeSelected()
{
  CardViewItem *item;
  AddresseeCardViewItem *aItem;

  bool found = false;
  for ( item = mCardView->firstItem(); item && !found; item = item->nextItem() ) {
    if ( item->isSelected() ) {
      aItem = dynamic_cast<AddresseeCardViewItem*>( item );
      if ( aItem ) {
        emit selected( aItem->addressee().uid() );
        found = true;
      }
    }
  }

  if ( !found )
    emit selected( TQString::null );
}

void KAddressBookCardView::rmbClicked( CardViewItem*, const TQPoint &point )
{
  popup( point );
}

void KAddressBookCardView::scrollUp()
{
  TQApplication::postEvent( mCardView, new TQKeyEvent( TQEvent::KeyPress, Qt::Key_Up, 0, 0 ) );
}

void KAddressBookCardView::scrollDown()
{
  TQApplication::postEvent( mCardView, new TQKeyEvent( TQEvent::KeyPress, Qt::Key_Down, 0, 0 ) );
}

#include "kaddressbookcardview.moc"
