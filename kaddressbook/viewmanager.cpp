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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qapplication.h>
#include <qclipboard.h>
#include <qcombobox.h>
#include <qdragobject.h>
#include <qevent.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qwidgetstack.h>

#include <kabc/addressbook.h>
#include <kabc/field.h>
#include <kabc/vcardconverter.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmultipledrag.h>
#include <libkdepim/kvcarddrag.h>

#include "undo.h"
#include "undocmds.h"
#include "viewmanager.h"
#include "configureviewdialog.h"
#include "viewwrapper.h"
#include "iconviewwrapper.h"
#include "tableviewwrapper.h"
#include "detailsviewcontainer.h"
#include "cardviewwrapper.h"
#include "addviewdialog.h"
#include "jumpbuttonbar.h"
#include "addresseeutil.h"
#include "addresseeeditorwidget.h"
#include "filterselectionwidget.h"
#include "extensionwidget.h"

ViewManager::ViewManager( KABC::AddressBook *ab, KConfig *config,
                          QWidget *parent, const char *name )
  : QWidget( parent, name ), mAddressBook( ab ), mConfig( config )
{
  initGUI();

  mViewDict.setAutoDelete( true );
  mViewWrapperDict.setAutoDelete( true );

  createViewWrappers();

  mActiveView = 0;
}

ViewManager::~ViewManager()
{
  unloadViews();
  mViewWrapperDict.clear();
}

void ViewManager::readConfig()
{
  // Read the view names
  mConfig->setGroup( "Views" );
  mViewNameList = mConfig->readListEntry( "Names" );

  if ( mViewNameList.size() == 0 )  // Add a default
    mViewNameList.append( i18n("Default Table View") );

  mFilterList = Filter::restore( mConfig, "Filter" );
  filtersChanged( mFilterList );
  mConfig->setGroup( "Filter" );
  if ( mConfig->hasKey( "Active" ) )
    emit setCurrentFilterName( mConfig->readEntry( "Active" ) );

  // Tell the views to reread their config, since they may have
  // been modified by global settings
  QDictIterator<KAddressBookView> it( mViewDict );
  for ( it.toFirst(); it.current(); ++it ) {
    mConfig->setGroup( it.currentKey() );
    it.current()->readConfig( mConfig );
  }

  QValueList<int> splitterSize;
  mConfig->setGroup( "Splitter" );
  splitterSize = mConfig->readIntListEntry( "ExtensionsSplitter" );
  if ( splitterSize.count() == 0 ) {
    splitterSize.append( width() / 2 );
    splitterSize.append( width() / 2 );
  }
  mExtensionBarSplitter->setSizes( splitterSize );

  splitterSize = mConfig->readIntListEntry( "DetailsSplitter" );
  if ( splitterSize.count() == 0 ) {
    splitterSize.append( height() / 2 );
    splitterSize.append( height() / 2 );
  }
  mDetailsSplitter->setSizes( splitterSize );
}

void ViewManager::writeConfig()
{
  QDictIterator<KAddressBookView> it( mViewDict );
  for ( it.toFirst(); it.current(); ++it ) {
    mConfig->setGroup( it.currentKey() );
    (*it)->writeConfig( mConfig );
  }

  Filter::save( mConfig, QString( "Filter" ), mFilterList );
  mConfig->setGroup( "Filter" );
  mConfig->writeEntry( "Active", mCurrentFilter.name() );

  // write the view name list
  mConfig->setGroup( "Views" );
  mConfig->writeEntry( "Names", mViewNameList );

  mConfig->setGroup( "Splitter" );
  mConfig->writeEntry( "ExtensionsSplitter", mExtensionBarSplitter->sizes() );
  mConfig->writeEntry( "DetailsSplitter", mDetailsSplitter->sizes() );
}

QStringList ViewManager::selectedUids() const
{
  return mActiveView->selectedUids();
}

void ViewManager::sendMail()
{
  QString emailAddrs = mActiveView->selectedEmails();
  kapp->invokeMailer( emailAddrs, "" );
}

void ViewManager::sendMail( const QString& email )
{
  kapp->invokeMailer( email, "" );
}

