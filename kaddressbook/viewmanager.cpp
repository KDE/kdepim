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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlayout.h>
#include <qwidgetstack.h>

#include <libkdepim/kvcarddrag.h>
#include <kabc/addressbook.h>
#include <kabc/vcardconverter.h>
#include <kactionclasses.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmultipledrag.h>
#include <ktrader.h>
#include <kurldrag.h>

#include "addviewdialog.h"
#include "addresseeutil.h"
#include "filtereditdialog.h"
#include "kabcore.h"

#include "viewmanager.h"

ViewManager::ViewManager( KABCore *core, QWidget *parent, const char *name )
  : QWidget( parent, name ), mCore( core ), mActiveView( 0 )
{
  initGUI();
  initActions();

  mViewDict.setAutoDelete( true );
  mViewFactoryDict.setAutoDelete( true );

  createViewFactories();
}

ViewManager::~ViewManager()
{
  unloadViews();
  mViewFactoryDict.clear();
}

void ViewManager::restoreSettings()
{
  KConfigGroupSaver viewSaver( mCore->config(), "Views" );
  mViewNameList = mCore->config()->readListEntry( "Names" );
  QString activeViewName = mCore->config()->readEntry( "Active" );

  if ( mViewNameList.size() == 0 )  // Add a default
    mViewNameList.append( i18n( "Default Table View" ) );

  mActionSelectView->setItems( mViewNameList );

  // Filter
  mFilterList = Filter::restore( mCore->config(), "Filter" );

  mActionSelectFilter->setItems( filterNames() );

  KConfigGroupSaver filterSaver( mCore->config(), "Filter" );
  if ( mCore->config()->hasKey( "Active" ) ) {
    uint pos = mCore->config()->readNumEntry( "Active", 0 );
    mActionSelectFilter->setCurrentItem( pos );
  }

  // Tell the views to reread their config, since they may have
  // been modified by global settings
  QDictIterator<KAddressBookView> it( mViewDict );
  for ( it.toFirst(); it.current(); ++it ) {
    KConfigGroupSaver saver( mCore->config(), it.currentKey() );
    it.current()->readConfig( mCore->config() );
  }

  setActiveView( activeViewName );
}

void ViewManager::saveSettings()
{
  QDictIterator<KAddressBookView> it( mViewDict );
  for ( it.toFirst(); it.current(); ++it ) {
    KConfigGroupSaver saver( mCore->config(), it.currentKey() );
    (*it)->writeConfig( mCore->config() );
  }

  Filter::save( mCore->config(), "Filter", mFilterList );
  KConfigGroupSaver filterSaver( mCore->config(), "Filter" );
  mCore->config()->writeEntry( "Active", mActionSelectFilter->currentItem() );

  // write the view name list
  KConfigGroupSaver viewSaver( mCore->config(), "Views" );
  mCore->config()->writeEntry( "Names", mViewNameList );
  mCore->config()->writeEntry( "Active", mActiveView->name() );
}

QStringList ViewManager::selectedUids() const
{
  return mActiveView->selectedUids();
}

QStringList ViewManager::selectedEmails() const
{
  return mActiveView->selectedEmails();
}

void ViewManager::setSelected( const QString &uid, bool selected )
{
  mActiveView->setSelected( uid, selected );
}

void ViewManager::unloadViews()
{
  mViewDict.clear();
  mActiveView = 0;
}

