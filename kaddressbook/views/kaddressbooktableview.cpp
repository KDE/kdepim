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
#include <qlayout.h>
#include <q3header.h>
#include <q3vbox.h>
#include <q3listbox.h>
#include <qwidget.h>
#include <qfile.h>
#include <qimage.h>
#include <qcombobox.h>
#include <qapplication.h>
#include <q3dragobject.h>
#include <qevent.h>
#include <q3url.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QDropEvent>
#include <Q3PtrList>

#include <kabc/addressbook.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcolorbutton.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kurlrequester.h>
#include <kimproxy.h>

#include "configuretableviewdialog.h"
#include "contactlistview.h"
#include "core.h"
#include "kabprefs.h"
#include "undocmds.h"

#include "kaddressbooktableview.h"

class TableViewFactory : public ViewFactory
{
  public:
    KAddressBookView *view( KAB::Core *core, QWidget *parent, const char *name )
    {
      return new KAddressBookTableView( core, parent, name );
    }

    QString type() const { return I18N_NOOP( "Table" ); }

    QString description() const { return i18n( "A listing of contacts in a table. Each cell of "
                                  "the table holds a field of the contact." ); }

    ViewConfigureWidget *configureWidget( KABC::AddressBook *ab, QWidget *parent,
                                          const char *name = 0 )
    {
      return new ConfigureTableViewWidget( ab, parent, name );
    }
};

extern "C" {
  void *init_libkaddrbk_tableview()
  {
    return ( new TableViewFactory );
  }
}

KAddressBookTableView::KAddressBookTableView( KAB::Core *core,
                                              QWidget *parent, const char *name )
  : KAddressBookView( core, parent, name )
{
  mMainLayout = new QVBoxLayout( viewWidget(), 2 );

  // The list view will be created when the config is read.
  mListView = 0;
  mIMProxy = 0;
}

KAddressBookTableView::~KAddressBookTableView()
{
}

void KAddressBookTableView::reconstructListView()
{
  if ( mListView ) {
    disconnect( mListView, SIGNAL( selectionChanged() ),
                this, SLOT( addresseeSelected() ) );
    disconnect( mListView, SIGNAL( executed( Q3ListViewItem* ) ),
                this, SLOT( addresseeExecuted( Q3ListViewItem* ) ) );
    disconnect( mListView, SIGNAL( doubleClicked( Q3ListViewItem* ) ),
                this, SLOT( addresseeExecuted( Q3ListViewItem* ) ) );
    disconnect( mListView, SIGNAL( startAddresseeDrag() ),
                this, SIGNAL( startDrag() ) );
    disconnect( mListView, SIGNAL( addresseeDropped( QDropEvent* ) ),
                this, SIGNAL( dropped( QDropEvent* ) ) );
    delete mListView;
  }

  mListView = new ContactListView( this, core()->addressBook(), viewWidget() );

  mListView->setShowIM( mIMProxy != 0 );

  // Add the columns
  const KABC::Field::List fieldList( fields() );
  KABC::Field::List::ConstIterator it;

  int c = 0;
  for ( it = fieldList.begin(); it != fieldList.end(); ++it ) {
    mListView->addColumn( (*it)->label() );
    mListView->setColumnWidthMode( c++, Q3ListView::Manual );
  }

  if ( mListView->showIM() ) {
    // IM presence is added separately, because it's not a KABC field.
    // If you want to make this appear as the leftmost column by default, move
    // this block immediately before the preceding for loop
    // after the declaration of c.
    mListView->addColumn( i18n( "Presence" ) );
    mListView->setIMColumn( c++ );
  }

  mListView->setFullWidth( true );

  connect( mListView, SIGNAL( selectionChanged() ),
           this, SLOT( addresseeSelected() ) );
  connect( mListView, SIGNAL( startAddresseeDrag() ),
           this, SIGNAL( startDrag() ) );
  connect( mListView, SIGNAL( addresseeDropped( QDropEvent* ) ),
           this, SIGNAL( dropped( QDropEvent* ) ) );
  connect( mListView, SIGNAL( contextMenu( KListView*, Q3ListViewItem*, const QPoint& ) ),
           this, SLOT( rmbClicked( KListView*, Q3ListViewItem*, const QPoint& ) ) );
  connect( mListView->header(), SIGNAL( clicked( int ) ),
           this, SIGNAL( sortFieldChanged() ) );

  if ( KABPrefs::instance()->honorSingleClick() )
    connect( mListView, SIGNAL( executed( Q3ListViewItem* ) ),
             this, SLOT( addresseeExecuted( Q3ListViewItem* ) ) );
  else
    connect( mListView, SIGNAL( doubleClicked( Q3ListViewItem* ) ),
             this, SLOT( addresseeExecuted( Q3ListViewItem* ) ) );

  refresh();

  mListView->setSorting( 0, true );
  mMainLayout->addWidget( mListView );
  mMainLayout->activate();
  mListView->show();
}