void ViewManager::browse( const QString& url )
{
  kapp->invokeBrowser( url );
}

void ViewManager::deleteAddressees()
{
  QStringList uidList = mActiveView->selectedUids();

  if ( uidList.size() > 0 ) {
    PwDeleteCommand *command = new PwDeleteCommand( mAddressBook, uidList );
    UndoStack::instance()->push( command );
    RedoStack::instance()->clear();

    // now if we deleted anything, refresh
    mActiveView->refresh();
    emit selected( QString::null );
    addresseeSelected( QString::null );

    emit modified();
  }
}

void ViewManager::paste()
{
  QClipboard *cb = QApplication::clipboard();
  PwPasteCommand *command = new PwPasteCommand( mAddressBook, cb->text() );
  UndoStack::instance()->push( command );
  RedoStack::instance()->clear();

  modified();
  mActiveView->refresh();
}

void ViewManager::copy()
{
  QStringList uidList = mActiveView->selectedUids();
  KABC::Addressee::List addrList;

  QStringList::Iterator it;
  for ( it = uidList.begin(); it != uidList.end(); ++it )
    addrList.append( mAddressBook->findByUid( *it ) );

  QString clipText = AddresseeUtil::addresseesToClipboard( addrList );

  kdDebug(5720) << "ViewManager::copy: " << clipText << endl;

  QClipboard *cb = QApplication::clipboard();
  cb->setText( clipText );
}

void ViewManager::cut()
{
  QStringList uidList = mActiveView->selectedUids();

  if ( uidList.size() > 0 ) {
    PwCutCommand *command = new PwCutCommand( mAddressBook, uidList );
    UndoStack::instance()->push( command );
    RedoStack::instance()->clear();

    mActiveView->refresh();
    emit modified();
  }
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
    KConfig *config = kapp->config();
    config->setGroup( name );
    QString type = config->readEntry( "Type", "Table" );

    kdDebug(5720) << "ViewManager::setActiveView: creating view - " << name << endl;

    ViewWrapper *wrapper = mViewWrapperDict.find( type );
    if ( wrapper )
      view = wrapper->createView( mAddressBook, mViewWidgetStack, name.latin1() );

    if ( view ) {
      mViewDict.insert( name, view );
      mViewWidgetStack->addWidget( view );
      view->readConfig( config );

      // The manager just relays the signals
      connect( view, SIGNAL( selected( const QString& ) ),
               SIGNAL( selected( const QString & ) ) );
      connect( view, SIGNAL( selected( const QString& ) ),
               SLOT( addresseeSelected( const QString& ) ) );
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
    if ( view->defaultFilterType() == KAddressBookView::None )
      emit setCurrentFilter( 0 );
    else if ( view->defaultFilterType() == KAddressBookView::Active )
      emit setCurrentFilterName( mCurrentFilter.name() );
    else {
      QString filterName = view->defaultFilterName();
      emit setCurrentFilterName( filterName );
    }

    // Update the inc search combo to show the fields in the new active
    // view.
    refreshIncrementalSearchCombo();

    mActiveView->refresh( QString::null );
  } else
    kdDebug(5720) << "ViewManager::setActiveView: unable to find view\n";
}

void ViewManager::refresh( const QString &uid )
{
  if ( mActiveView ) {
    mActiveView->refresh( uid );
    addresseeSelected( uid );
  }
}

void ViewManager::modifyView()
{
  if ( !mActiveView )
    return;

  ConfigureViewDialog *dialog = 0;

  ViewWrapper *wrapper = mViewWrapperDict.find( mActiveView->type() );

  if ( wrapper ) {
    // Save the filters so the dialog has the latest set
    Filter::save( mConfig, "Filter", mFilterList );

    dialog = wrapper->createConfigureViewDialog( mActiveView->name(),
                                                 mAddressBook, this );
  }

  if ( dialog ) {
    mConfig->setGroup( mActiveView->name() );
    dialog->readConfig( mConfig );
    if ( dialog->exec() ) {
      dialog->writeConfig( mConfig );
      mActiveView->readConfig( mConfig );

      // Set the proper filter in the view. By setting the combo
      // box, the activated slot will be called, which will push
      // the filter to the view and refresh it.
      if ( mActiveView->defaultFilterType() == KAddressBookView::None )
        emit setCurrentFilter( 0 );
      else if ( mActiveView->defaultFilterType() == KAddressBookView::Active )
        emit setCurrentFilterName( mCurrentFilter.name() );
      else {
        QString filterName = mActiveView->defaultFilterName();
        emit setCurrentFilterName( filterName );
      }

      refreshIncrementalSearchCombo();

      mActiveView->refresh();

      delete dialog;
    }
  }
}