void ViewManager::setActiveView( const QString &name )
{
  KAddressBookView *view = 0;

  // Check that this isn't the same as the current active view
  if ( mActiveView && ( mActiveView->name() == name ) )
    return;

  // At this point we know the view that should be active is not
  // currently active. We will try to find the new on in the list. If
  // we can't find it, it means it hasn't been instantiated, so we will
  // create it on demand.

  view = mViewDict.find( name );

  // Check if we found the view. If we didn't, then we need to create it
  if ( view == 0 ) {
    KConfig *config = mCore->config();
    KConfigGroupSaver saver( config, name );
    QString type = config->readEntry( "Type", "Table" );

    kdDebug(5720) << "ViewManager::setActiveView: creating view - " << name << endl;

    ViewFactory *factory = mViewFactoryDict.find( type );
    if ( factory )
      view = factory->view( mCore->addressBook(), mViewWidgetStack, name.latin1() );

    if ( view ) {
      mViewDict.insert( name, view );
      mViewWidgetStack->addWidget( view );
      view->readConfig( config );

      // The manager just relays the signals
      connect( view, SIGNAL( selected( const QString& ) ),
               SIGNAL( selected( const QString & ) ) );
      connect( view, SIGNAL( executed( const QString& ) ),
               SIGNAL( executed( const QString& ) ) );
      connect( view, SIGNAL( modified() ), SIGNAL( modified() ) );
      connect( view, SIGNAL( dropped( QDropEvent* ) ),
               SLOT( dropped( QDropEvent* ) ) );
      connect( view, SIGNAL( startDrag() ), SLOT( startDrag() ) );
    }
  }

  // If we found or created the view, raise it and refresh it
  if ( view ) {
    mActiveView = view;
    mViewWidgetStack->raiseWidget( view );
    // Set the proper filter in the view. By setting the combo
    // box, the activated slot will be called, which will push
    // the filter to the view and refresh it.
    if ( view->defaultFilterType() == KAddressBookView::None ) {
      mActionSelectFilter->setCurrentItem( 0 );
      setActiveFilter( 0 );
    } else if ( view->defaultFilterType() == KAddressBookView::Active ) {
      setActiveFilter( mActionSelectFilter->currentItem() );
    } else {
      uint pos = filterPosition( view->defaultFilterName() );
      mActionSelectFilter->setCurrentItem( pos );
      setActiveFilter( pos );
    }

    // Update the inc search widget to show the fields in the new active
    // view.
    mCore->setSearchFields( mActiveView->fields() );
    mActiveView->refresh();

  } else
    kdDebug(5720) << "ViewManager::setActiveView: unable to find view\n";
}

void ViewManager::refreshView( const QString &uid )
{
  if ( mActiveView )
    mActiveView->refresh( uid );
}

void ViewManager::editView()
{
  if ( !mActiveView )
    return;

  ViewFactory *factory = mViewFactoryDict.find( mActiveView->type() );
  ViewConfigureWidget *wdg = 0;

  if ( factory ) {
    // Save the filters so the dialog has the latest set
    Filter::save( mCore->config(), "Filter", mFilterList );

    wdg = factory->configureWidget( mCore->addressBook(), 0 );
  }

  if ( wdg ) {
    ViewConfigureDialog dlg( wdg, mActiveView->name(), this );

    KConfigGroupSaver saver( mCore->config(), mActiveView->name() );
    dlg.restoreSettings( mCore->config() );

    if ( dlg.exec() ) {
      dlg.saveSettings( mCore->config() );
      mActiveView->readConfig( mCore->config() );

      // Set the proper filter in the view. By setting the combo
      // box, the activated slot will be called, which will push
      // the filter to the view and refresh it.
      if ( mActiveView->defaultFilterType() == KAddressBookView::None ) {
        mActionSelectFilter->setCurrentItem( 0 );
        setActiveFilter( 0 );
      } else if ( mActiveView->defaultFilterType() == KAddressBookView::Active ) {
        setActiveFilter( mActionSelectFilter->currentItem() );
      } else {
        uint pos = filterPosition( mActiveView->defaultFilterName() );
        mActionSelectFilter->setCurrentItem( pos );
        setActiveFilter( pos );
      }

      mCore->setSearchFields( mActiveView->fields() );
      mActiveView->refresh();
    }
  }
}

void ViewManager::deleteView()
{
  QString text = i18n( "<qt>Are you sure that you want to delete the view <b>%1</b>?</qt>" ).arg( mActiveView->name() );
  QString caption = i18n( "Confirm Delete" );

  if ( KMessageBox::questionYesNo( this, text, caption ) == KMessageBox::Yes ) {
    mViewNameList.remove( mActiveView->name() );

    // remove the view from the config file
    KConfig *config = mCore->config();
    config->deleteGroup( mActiveView->name() );

    mViewDict.remove( mActiveView->name() );
    mActiveView = 0;

    // we are in an invalid state now, but that should be fixed after
    // we emit the signal
    mActionSelectView->setItems( mViewNameList );
    if ( mViewNameList.count() > 0 ) {
      mActionSelectView->setCurrentItem( 0 );
      setActiveView( mViewNameList[ 0 ] );
    }
    mActionDeleteView->setEnabled( mViewNameList.count() > 0 );
  }
}