KABC::Field *KAddressBookTableView::sortField() const
{
  // we have hardcoded sorting, so we have to return a hardcoded field :(
  return ( mListView->sortColumn() == -1 ? fields()[ 0 ] : fields()[ mListView->sortColumn() ] );
}

void KAddressBookTableView::writeConfig( KConfig *config )
{
  KAddressBookView::writeConfig( config );

  mListView->saveLayout( config, config->group() );
}

void KAddressBookTableView::readConfig( KConfig *config )
{
  KAddressBookView::readConfig( config );

  if ( config->readBoolEntry( "InstantMessagingPresence", false ) ) {
    if ( !mIMProxy ) {
      mIMProxy = KIMProxy::instance( kapp->dcopClient() );
      connect( mIMProxy, SIGNAL( sigContactPresenceChanged( const QString& ) ),
               this, SLOT( updatePresence( const QString& ) ) );
    }
  } else {
    if ( mIMProxy ) {
      disconnect( mIMProxy, SIGNAL( sigContactPresenceChanged( const QString& ) ),
                  this, SLOT( updatePresence( const QString& ) ) );
      mIMProxy = 0;
    }
  }

  // The config could have changed the fields, so we need to reconstruct
  // the listview.
  reconstructListView();

  // Set the list view options
  mListView->setAlternateBackgroundEnabled( config->readBoolEntry( "ABackground", true ) );
  mListView->setSingleLineEnabled( config->readBoolEntry( "SingleLine", false ) );
  mListView->setToolTipsEnabled( config->readBoolEntry( "ToolTips", true ) );

  if ( config->readBoolEntry( "Background", false ) )
    mListView->setBackgroundPixmap( config->readPathEntry( "BackgroundName" ) );

  // Restore the layout of the listview
  mListView->restoreLayout( config, config->group() );
}

void KAddressBookTableView::refresh( const QString &uid )
{
  if ( uid.isEmpty() ) {
    // Clear the list view
    QString currentUID, nextUID;
    ContactListViewItem *currentItem = dynamic_cast<ContactListViewItem*>( mListView->currentItem() );
    if ( currentItem ) {
      ContactListViewItem *nextItem = dynamic_cast<ContactListViewItem*>( currentItem->itemBelow() );
      if ( nextItem )
        nextUID = nextItem->addressee().uid();
      currentUID = currentItem->addressee().uid();
    }

    mListView->clear();

    currentItem = 0;
    const KABC::Addressee::List addresseeList( addressees() );
    KABC::Addressee::List::ConstIterator it( addresseeList.begin() );
    const KABC::Addressee::List::ConstIterator endIt( addresseeList.end() );
    for ( ; it != endIt; ++it ) {
      ContactListViewItem *item = new ContactListViewItem( *it, mListView,
                                        core()->addressBook(), fields(), mIMProxy );
      if ( (*it).uid() == currentUID )
        currentItem = item;
      else if ( (*it).uid() == nextUID && !currentItem )
        currentItem = item;
    }

    // Sometimes the background pixmap gets messed up when we add lots
    // of items.
    mListView->repaint();

    if ( currentItem ) {
      mListView->setCurrentItem( currentItem );
      mListView->ensureItemVisible( currentItem );
    }
  } else {
    // Only need to update on entry. Iterate through and try to find it
    ContactListViewItem *ceItem;
    Q3PtrList<Q3ListViewItem> selectedItems( mListView->selectedItems() );
    Q3ListViewItem *it;
    for ( it = selectedItems.first(); it; it = selectedItems.next() ) {
      ceItem = dynamic_cast<ContactListViewItem*>( it );
      if ( ceItem && ceItem->addressee().uid() == uid ) {
        ceItem->refresh();
        return;
      }
    }
    refresh( QString::null );
  }
}