void ViewManager::deleteView()
{
  QString text = i18n( "Are you sure that you want to delete the view \"%1\"?" ).arg( mActiveView->name() );
  QString caption = i18n( "Confirm Delete" );

  if ( KMessageBox::questionYesNo( this, text, caption ) == KMessageBox::Yes ) {
    mViewNameList.remove( mActiveView->name() );

    // remove the view from the config file
    KConfig *config = kapp->config();
    config->deleteGroup( mActiveView->name() );

    mViewDict.remove( mActiveView->name() );
    mActiveView = 0;

    // we are in an invalid state now, but that should be fixed after
    // we emit the signal
    emit viewConfigChanged( QString::null );
  }
}

void ViewManager::addView()
{
  AddViewDialog dialog( &mViewWrapperDict, this );

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
    KConfig *config = kapp->config();
    config->deleteGroup( newName );
    config->setGroup( newName );
    config->writeEntry( "Type", type );

    // try to set the active view
    emit viewConfigChanged( newName );

    modifyView();
  }
}

void ViewManager::createViewWrappers()
{
  // View Developers: Add an entry here to create the wrapper for your view
  // type and add it to the list. Thats it :D

  ViewWrapper *wrapper = new IconViewWrapper();
  mViewWrapperDict.insert( wrapper->type(), wrapper );

  wrapper = new TableViewWrapper();
  mViewWrapperDict.insert( wrapper->type(), wrapper );

  wrapper = new CardViewWrapper();
  mViewWrapperDict.insert( wrapper->type(), wrapper );
}

void ViewManager::initGUI()
{
  mCurrentExtensionWidget = 0;

  QHBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing( KDialogBase::spacingHint() );

  mExtensionBarSplitter = new QSplitter( this );
  mExtensionBarSplitter->setOrientation( Qt::Vertical );

  mDetailsSplitter = new QSplitter( mExtensionBarSplitter );

  mViewWidgetStack = new QWidgetStack( mDetailsSplitter );

  mDetails = new ViewContainer( mDetailsSplitter );
  connect( mDetails, SIGNAL( sendEmail( const QString& ) ),
           SLOT( sendMail( const QString& ) ) );
  connect( mDetails, SIGNAL( browse( const QString& ) ),
           SLOT( browse( const QString& ) ) );

  mJumpButtonBar = new JumpButtonBar( this );
  connect( mJumpButtonBar, SIGNAL( jumpToLetter( const QChar& ) ),
           SLOT( jumpToLetter( const QChar& ) ) );

  // Setup the feature bar widget.
  mExtensionBar = new QHBox( mExtensionBarSplitter );

  /**
    Add all extension bar widgets.
   */
  ExtensionWidget *wdg = new AddresseeEditorWidget( this, mExtensionBar );
  wdg->hide();
  connect( wdg, SIGNAL( modified( KABC::Addressee::List ) ),
           SLOT( extensionWidgetModified( KABC::Addressee::List ) ) );
  mExtensionWidgetList.append( wdg );

  KTrader::OfferList plugins = availablePlugins( "KAddressBook/Extension" );
  KTrader::OfferList::ConstIterator it;
  for ( it = plugins.begin(); it != plugins.end(); ++it ) {
    kdDebug() << "Part: " << (*it)->desktopEntryName() << " ("
              << (*it)->name() << ")" << endl;
    wdg = loadExtension( *it, mExtensionBar );
    if ( wdg ) {
      wdg->hide();
      connect( wdg, SIGNAL( modified( KABC::Addressee::List ) ),
               SLOT( extensionWidgetModified( KABC::Addressee::List ) ) );
      mExtensionWidgetList.append( wdg );
    }
  }

  topLayout->addWidget( mExtensionBarSplitter );
  topLayout->setStretchFactor( mExtensionBarSplitter, 100 );
  topLayout->addWidget( mJumpButtonBar );
  topLayout->setStretchFactor( mJumpButtonBar, 1 );
}

