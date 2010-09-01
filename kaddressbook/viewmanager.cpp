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

#include <tqfile.h>
#include <tqlayout.h>
#include <tqwidgetstack.h>

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
#include <ktempdir.h>
#include <ktrader.h>
#include <kurldrag.h>

#include "addviewdialog.h"
#include "addresseeutil.h"
#include "core.h"
#include "filtereditdialog.h"
#include "filterselectionwidget.h"
#include "kabprefs.h"

#include "viewmanager.h"

ViewManager::ViewManager( KAB::Core *core, TQWidget *parent, const char *name )
  : TQWidget( parent, name ), mCore( core ), mActiveView( 0 ),
    mFilterSelectionWidget( 0 )
{
  initGUI();
  initActions();

  mViewDict.setAutoDelete( true );

  createViewFactories();
}

ViewManager::~ViewManager()
{
  unloadViews();
  mViewFactoryDict.clear();
}

void ViewManager::restoreSettings()
{
  mViewNameList = KABPrefs::instance()->viewNames();
  TQString activeViewName = KABPrefs::instance()->currentView();

  mActionSelectView->setItems( mViewNameList );

  // Filter
  mFilterList = Filter::restore( mCore->config(), "Filter" );
  mFilterSelectionWidget->setItems( filterNames() );
  mFilterSelectionWidget->setCurrentItem( KABPrefs::instance()->currentFilter() );

  // Tell the views to reread their config, since they may have
  // been modified by global settings
  TQDictIterator<KAddressBookView> it( mViewDict );
  for ( it.toFirst(); it.current(); ++it ) {
    KConfigGroupSaver saver( mCore->config(), it.currentKey() );
    it.current()->readConfig( mCore->config() );
  }

  setActiveView( activeViewName );

  mActionDeleteView->setEnabled( mViewNameList.count() > 1 );
}

void ViewManager::saveSettings()
{
  TQDictIterator<KAddressBookView> it( mViewDict );
  for ( it.toFirst(); it.current(); ++it ) {
    KConfigGroupSaver saver( mCore->config(), it.currentKey() );
    (*it)->writeConfig( mCore->config() );
  }

  Filter::save( mCore->config(), "Filter", mFilterList );
  KABPrefs::instance()->setCurrentFilter( mFilterSelectionWidget->currentItem() );

  // write the view name list
  KABPrefs::instance()->setViewNames( mViewNameList );

  if ( mActiveView )
    KABPrefs::instance()->setCurrentView( mActiveView->caption() );
}

TQStringList ViewManager::selectedUids() const
{
  if ( mActiveView ) {
    return mActiveView->selectedUids();
  } else
    return TQStringList();
}

TQStringList ViewManager::selectedEmails() const
{
  if ( mActiveView )
    return mActiveView->selectedEmails();
  else
    return TQStringList();
}

KABC::Addressee::List ViewManager::selectedAddressees() const
{
  KABC::Addressee::List list;

  const TQStringList uids = selectedUids();
  TQStringList::ConstIterator it;
  for ( it = uids.begin(); it != uids.end(); ++it ) {
    KABC::Addressee addr = mCore->addressBook()->findByUid( *it );
    if ( !addr.isEmpty() )
      list.append( addr );
  }

  return list;
}

void ViewManager::setFilterSelectionWidget( FilterSelectionWidget *wdg )
{
  mFilterSelectionWidget = wdg;
}

KABC::Field *ViewManager::currentSortField() const
{
  if ( mActiveView )
    return mActiveView->sortField();
  else
    return 0;
}

KABC::Field::List ViewManager::viewFields() const
{
/*
  if ( mActiveView )
    return mActiveView->fields();
  else
*/
    return KABC::Field::List();
}

void ViewManager::setSelected( const TQString &uid, bool selected )
{
  if ( mActiveView )
    mActiveView->setSelected( uid, selected );
}

void ViewManager::setFirstSelected( bool selected )
{
  if ( mActiveView )
    mActiveView->setFirstSelected( selected );
}

void ViewManager::unloadViews()
{
  mViewDict.clear();
  mActiveView = 0;
}