void ViewManager::addView()
{
  AddViewDialog dialog( &mViewFactoryDict, this );

  if ( dialog.exec() ) {
    QString newName = dialog.viewName();
    QString type = dialog.viewType();

    // Check for name conflicts
    bool firstConflict = true;
    int numTries = 1;
    while ( mViewNameList.contains( newName ) > 0 ) {
      if ( !firstConflict ) {
        newName = newName.left( newName.length() - 4 );
        firstConflict = false;
      }

      newName.sprintf( "%s <%d>", newName.latin1(), numTries );
      numTries++;
    }

    // Add the new one to the list
    mViewNameList.append( newName );

    // write the view to the config file,
    KConfig *config = mCore->config();
    config->deleteGroup( newName );
    KConfigGroupSaver saver( config, newName );
    config->writeEntry( "Type", type );

    // try to set the active view
    mActionSelectView->setItems( mViewNameList );
    mActionSelectView->setCurrentItem( mViewNameList.findIndex( newName ) );
    setActiveView( newName );

    editView();

    mActionDeleteView->setEnabled( mViewNameList.count() > 0 );
  }
}

void ViewManager::createViewFactories()
{
  KTrader::OfferList plugins = KTrader::self()->query( "KAddressBook/View" );
  KTrader::OfferList::ConstIterator it;
  for ( it = plugins.begin(); it != plugins.end(); ++it ) {
    if ( !(*it)->hasServiceType( "KAddressBook/View" ) )
      continue;

    KLibFactory *factory = KLibLoader::self()->factory( (*it)->library() );

    if ( !factory ) {
      kdDebug(5720) << "ViewManager::createViewFactories(): Factory creation failed" << endl;
      continue;
    }

    ViewFactory *viewFactory = static_cast<ViewFactory*>( factory );

    if ( !viewFactory ) {
      kdDebug(5720) << "ViewManager::createViewFactories(): Cast failed" << endl;
      continue;
    }

    mViewFactoryDict.insert( viewFactory->type(), viewFactory );
  }
}

void ViewManager::dropped( QDropEvent *e )
{
  kdDebug(5720) << "ViewManager::dropped: got a drop event" << endl;

  QString clipText, vcards;
  KURL::List urls;

  if ( KURLDrag::decode( e, urls) ) {
    KURL::List::Iterator it = urls.begin();
    int c = urls.count();
    if ( c > 1 ) {
      QString questionString = i18n( "Import one contact into your addressbook?", "Import %n contacts into your addressbook?", c );
      if ( KMessageBox::questionYesNo( this, questionString, i18n( "Import Contacts?" ) ) == KMessageBox::Yes ) {
        for ( ; it != urls.end(); ++it )
          emit urlDropped( *it, false );
      }
    } else if ( c == 1 )
      emit urlDropped( *it, true );
  } else if ( KVCardDrag::decode( e, vcards ) ) {
    KABC::Addressee addr;
    KABC::VCardConverter converter;
    QStringList list = QStringList::split( "\r\n\r\n", vcards );
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
      if ( converter.vCardToAddressee( (*it).stripWhiteSpace(), addr ) ) {
        KABC::Addressee a = mCore->addressBook()->findByUid( addr.uid() );
        if ( a.isEmpty() ) {
          mCore->addressBook()->insertAddressee( addr );
          emit modified();
        }
      }
    }

    mActiveView->refresh();
  }
}