void ViewManager::refreshIncrementalSearchCombo()
{
  QStringList items;

  KABC::Field::List fields = mActiveView->fields();

  mIncrementalSearchFields.clear();

  KABC::Field::List::Iterator it;
  for ( it = fields.begin(); it != fields.end(); ++it ) {
    items.append( (*it)->label() );
    mIncrementalSearchFields.append( *it );
  }

  mCurrentIncSearchField = mIncrementalSearchFields.first(); // we assume there are always columns?
  emit setIncSearchFields( items );
}

void ViewManager::incSearch( const QString& text, int field )
{
  mCurrentIncSearchField = mIncrementalSearchFields[ field ];

  mActiveView->incrementalSearch( text, mCurrentIncSearchField );
}

void ViewManager::jumpToLetter( const QChar &ch )
{
  mActiveView->incrementalSearch( QString(ch), mCurrentIncSearchField );
}

void ViewManager::setJumpButtonBarVisible( bool visible )
{
  if ( visible )
    mJumpButtonBar->show();
  else
    mJumpButtonBar->hide();
}

void ViewManager::setDetailsVisible( bool visible )
{
  if ( visible )
    mDetails->show();
  else
    mDetails->hide();
}

bool ViewManager::isQuickEditVisible()
{
  return ( mCurrentExtensionWidget &&
      mCurrentExtensionWidget->identifier() == "contact_editor" );
}