void ViewManager::setActiveView( const TQString &name )
{
  KAddressBookView *view = 0;

  // Check that this isn't the same as the current active view
  if ( mActiveView && ( mActiveView->caption() == name ) )
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
    TQString type = config->readEntry( "Type", "Table" );

    kdDebug(5720) << "ViewManager::setActiveView: creating view - " << name << endl;

    ViewFactory *factory = mViewFactoryDict.find( type );
    if ( factory )
      view = factory->view( mCore, mViewWidgetStack );

    if ( view ) {
      view->setCaption( name );
      mViewDict.insert( name, view );
      mViewWidgetStack->addWidget( view );
      view->readConfig( config );

      // The manager just relays the signals
      connect( view, TQT_SIGNAL( selected( const TQString& ) ),
               TQT_SIGNAL( selected( const TQString & ) ) );
      connect( view, TQT_SIGNAL( executed( const TQString& ) ),
               TQT_SIGNAL( executed( const TQString& ) ) );
      connect( view, TQT_SIGNAL( modified() ), TQT_SIGNAL( modified() ) );
      connect( view, TQT_SIGNAL( dropped( TQDropEvent* ) ),
               TQT_SLOT( dropped( TQDropEvent* ) ) );
      connect( view, TQT_SIGNAL( startDrag() ), TQT_SLOT( startDrag() ) );
      connect( view, TQT_SIGNAL( sortFieldChanged() ), TQT_SIGNAL( sortFieldChanged() ) );
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
      mFilterSelectionWidget->setCurrentItem( 0 );
      setActiveFilter( 0 );
    } else if ( view->defaultFilterType() == KAddressBookView::Active ) {
      setActiveFilter( mFilterSelectionWidget->currentItem() );
    } else {
      uint pos = filterPosition( view->defaultFilterName() );
      mFilterSelectionWidget->setCurrentItem( pos );
      setActiveFilter( pos );
    }

    // Update the inc search widget to show the fields in the new active
    // view.
    mActiveView->refresh();

  } else
    kdDebug(5720) << "ViewManager::setActiveView: unable to find view\n";
}

void ViewManager::refreshView( const TQString &uid )
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
    ViewConfigureDialog dlg( wdg, mActiveView->caption(), this );

    KConfigGroupSaver saver( mCore->config(), mActiveView->caption() );
    dlg.restoreSettings( mCore->config() );

    if ( dlg.exec() ) {
      dlg.saveSettings( mCore->config() );
      mActiveView->readConfig( mCore->config() );
      // Set the proper filter in the view. By setting the combo
      // box, the activated slot will be called, which will push
      // the filter to the view and refresh it.
      if ( mActiveView->defaultFilterType() == KAddressBookView::None ) {
        mFilterSelectionWidget->setCurrentItem( 0 );
        setActiveFilter( 0 );
      } else if ( mActiveView->defaultFilterType() == KAddressBookView::Active ) {
        setActiveFilter( mFilterSelectionWidget->currentItem() );
      } else {
        uint pos = filterPosition( mActiveView->defaultFilterName() );
        mFilterSelectionWidget->setCurrentItem( pos );
        setActiveFilter( pos );
      }

      mActiveView->refresh();
      emit viewFieldsChanged();
    }
  }
}

void ViewManager::deleteView()
{
  TQString text = i18n( "<qt>Are you sure that you want to delete the view <b>%1</b>?</qt>" )
                     .arg( mActiveView->caption() );
  TQString caption = i18n( "Confirm Delete" );

  if ( KMessageBox::warningContinueCancel( this, text, caption, KGuiItem( i18n("&Delete"), "editdelete") ) == KMessageBox::Continue ) {
    mViewNameList.remove( mActiveView->caption() );

    // remove the view from the config file
    KConfig *config = mCore->config();
    config->deleteGroup( mActiveView->caption() );

    mViewDict.remove( mActiveView->caption() );
    mActiveView = 0;

    // we are in an invalid state now, but that should be fixed after
    // we emit the signal
    mActionSelectView->setItems( mViewNameList );
    if ( mViewNameList.count() > 0 ) {
      mActionSelectView->setCurrentItem( 0 );
      setActiveView( mViewNameList[ 0 ] );
    }
    mActionDeleteView->setEnabled( mViewNameList.count() > 1 );
  }
}

