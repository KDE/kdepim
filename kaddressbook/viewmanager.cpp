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
#include <qvbox.h>
#include <qwidgetstack.h>

#include <kabc/addressbook.h>
#include <kabc/addresseelist.h>
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
#include <kstandarddirs.h>
#include <kurldrag.h>
#include <errno.h>
#include <ktempfile.h>
#include <qdir.h>

#include "addresseeeditorwidget.h"
#include "addresseeutil.h"
#include "addviewdialog.h"
#include "detailsviewcontainer.h"
#include "extensionwidget.h"
#include "filterselectionwidget.h"
#include "incsearchwidget.h"
#include "jumpbuttonbar.h"
#include "undo.h"
#include "undocmds.h"
#include "viewconfigurewidget.h"

#include "viewmanager.h"

ViewManager::ViewManager( KABC::AddressBook *ab, KConfig *config,
                          QWidget *parent, const char *name )
  : QWidget( parent, name ), mAddressBook( ab ), mConfig( config )
{
  initGUI();

  mViewDict.setAutoDelete( true );
//  mViewFactoryDict.setAutoDelete( true );

  createViewFactories();

  mActiveView = 0;
}

ViewManager::~ViewManager()
{
  unloadViews();
  mViewFactoryDict.clear();
}

void ViewManager::readConfig()
{
  mConfig->reparseConfiguration();

  KConfigGroupSaver viewSaver( mConfig, "Views" );
  mViewNameList = mConfig->readListEntry( "Names" );

  if ( mViewNameList.size() == 0 )  // Add a default
    mViewNameList.append( i18n("Default Table View") );

  mFilterList = Filter::restore( mConfig, "Filter" );
  mCurrentFilter = Filter();

  emit filtersEdited();

  KConfigGroupSaver filterSaver( mConfig, "Filter" );
  if ( mConfig->hasKey( "Active" ) )
    emit currentFilterChanged( mConfig->readEntry( "Active" ) );

  // Tell the views to reread their config, since they may have
  // been modified by global settings
  QDictIterator<KAddressBookView> it( mViewDict );
  for ( it.toFirst(); it.current(); ++it ) {
    KConfigGroupSaver saver( mConfig, it.currentKey() );
    it.current()->readConfig( mConfig );
  }

  QValueList<int> splitterSize;
  KConfigGroupSaver splitterSaver( mConfig, "Splitter" );
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

  loadExtensions();
}

void ViewManager::writeConfig()
{
  QDictIterator<KAddressBookView> it( mViewDict );
  for ( it.toFirst(); it.current(); ++it ) {
    KConfigGroupSaver saver( mConfig, it.currentKey() );
    (*it)->writeConfig( mConfig );
  }

  Filter::save( mConfig, QString( "Filter" ), mFilterList );
  KConfigGroupSaver filterSaver( mConfig, "Filter" );
  mConfig->writeEntry( "Active", mCurrentFilter.name() );

  // write the view name list
  KConfigGroupSaver viewSaver( mConfig, "Views" );
  mConfig->writeEntry( "Names", mViewNameList );

  KConfigGroupSaver splitterSaver( mConfig, "Splitter" );
  mConfig->writeEntry( "ExtensionsSplitter", mExtensionBarSplitter->sizes() );
  mConfig->writeEntry( "DetailsSplitter", mDetailsSplitter->sizes() );
}

QStringList ViewManager::selectedUids() const
{
  return mActiveView->selectedUids();
}

void ViewManager::sendMail()
{
  sendMail( mActiveView->selectedEmails() );
}

void ViewManager::sendMail( const QString& email )
{
  kapp->invokeMailer( email, "" );
}

void ViewManager::mailVCard()
{
  QStringList uids = mActiveView->selectedUids();
  if ( !uids.isEmpty() )
    mailVCard( uids );
}