void ViewManager::startDrag()
{
  kdDebug(5720) << "ViewManager::startDrag: starting to drag" << endl;

  // Get the list of all the selected addressees
  KABC::Addressee::List addrList;
  QStringList uidList = selectedUids();
  QStringList::Iterator iter;
  for ( iter = uidList.begin(); iter != uidList.end(); ++iter )
    addrList.append( mCore->addressBook()->findByUid( *iter ) );

  KMultipleDrag *drag = new KMultipleDrag( this );
  drag->addDragObject( new QTextDrag( AddresseeUtil::addresseesToClipboard(addrList), this ) );
  KABC::Addressee::List::Iterator it;
  QStringList vcards;
  for ( it = addrList.begin(); it != addrList.end(); ++it ) {
    QString vcard = QString::null;
    KABC::VCardConverter converter;
    if ( converter.addresseeToVCard( *it, vcard ) )
      vcards.append( vcard );
  }
  drag->addDragObject( new KVCardDrag( vcards.join( "\r\n" ), this ) );

  drag->setPixmap( KGlobal::iconLoader()->loadIcon( "vcard", KIcon::Desktop ) );
  drag->dragCopy();
}

void ViewManager::setActiveFilter( int index )
{
  Filter currentFilter;

  if ( ( index - 1 ) < 0 )
    currentFilter = Filter();
  else
    currentFilter = mFilterList[ index - 1 ];

  // Check if we have a view. Since the filter combo is created before
  // the view, this slot could be called before there is a valid view.
  if ( mActiveView ) {
    mActiveView->setFilter( currentFilter );
    mActiveView->refresh();
    emit selected( QString::null );
  }
}

void ViewManager::configureFilters()
{
  FilterDialog dlg( this );

  dlg.setFilters( mFilterList );

  if ( dlg.exec() )
    mFilterList = dlg.filters();

  uint pos = mActionSelectFilter->currentItem();
  mActionSelectFilter->setItems( filterNames() );
  mActionSelectFilter->setCurrentItem( pos );
  setActiveFilter( pos );
}

QStringList ViewManager::filterNames() const
{
  QStringList names( i18n( "None" ) );

  Filter::List::ConstIterator it;
  for ( it = mFilterList.begin(); it != mFilterList.end(); ++it )
    names.append( (*it).name() );

  return names;
}

int ViewManager::filterPosition( const QString &name ) const
{
  int pos = 0;

  Filter::List::ConstIterator it;
  for ( it = mFilterList.begin(); it != mFilterList.end(); ++it, ++pos )
    if ( name == (*it).name() )
      return pos + 1;

  return 0;
}

void ViewManager::initActions()
{
  mActionSelectView = new KSelectAction( i18n( "Select View" ), 0, mCore->actionCollection(), "select_view" );
#if KDE_VERSION >= 309
  mActionSelectView->setMenuAccelsEnabled( false );
#endif
  connect( mActionSelectView, SIGNAL( activated( const QString& ) ),
           SLOT( setActiveView( const QString& ) ) );

  new KAction( i18n( "Modify View..." ), "configure", 0, this,
               SLOT( editView() ), mCore->actionCollection(), "view_modify" );

  new KAction( i18n( "Add View..." ), "window_new", 0, this,
               SLOT( addView() ), mCore->actionCollection(), "view_add" );

  mActionDeleteView = new KAction( i18n( "Delete View" ), "view_remove", 0,
                                   this, SLOT( deleteView() ),
                                   mCore->actionCollection(), "view_delete" );

  new KAction( i18n( "Refresh View" ), "reload", 0, this,
               SLOT( refreshView() ), mCore->actionCollection(),
               "view_refresh" );

  new KAction( i18n( "Edit &Filters..." ), "filter", 0, this,
               SLOT( configureFilters() ), mCore->actionCollection(),
               "options_edit_filters" );

  mActionSelectFilter = new KSelectAction( i18n( "Select Filter" ), 0, mCore->actionCollection(), "select_filter" );
#if KDE_VERSION >= 309
  mActionSelectFilter->setMenuAccelsEnabled( false );
#endif
  connect( mActionSelectFilter, SIGNAL( activated( int ) ),
           SLOT( setActiveFilter( int ) ) );
}

void ViewManager::initGUI()
{
  QHBoxLayout *layout = new QHBoxLayout( this );
  mViewWidgetStack = new QWidgetStack( this );
  layout->addWidget( mViewWidgetStack );
}

#include "viewmanager.moc"