void ViewManager::addView()
{
  AddViewDialog dialog( &mViewFactoryDict, this );

  if ( dialog.exec() ) {
    TQString newName = dialog.viewName();
    TQString type = dialog.viewType();

    // Check for name conflicts
    bool firstConflict = true;
    int numTries = 1;
    while ( mViewNameList.contains( newName ) > 0 ) {
      if ( !firstConflict ) {
        newName = newName.left( newName.length() - 4 );
        firstConflict = false;
      }

      newName = TQString( "%1 <%2>" ).arg( newName ).arg( numTries );
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

    mActionDeleteView->setEnabled( mViewNameList.count() > 1 );
  }
}

void ViewManager::scrollUp()
{
  if ( mActiveView )
    mActiveView->scrollUp();
}

void ViewManager::scrollDown()
{
  if ( mActiveView )
    mActiveView->scrollDown();
}

void ViewManager::createViewFactories()
{
  const KTrader::OfferList plugins = KTrader::self()->query( "KAddressBook/View",
    TQString( "[X-KDE-KAddressBook-ViewPluginVersion] == %1" ).arg(  KAB_VIEW_PLUGIN_VERSION ) );
  KTrader::OfferList::ConstIterator it;
  for ( it = plugins.begin(); it != plugins.end(); ++it ) {
    if ( !(*it)->hasServiceType( "KAddressBook/View" ) )
      continue;

    KLibFactory *factory = KLibLoader::self()->factory( (*it)->library().latin1() );

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

void ViewManager::dropped( TQDropEvent *e )
{
  kdDebug(5720) << "ViewManager::dropped: got a drop event" << endl;

  // don't allow drops from our own drags
  if ( e->source() == this )
    return;

  KABC::Addressee::List list;
  KURL::List urls;

  if ( KURLDrag::decode( e, urls) ) {
    KURL::List::ConstIterator it = urls.begin();
    int c = urls.count();
    if ( c > 1 ) {
      TQString questionString = i18n( "Import one contact into your addressbook?", "Import %n contacts into your addressbook?", c );
      if ( KMessageBox::questionYesNo( this, questionString, i18n( "Import Contacts?" ), i18n("Import"), i18n("Do Not Import") ) == KMessageBox::Yes ) {
        for ( ; it != urls.end(); ++it )
          emit urlDropped( *it );
      }
    } else if ( c == 1 )
      emit urlDropped( *it );
  } else if ( KVCardDrag::decode( e, list ) ) {
    KABC::Addressee::List::ConstIterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
      KABC::Addressee a = mCore->addressBook()->findByUid( (*it).uid() );
      if ( a.isEmpty() ) { // not yet in address book
        mCore->addressBook()->insertAddressee( *it );
        emit modified();
      }
    }

    mActiveView->refresh();
  }
}

void ViewManager::startDrag()
{
  // Get the list of all the selected addressees
  KABC::Addressee::List addrList;
  const TQStringList uidList = selectedUids();
  if (  uidList.isEmpty() )
    return;

  kdDebug(5720) << "ViewManager::startDrag: starting to drag" << endl;

  TQStringList::ConstIterator it;
  for ( it = uidList.begin(); it != uidList.end(); ++it )
    addrList.append( mCore->addressBook()->findByUid( *it ) );

  KMultipleDrag *drag = new KMultipleDrag( this );

  KABC::VCardConverter converter;
#if defined(KABC_VCARD_ENCODING_FIX)
  TQCString vcards = converter.createVCardsRaw( addrList );
#else
  TQString vcards = converter.createVCards( addrList );
#endif

  // Best text representation is given by textdrag, so it must be first
  drag->addDragObject( new TQTextDrag( AddresseeUtil::addresseesToEmails( addrList ), this ) );
  drag->addDragObject( new KVCardDrag( vcards, this ) );

  KTempDir tempDir;
  // can't set tempDir to autoDelete, in case of dropping on the desktop, the copy is async...
  if ( tempDir.status() == 0 ) {
    TQString fileName;
    if ( addrList.count() == 1 )
      fileName = addrList[ 0 ].givenName() + "_" + addrList[ 0 ].familyName() + ".vcf";
    else
      fileName = "contacts.vcf";

    TQFile tempFile( tempDir.name() + "/" + fileName );
    if ( tempFile.open( IO_WriteOnly ) ) {
#if defined(KABC_VCARD_ENCODING_FIX)
      tempFile.writeBlock( vcards, vcards.length() );
#else
      tempFile.writeBlock( vcards.utf8() );
#endif
      tempFile.close();

      KURLDrag *urlDrag = new KURLDrag( KURL( tempFile.name() ), this );
      drag->addDragObject( urlDrag );
    }
  }

  drag->setPixmap( KGlobal::iconLoader()->loadIcon( "vcard", KIcon::Desktop ) );
  drag->dragCopy();
}

void ViewManager::setActiveFilter( int index )
{
  Filter currentFilter;

  if ( ( index - 1 ) < 0 )
    currentFilter = Filter();
  else if ( ( index - 1 ) < 1 ) {
    currentFilter = Filter();
    currentFilter.setMatchRule(Filter::NotMatching);
  }
  else
    currentFilter = mFilterList[ index - 2 ];

  // Check if we have a view. Since the filter combo is created before
  // the view, this slot could be called before there is a valid view.
  if ( mActiveView ) {
    mActiveView->setFilter( currentFilter );
    mActiveView->refresh();
    emit selected( TQString::null );
  }
}

void ViewManager::configureFilters()
{
  FilterDialog dlg( this );

  dlg.setFilters( mFilterList );

  if ( dlg.exec() )
    mFilterList = dlg.filters();

  uint pos = mFilterSelectionWidget->currentItem();
  mFilterSelectionWidget->setItems( filterNames() );
  mFilterSelectionWidget->setCurrentItem( pos );
  setActiveFilter( pos );
}

TQStringList ViewManager::filterNames() const
{
  TQStringList names( i18n( "None" ) );
  names.append( i18n( "Unfiled" ) );

  Filter::List::ConstIterator it;
  for ( it = mFilterList.begin(); it != mFilterList.end(); ++it )
    names.append( (*it).name() );

  return names;
}

int ViewManager::filterPosition( const TQString &name ) const
{
  int pos = 0;

  Filter::List::ConstIterator it;
  for ( it = mFilterList.begin(); it != mFilterList.end(); ++it, ++pos )
    if ( name == (*it).name() )
      return pos + 2;

  return 0;
}

void ViewManager::initActions()
{
  mActionSelectView = new KSelectAction( i18n( "Select View" ), 0, mCore->actionCollection(), "select_view" );
#if KDE_VERSION >= 309
  mActionSelectView->setMenuAccelsEnabled( false );
#endif
  connect( mActionSelectView, TQT_SIGNAL( activated( const TQString& ) ),
           TQT_SLOT( setActiveView( const TQString& ) ) );

  KAction *action;

  action = new KAction( i18n( "Modify View..." ), "configure", 0, this,
                        TQT_SLOT( editView() ), mCore->actionCollection(),
                        "view_modify" );
  action->setWhatsThis( i18n( "By pressing this button a dialog opens that allows you to modify the view of the addressbook. There you can add or remove fields that you want to be shown or hidden in the addressbook like the name for example." ) );

  action = new KAction( i18n( "Add View..." ), "window_new", 0, this,
                        TQT_SLOT( addView() ), mCore->actionCollection(),
                        "view_add" );
  action->setWhatsThis( i18n( "You can add a new view by choosing one from the dialog that appears after pressing the button. You have to give the view a name, so that you can distinguish between the different views." ) );

  mActionDeleteView = new KAction( i18n( "Delete View" ), "view_remove", 0,
                                   this, TQT_SLOT( deleteView() ),
                                   mCore->actionCollection(), "view_delete" );
  mActionDeleteView->setWhatsThis( i18n( "By pressing this button you can delete the actual view, which you have added before." ) );

  action = new KAction( i18n( "Refresh View" ), "reload", 0, this,
               TQT_SLOT( refreshView() ), mCore->actionCollection(),
               "view_refresh" );
  action->setWhatsThis( i18n( "The view will be refreshed by pressing this button." ) );

  action = new KAction( i18n( "Edit &Filters..." ), "filter", 0, this,
               TQT_SLOT( configureFilters() ), mCore->actionCollection(),
               "options_edit_filters" );
  action->setWhatsThis( i18n( "Edit the contact filters<p>You will be presented with a dialog, where you can add, remove and edit filters." ) );
}

void ViewManager::initGUI()
{
  TQHBoxLayout *layout = new TQHBoxLayout( this );
  mViewWidgetStack = new TQWidgetStack( this );
  layout->addWidget( mViewWidgetStack );
}

#include "viewmanager.moc"