void ViewManager::mailVCard(const QStringList& uids)
{
  QStringList urls;
  // Create a temp dir, so that we can put the files in it with proper names
  KTempFile tempDir;
  if ( tempDir.status() != 0 ) {
    kdWarning() << strerror( tempDir.status() ) << endl;
    return;
  }

  QString dirName = tempDir.name();
  tempDir.unlink();
  QDir().mkdir(dirName,true);

  for( QStringList::ConstIterator it = uids.begin(); it != uids.end(); ++it ) {
    KABC::Addressee a = mAddressBook->findByUid( (*it) );

    if ( a.isEmpty() )
      continue;

    QString name = a.givenName() + "_" + a.familyName() + ".vcf";

    QString fileName = dirName + "/" + name;

    QFile outFile(fileName);
    if ( outFile.open(IO_WriteOnly) )
    {    // file opened successfully
      KABC::VCardConverter converter;
      QString vcard;

      converter.addresseeToVCard( a, vcard );

      QTextStream t( &outFile );        // use a text stream
      t.setEncoding( QTextStream::UnicodeUTF8 );
      t << vcard;

      outFile.close();

      urls.append( fileName );
    }
  }

  kapp->invokeMailer(QString::null, QString::null, QString::null,
                     QString::null, // subject
                     QString::null, // body
                     QString::null,
                     urls); // attachments
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
    KConfig *config = ViewManager::config();
    KConfigGroupSaver saver( config, name );
    QString type = config->readEntry( "Type", "Table" );

    kdDebug(5720) << "ViewManager::setActiveView: creating view - " << name << endl;

    ViewFactory *factory = mViewFactoryDict.find( type );
    if ( factory )
      view = factory->view( this, mViewWidgetStack, name.latin1() );

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
      emit currentFilterChanged( "" );
    else if ( view->defaultFilterType() == KAddressBookView::Active )
      emit currentFilterChanged( mCurrentFilter.name() );
    else {
      QString filterName = view->defaultFilterName();
      emit currentFilterChanged( filterName );
    }

    // Update the inc search widget to show the fields in the new active
    // view.
    mIncSearchWidget->setFields( mActiveView->fields() );
    mActiveView->refresh();

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

  ViewFactory *factory = mViewFactoryDict.find( mActiveView->type() );
  ViewConfigureWidget *wdg = 0;

  if ( factory ) {
    // Save the filters so the dialog has the latest set
    Filter::save( mConfig, "Filter", mFilterList );

    wdg = factory->configureWidget( this, 0 );
  }

  if ( wdg ) {
    ViewConfigureDialog dlg( wdg, mActiveView->name(), this );

    KConfigGroupSaver saver( mConfig, mActiveView->name() );
    dlg.restoreSettings( mConfig );

    if ( dlg.exec() ) {
      dlg.saveSettings( mConfig );
      mActiveView->readConfig( mConfig );

      // Set the proper filter in the view. By setting the combo
      // box, the activated slot will be called, which will push
      // the filter to the view and refresh it.
      if ( mActiveView->defaultFilterType() == KAddressBookView::None )
        emit currentFilterChanged( "" );
      else if ( mActiveView->defaultFilterType() == KAddressBookView::Active )
        emit currentFilterChanged( mCurrentFilter.name() );
      else {
        QString filterName = mActiveView->defaultFilterName();
        emit currentFilterChanged( filterName );
      }

      mIncSearchWidget->setFields( mActiveView->fields() );
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
    KConfig *config = ViewManager::config();
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
    KConfig *config = ViewManager::config();
    config->deleteGroup( newName );
    KConfigGroupSaver saver( config, newName );
    config->writeEntry( "Type", type );

    // try to set the active view
    emit viewConfigChanged( newName );

    modifyView();
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

void ViewManager::initGUI()
{
  mCurrentExtensionWidget = 0;

  QHBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing( KDialogBase::spacingHint() );

  mExtensionBarSplitter = new QSplitter( this );
  mExtensionBarSplitter->setOrientation( Qt::Vertical );

  mDetailsSplitter = new QSplitter( mExtensionBarSplitter );

  QVBox *viewSpace = new QVBox( mDetailsSplitter );
  mIncSearchWidget = new IncSearchWidget( viewSpace );
  connect( mIncSearchWidget, SIGNAL( doSearch( const QString& ) ),
           SLOT( incSearch( const QString& ) ) );

  mViewWidgetStack = new QWidgetStack( viewSpace );
  viewSpace->setStretchFactor( mViewWidgetStack, 1 );

  mDetails = new ViewContainer( mDetailsSplitter );
  connect( mDetails, SIGNAL( sendEmail( const QString& ) ),
           SLOT( sendMail( const QString& ) ) );
  connect( mDetails, SIGNAL( browse( const QString& ) ),
           SLOT( browse( const QString& ) ) );

  mJumpButtonBar = new JumpButtonBar( this );
  connect( mJumpButtonBar, SIGNAL( jumpToLetter( const QString& ) ),
           SLOT( incSearch( const QString& ) ) );
  connect( mIncSearchWidget, SIGNAL( fieldChanged() ),
           mJumpButtonBar, SLOT( recreateButtons() ) );

  // Setup the feature bar widget.
  mExtensionBar = new QHBox( mExtensionBarSplitter );

  loadExtensions();

  topLayout->addWidget( mExtensionBarSplitter );
  topLayout->setStretchFactor( mExtensionBarSplitter, 100 );
  topLayout->addWidget( mJumpButtonBar );
  topLayout->setStretchFactor( mJumpButtonBar, 1 );
}

void ViewManager::incSearch( const QString& text )
{
  if ( !mActiveView )
    return;

  mActiveView->setSelected( QString::null, false );

  if ( !text.isEmpty() ) {
    KABC::Field *field = mIncSearchWidget->currentField();

#if KDE_VERSION >= 319
    KABC::AddresseeList list( mAddressBook->allAddressees() );
    if (  field ) {
      list.sortByField( field );
      KABC::AddresseeList::Iterator it;
      for ( it = list.begin(); it != list.end(); ++it ) {
        if ( field->value( *it ).startsWith( text ) ) {
          mActiveView->setSelected( (*it).uid(), true );
          return;
        }
      }
    } else {
      KABC::AddresseeList::Iterator it;
      for ( it = list.begin(); it != list.end(); ++it ) {
        KABC::Field::List fieldList = mActiveView->fields();
        KABC::Field::List::ConstIterator fieldIt;
        for ( fieldIt = fieldList.begin(); fieldIt != fieldList.end(); ++fieldIt ) {
          if ( (*fieldIt)->value( *it ).startsWith( text ) ) {
            mActiveView->setSelected( (*it).uid(), true );
            return;
          }
        }
      }
    }
#else
    KABC::AddressBook::Iterator it;
    for ( it = mAddressBook->begin(); it != mAddressBook->end(); ++it ) {
      if ( field ) {
        if ( field->value( *it ).startsWith( text ) ) {
          mActiveView->setSelected( (*it).uid(), true );
          return;
        }
      } else {
        KABC::Field::List fieldList = mActiveView->fields();
        KABC::Field::List::ConstIterator fieldIt;
        for ( fieldIt = fieldList.begin(); fieldIt != fieldList.end(); ++fieldIt ) {
          if ( (*fieldIt)->value( *it ).startsWith( text ) ) {
            mActiveView->setSelected( (*it).uid(), true );
            return;
          }
        }
      }
    }
#endif
  }
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
  KURL::List urls;

  if ( KURLDrag::decode( e, urls) ) {
    KURL::List::Iterator it = urls.begin();
    int c = urls.count();
    if ( c > 1 ) {
      QString questionString = i18n( "Import one contact into your addressbook?", "Import %n contacts into your addressbook?", c );
      if ( KMessageBox::questionYesNo( this, questionString, i18n( "Import Contacts?" ) ) == KMessageBox::Yes ) {
        for ( ; it != urls.end(); ++it )
          emit importVCard( *it, false );
      }
    } else if ( c == 1 )
      emit importVCard( *it, true );
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
  if ( !mDetails->isHidden() )
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

void ViewManager::setActiveExtension( int id )
{
  if ( id == 0 ) {
    mExtensionBar->hide();
    mCurrentExtensionWidget = 0;
  } else {
    if ( mCurrentExtensionWidget )
      mCurrentExtensionWidget->hide();

    mCurrentExtensionWidget = mExtensionWidgetList.at( id - 1 );
    if ( mCurrentExtensionWidget ) {
      mExtensionBar->show();
      mCurrentExtensionWidget->show();
    } else {
      mExtensionBar->hide();
      mCurrentExtensionWidget = 0;
    }
  }
}

KConfig* ViewManager::config()
{
  static KConfig *mConfig = 0;
  if ( !mConfig )
    mConfig = new KConfig( locateLocal( "config", "kaddressbookrc" ) );
  return mConfig;
}


QStringList ViewManager::extensionNames()
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

KABC::Field *ViewManager::currentSearchField()
{
  return mIncSearchWidget->currentField();
}

const QStringList &ViewManager::viewNames()
{
  return mViewNameList;
}

void ViewManager::setFilters( const Filter::List &list )
{
  mFilterList = list;
  emit filtersEdited();
}

const Filter::List &ViewManager::filters() const
{
  return mFilterList;
}

QStringList ViewManager::filterNames()
{
  QStringList names;

  Filter::List::Iterator it;
  for ( it = mFilterList.begin(); it != mFilterList.end(); ++it )
    names.append( (*it).name() );

  return names;
}

void ViewManager::loadExtensions()
{
  // clear extension widget list
  mExtensionWidgetList.setAutoDelete( true );
  QPtrListIterator<ExtensionWidget> wdgIt( mExtensionWidgetList );
  ExtensionWidget *wdg = 0;
  while ( ( wdg = wdgIt.current() ) != 0 )
    mExtensionWidgetList.remove( wdg );

  mExtensionWidgetList.setAutoDelete( false );

  // add addressee editor as default
  wdg = new AddresseeEditorWidget( this, mExtensionBar );
  wdg->hide();
  connect( wdg, SIGNAL( modified( KABC::Addressee::List ) ),
           SLOT( extensionWidgetModified( KABC::Addressee::List ) ) );
  mExtensionWidgetList.append( wdg );

  // load the other extensions
  KConfig *config = ViewManager::config();
  KConfigGroupSaver saver( config, "Extensions_General" );
  QStringList activeExtensions = config->readListEntry( "activeExtensions" );

  KTrader::OfferList plugins = KTrader::self()->query( "KAddressBook/Extension" );
  KTrader::OfferList::ConstIterator it;
  for ( it = plugins.begin(); it != plugins.end(); ++it ) {
    if ( !(*it)->hasServiceType( "KAddressBook/Extension" ) )
      continue;

    KLibFactory *factory = KLibLoader::self()->factory( (*it)->library() );
    if ( !factory ) {
      kdDebug(5720) << "ViewManager::loadExtensions(): Factory creation failed" << endl;
      continue;
    }

    ExtensionFactory *extensionFactory = static_cast<ExtensionFactory*>( factory );

    if ( !extensionFactory ) {
      kdDebug(5720) << "ViewManager::loadExtensions(): Cast failed" << endl;
      continue;
    }

    if ( !activeExtensions.contains( extensionFactory->identifier() ) )
      continue;

    wdg = extensionFactory->extension( this, mExtensionBar );
    if ( wdg ) {
      wdg->hide();
      connect( wdg, SIGNAL( modified( KABC::Addressee::List ) ),
               SLOT( extensionWidgetModified( KABC::Addressee::List ) ) );
      mExtensionWidgetList.append( wdg );
    }
  }

  emit extensionsReloaded();

  mIncSearchWidget->setFocus();
}

#include "viewmanager.moc"