void ViewManager::dropped( QDropEvent *e )
{
  kdDebug(5720) << "ViewManager::dropped: got a drop event" << endl;

  QString clipText, vcards;
  QStrList urls;

  if ( QUriDrag::decode( e, urls) ) {
    QPtrListIterator<char> it( urls );
    int c = urls.count();
    if ( c > 1 ) {
      QString questionString = i18n( "Import one contact into your addressbook?", "Import %n contacts into your addressbook?", c );
      if ( KMessageBox::questionYesNo( this, questionString, i18n( "Import Contacts?" ) ) == KMessageBox::Yes ) {
        for ( ; it.current(); ++it) {
          KURL url(*it);
          emit importVCard( url.path(), false );
        }
      }
    } else if ( c == 1 ) {
      KURL url(*it);
      emit importVCard( url.path(), true );
    }
  } else if ( KVCardDrag::decode( e, vcards ) ) {
    KABC::Addressee addr;
    KABC::VCardConverter converter;
    QStringList list = QStringList::split( "\r\n\r\n", vcards );
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
      if ( converter.vCardToAddressee( (*it).stripWhiteSpace(), addr ) ) {
        KABC::Addressee a = mAddressBook->findByUid( addr.uid() );
        if ( a.isEmpty() ) {
          mAddressBook->insertAddressee( addr );
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
    addrList.append( mAddressBook->findByUid( *iter ) );

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

void ViewManager::addresseeSelected( const QString &uid )
{
  KABC::Addressee addr = mAddressBook->findByUid( uid );
  mDetails->setAddressee( addr );

  if ( mCurrentExtensionWidget )
    mCurrentExtensionWidget->addresseeSelectionChanged();
}

void ViewManager::extensionWidgetModified( KABC::Addressee::List list )
{
  if ( mCurrentExtensionWidget &&
                   mCurrentExtensionWidget->identifier() == "contact_editor" ) {
    AddresseeEditorWidget *wdg =
               static_cast<AddresseeEditorWidget*>( mCurrentExtensionWidget );
    if ( wdg ) {
      wdg->save();
      mAddressBook->insertAddressee( wdg->addressee() );
    }
  }

  if ( list.count() == 0 )
    mActiveView->refresh();
  else
    mActiveView->refresh( list[0].uid() );

  emit modified();
}

void ViewManager::filtersChanged( const Filter::List &list )
{
  mFilterList = list;

  QStringList names;
  Filter::List::Iterator it;
  for ( it = mFilterList.begin(); it != mFilterList.end(); ++it )
    names.append( (*it).name() );

  // update the combo
  emit setFilterNames( names );

  mCurrentFilter = Filter();
}

void ViewManager::filterActivated( int index )
{
  if ( index < 0 )
    mCurrentFilter = Filter();
  else
    mCurrentFilter = mFilterList[ index ];

  // Check if we have a view. Since the filter combo is created before
  // the view, this slot could be called before there is a valid view.
  if ( mActiveView ) {
    mActiveView->setFilter( mCurrentFilter );
    mActiveView->refresh();
  }
}

void ViewManager::slotModified()
{
  modified();
}

void ViewManager::showExtensionWidget( int id )
{
  if ( id == 0 ) {
    mExtensionBar->hide();
    mCurrentExtensionWidget = 0;
  } else {
    if ( mCurrentExtensionWidget )
      mCurrentExtensionWidget->hide();

    mCurrentExtensionWidget = mExtensionWidgetList.at( id - 1 );

    mExtensionBar->show();
    mCurrentExtensionWidget->show();
  }
}

QStringList ViewManager::extensionWidgetList()
{
  QStringList list;
  list.append( i18n( "None" ) );

  ExtensionWidget *wdg = 0;
  for ( wdg = mExtensionWidgetList.first(); wdg; wdg = mExtensionWidgetList.next() )
    list.append( wdg->title() );

  return list;
}

KABC::AddressBook *ViewManager::addressBook()
{
  return mAddressBook;
}

const QStringList &ViewManager::viewNames()
{
  return mViewNameList;
}

const Filter::List &ViewManager::filters() const
{
  return mFilterList;
}

KTrader::OfferList ViewManager::availablePlugins( const QString &type )
{
  return KTrader::self()->query( type );
}

ExtensionWidget *ViewManager::loadExtension( KService::Ptr service, QWidget *parent )
{
  if ( !service->hasServiceType( "KAddressBook/Extension" ) )
    return 0;

  KLibFactory *factory = KLibLoader::self()->factory( service->library() );

  if ( !factory ) {
    kdDebug(5720) << "ViewManager::loadExtension(): Factory creation failed" << endl;
    return 0;
  }
  
  ExtensionFactory *extensionFactory = static_cast<ExtensionFactory*>( factory );
  
  if ( !extensionFactory ) {
    kdDebug(5720) << "ViewManager::loadExtension(): Cast failed" << endl;
    return 0;
  }
  
  return extensionFactory->create( this, parent );
}

ExtensionWidget *ViewManager::loadExtension( const QString &name, QWidget *parent )
{
  KTrader::OfferList list = availablePlugins( "KAddressBook/Extension" );
  KTrader::OfferList::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    if ( (*it)->desktopEntryName() == name )
      return loadExtension( *it, parent );
  }

  return 0;
}

KAddressBookView *ViewManager::loadView( KService::Ptr service, QWidget *parent )
{
  if ( !service->hasServiceType( "KAddressBook/View" ) )
    return 0;

  KLibFactory *factory = KLibLoader::self()->factory( service->library() );

  if ( !factory ) {
    kdDebug(5720) << "ViewManager::loadView(): Factory creation failed" << endl;
    return 0;
  }
  
  ViewFactory *viewFactory = static_cast<ViewFactory*>( factory );
  
  if ( !viewFactory ) {
    kdDebug(5720) << "ViewManager::loadView(): Cast failed" << endl;
    return 0;
  }
  
  return viewFactory->create( this, parent );
}

KAddressBookView *ViewManager::loadView( const QString &name, QWidget *parent )
{
  KTrader::OfferList list = availablePlugins( "KAddressBook/View" );
  KTrader::OfferList::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    if ( (*it)->desktopEntryName() == name )
      return loadView( *it, parent );
  }

  return 0;
}

#include "viewmanager.moc"