QStringList KAddressBookTableView::selectedUids()
{
  QStringList uidList;
  ContactListViewItem *item;

  Q3ListViewItemIterator it( mListView, Q3ListViewItemIterator::Selected );
  while ( it.current() ) {
    item = dynamic_cast<ContactListViewItem*>( it.current() );
    if ( item )
      uidList << item->addressee().uid();

    ++it;
  }

  return uidList;
}

void KAddressBookTableView::setSelected( const QString &uid, bool selected )
{
  if ( uid.isEmpty() )
    mListView->selectAll( selected );
  else {
    Q3ListViewItemIterator it( mListView );
    while ( it.current() ) {
      ContactListViewItem *item = dynamic_cast<ContactListViewItem*>( it.current() );
      if ( item && (item->addressee().uid() == uid) ) {
        mListView->setSelected( item, selected );

        if ( selected )
          mListView->ensureItemVisible( item );
      }

      ++it;
    }
  }
}

void KAddressBookTableView::setFirstSelected( bool selected )
{
  if ( mListView->firstChild() ) {
    mListView->setSelected( mListView->firstChild(), selected );
    mListView->ensureItemVisible( mListView->firstChild() );
  }
}

void KAddressBookTableView::addresseeSelected()
{
  // We need to try to find the first selected item. This might not be the
  // last selected item, but when QListView is in multiselection mode,
  // there is no way to figure out which one was
  // selected last.
  bool found =false;

  Q3ListViewItemIterator it( mListView, Q3ListViewItemIterator::Selected );
  while ( it.current() && !found ) {
    found = true;
    ContactListViewItem *item = dynamic_cast<ContactListViewItem*>( it.current() );
    if ( item )
      emit selected( item->addressee().uid() );

    ++it;
  }

  if ( !found )
      emit selected( QString::null );
}

void KAddressBookTableView::addresseeExecuted( Q3ListViewItem *item )
{
  if ( item ) {
    ContactListViewItem *ceItem = dynamic_cast<ContactListViewItem*>( item );

    if ( ceItem )
      emit executed( ceItem->addressee().uid() );
    else
      emit executed( QString::null );
  } else {
    emit executed( QString::null );
  }
}

void KAddressBookTableView::rmbClicked( KListView*, Q3ListViewItem*, const QPoint &point )
{
  popup( point );
}

void KAddressBookTableView::updatePresence( const QString &uid )
{
  // find the LVI to update and refresh() it
  Q3ListViewItem *item;
  ContactListViewItem *ceItem;
  for ( item = mListView->firstChild(); item; item = item->itemBelow() ) {
    ceItem = dynamic_cast<ContactListViewItem*>( item );
    if ( ( ceItem != 0L ) && ( ceItem->addressee().uid() == uid ) ) {
      ceItem->setHasIM( true );
      ceItem->refresh();
      break;
    }
  }

  if ( mListView->sortColumn() == mListView->imColumn() )
    mListView->sort();
}

void KAddressBookTableView::scrollUp()
{
  QApplication::postEvent( mListView, new QKeyEvent( QEvent::KeyPress, Qt::Key_Up, 0, 0 ) );
}

void KAddressBookTableView::scrollDown()
{
  QApplication::postEvent( mListView, new QKeyEvent( QEvent::KeyPress, Qt::Key_Down, 0, 0 ) );
}


#include "kaddressbooktableview.moc"
