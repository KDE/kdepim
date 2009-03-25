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

#include <QFile>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QDropEvent>
#include <kselectaction.h>
#include <kactioncollection.h>
#include <libkdepim/kvcarddrag.h>
#include <kabc/addressbook.h>
#include <kabc/vcardconverter.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktempdir.h>
#include <kservicetypetrader.h>
#include <kicon.h>
#include <kconfiggroup.h>

#include "addviewdialog.h"
#include "addresseeutil.h"
#include "core.h"
#include "filtereditdialog.h"
#include "filterselectionwidget.h"
#include "kabprefs.h"

#include "viewmanager.h"

ViewManager::ViewManager( KAB::Core *core, QWidget *parent, const char *name )
  : QWidget( parent ), mCore( core ), mActiveView( 0 ),
    mFilterSelectionWidget( 0 )
{
  setObjectName( name );
  initGUI();
  initActions();

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
  QString activeViewName = KABPrefs::instance()->currentView();

  mActionSelectView->setItems( mViewNameList );
  mActionSelectView->setCurrentItem( mViewNameList.indexOf( activeViewName ) );

  // Filter
  mFilterList = Filter::restore( mCore->config(), "Filter" );
  mFilterSelectionWidget->setItems( filterNames() );
  mFilterSelectionWidget->setCurrentItem( KABPrefs::instance()->currentFilter() );

  // Tell the views to reread their config, since they may have
  // been modified by global settings
  QHashIterator<QString, KAddressBookView* > it( mViewDict );
  while ( it.hasNext() ) {
    it.next();
    KConfigGroup group( mCore->config(), it.key() );
    it.value()->readConfig( group );
  }

  setActiveView( activeViewName );

  mActionDeleteView->setEnabled( mViewNameList.count() > 1 );
}

void ViewManager::saveSettings()
{
  QHashIterator<QString, KAddressBookView*> it( mViewDict );
  while ( it.hasNext() ) {
    it.next();
    KConfigGroup group( mCore->config(), it.key() );
    it.value()->writeConfig( group );
  }

  Filter::save( mCore->config(), "Filter", mFilterList );
  KABPrefs::instance()->setCurrentFilter( mFilterSelectionWidget->currentItem() );

  // write the view name list
  KABPrefs::instance()->setViewNames( mViewNameList );

  if ( mActiveView )
    KABPrefs::instance()->setCurrentView( mActiveView->windowTitle() );
}

QStringList ViewManager::selectedUids() const
{
  if ( mActiveView ) {
    return mActiveView->selectedUids();
  } else
    return QStringList();
}

QStringList ViewManager::selectedEmails() const
{
  if ( mActiveView && !mActiveView->selectedEmails().isEmpty() )
    return QStringList( mActiveView->selectedEmails() );
  else
    return QStringList();
}

KABC::Addressee::List ViewManager::selectedAddressees() const
{
  KABC::Addressee::List list;

  const QStringList uids = selectedUids();
  QStringList::ConstIterator it;
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

void ViewManager::setSelected( const QString &uid, bool selected )
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
  while ( mViewDict.count() ) {
    KAddressBookView *view = *(mViewDict.begin());
    mViewDict.remove( mViewDict.key( view ) );
    delete view;
  }
  mActiveView = 0;
}

void ViewManager::setActiveView( const QString &name )
{
  kDebug() << "entering ViewManager::setActiveView";

  KAddressBookView *view = 0;

  // Check that this isn't the same as the current active view
  if ( mActiveView && ( mActiveView->windowTitle() == name ) )
    return;

  // At this point we know the view that should be active is not
  // currently active. We will try to find the new on in the list. If
  // we can't find it, it means it hasn't been instantiated, so we will
  // create it on demand.

  view = mViewDict.value( name );

  // Check if we found the view. If we didn't, then we need to create it
  if ( !view ) {
    KConfig *config = mCore->config();
    KConfigGroup group( config, name );
    QString type = group.readEntry( "Type", "Table" );

    kDebug(5720) <<"ViewManager::setActiveView: creating view -" << name;

    ViewFactory *factory = mViewFactoryDict.value( type );
    if ( factory )
      view = factory->view( mCore, mViewWidgetStack );

    if ( view ) {
      view->setWindowTitle( name );
      mViewDict.insert( name, view );
      mViewWidgetStack->addWidget( view );
      view->readConfig( group );

      // The manager just relays the signals
      connect( view, SIGNAL( selected( const QString& ) ),
               SIGNAL( selected( const QString & ) ) );
      connect( view, SIGNAL( executed( const QString& ) ),
               SIGNAL( executed( const QString& ) ) );
      connect( view, SIGNAL( modified() ), SIGNAL( modified() ) );
      connect( view, SIGNAL( dropped( QDropEvent* ) ),
               SLOT( dropped( QDropEvent* ) ) );
      connect( view, SIGNAL( startDrag() ), SLOT( startDrag() ) );
      connect( view, SIGNAL( sortFieldChanged() ), SIGNAL( sortFieldChanged() ) );
    }
  }

  // If we found or created the view, raise it and refresh it
  if ( view ) {
    mActiveView = view;
    mViewWidgetStack->setCurrentWidget( view );
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
    kDebug(5720) <<"ViewManager::setActiveView: unable to find view";
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

  ViewFactory *factory = mViewFactoryDict.value( mActiveView->type() );
  ViewConfigureWidget *wdg = 0;

  if ( factory ) {
    // Save the filters so the dialog has the latest set
    Filter::save( mCore->config(), "Filter", mFilterList );

    wdg = factory->configureWidget( mCore->addressBook(), 0 );
  }

  if ( wdg ) {
    ViewConfigureDialog dlg( wdg, mActiveView->windowTitle(), this );

    KConfigGroup group( mCore->config(), mActiveView->windowTitle() );
    dlg.restoreSettings( group );

    if ( dlg.exec() ) {
      dlg.saveSettings( group );
      mActiveView->readConfig( group );
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
  QString text = i18n( "<qt>Are you sure that you want to delete the view <b>%1</b>?</qt>" ,
                       mActiveView->windowTitle() );
  QString caption = i18n( "Confirm Delete" );

  if ( KMessageBox::warningContinueCancel( this, text, caption, KGuiItem( i18n( "&Delete" ), "edit-delete" ) ) == KMessageBox::Continue ) {
    mViewNameList.removeAll( mActiveView->windowTitle() );

    // remove the view from the config file
    KConfig *config = mCore->config();
    config->deleteGroup( mActiveView->windowTitle() );

    KAddressBookView *view = mViewDict[ mActiveView->windowTitle() ];
    mViewDict.remove( mActiveView->windowTitle() );
    delete view;
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

      newName = QString( "%1 <%2>" ).arg( newName ).arg( numTries );
      numTries++;
    }

    // Add the new one to the list
    mViewNameList.append( newName );

    // write the view to the config file,
    KConfig *config = mCore->config();
    config->deleteGroup( newName );
    KConfigGroup group( config, newName );
    group.writeEntry( "Type", type );

    // try to set the active view
    mActionSelectView->setItems( mViewNameList );
    mActionSelectView->setCurrentItem( mViewNameList.indexOf( newName ) );
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
  const KService::List plugins = KServiceTypeTrader::self()->query( "KAddressBook/View",
    QString( "[X-KDE-KAddressBook-ViewPluginVersion] == %1" ).arg(  KAB_VIEW_PLUGIN_VERSION ) );
  foreach ( KService::Ptr pluginService, plugins ) {
    KPluginFactory *factory = KPluginLoader( *pluginService ).factory();
    if ( !factory ) {
      kDebug(5720) <<"ViewManager::createViewFactories(): Factory creation failed";
      continue;
    }

    ViewFactory *viewFactory = dynamic_cast<ViewFactory*>( factory );

    if ( !viewFactory ) {
      kDebug(5720) <<"ViewManager::createViewFactories(): Cast failed";
      continue;
    }

    mViewFactoryDict.insert( viewFactory->type(), viewFactory );
  }
}

void ViewManager::dropped( QDropEvent *e )
{
  kDebug(5720) <<"ViewManager::dropped: got a drop event";

  // don't allow drops from our own drags
  if ( e->source() == this )
    return;

  const QMimeData *md = e->mimeData();

  QString clipText, vcards;
  KUrl::List urls = KUrl::List::fromMimeData( md );
  if ( !urls.isEmpty() ) {
    KUrl::List::ConstIterator it = urls.begin();
    int c = urls.count();
    if ( c > 1 ) {
      QString questionString = i18np( "Import one contact into your addressbook?", "Import %1 contacts into your addressbook?", c );
      if ( KMessageBox::questionYesNo( this, questionString, i18n( "Import Contacts?" ), KGuiItem(i18n( "Import" )), KGuiItem(i18n( "Do Not Import" )) ) == KMessageBox::Yes ) {
        for ( ; it != urls.end(); ++it )
          emit urlDropped( *it );
      }
    } else if ( c == 1 )
      emit urlDropped( *it );
  } else if ( KPIM::KVCardDrag::canDecode( md ) ) {
    KABC::Addressee::List list;

    KPIM::KVCardDrag::fromMimeData( md, list );

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
  const QStringList uidList = selectedUids();
  if (  uidList.isEmpty() )
    return;

  kDebug(5720) <<"ViewManager::startDrag: starting to drag";

  QStringList::ConstIterator it;
  for ( it = uidList.begin(); it != uidList.end(); ++it )
    addrList.append( mCore->addressBook()->findByUid( *it ) );

  QDrag *drag = new QDrag( this );
  QMimeData *mimeData = new QMimeData;
  drag->setMimeData(mimeData);

  // Text should be first as the default format. If an application explicitly requests vcard, it will still get it.
  mimeData->setText( AddresseeUtil::addresseesToEmails( addrList ) );
  KPIM::KVCardDrag::populateMimeData( mimeData, addrList );


  KTempDir tempDir;
  // can't set tempDir to autoDelete, in case of dropping on the desktop, the copy is async...
  tempDir.setAutoRemove(false);
  if ( tempDir.status() == 0 ) {
    QString fileName;
    if ( addrList.count() == 1 )
      fileName = addrList[ 0 ].givenName() + '_' + addrList[ 0 ].familyName() + ".vcf";
    else
      fileName = "contacts.vcf";

    QFile tempFile( tempDir.name() + '/' + fileName );
    if ( tempFile.open( QIODevice::WriteOnly ) ) {
      KABC::VCardConverter converter;
      QByteArray vcards = converter.createVCards( addrList );
      tempFile.write( vcards );
      tempFile.close();

      KUrl url( tempFile.fileName() );
      url.populateMimeData( mimeData );
    }
  }

  drag->setPixmap( KIconLoader::global()->loadIcon( "view-pim-contacts", KIconLoader::Desktop ) );
  drag->start();
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
    emit selected( QString() );
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

QStringList ViewManager::filterNames() const
{
  QStringList names( i18n( "None" ) );
  names.append( i18n( "Unfiled" ) );

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
      return pos + 2;

  return 0;
}

void ViewManager::initActions()
{
  KActionCollection *coll = mCore->actionCollection();
  mActionSelectView = coll->add<KSelectAction>( "select_view" );
  mActionSelectView->setText( i18n( "Select View" ) );
  mActionSelectView->setMenuAccelsEnabled( false );
  connect( mActionSelectView, SIGNAL( triggered( const QString& ) ),
           SLOT( setActiveView( const QString& ) ) );

  QAction *action;

  action = coll->addAction( "view_modify" );
  action->setIcon( KIcon("configure") );
  action->setText( i18n( "Modify View..." ) );
  connect(action, SIGNAL(triggered(bool) ), SLOT( editView() ));
  action->setWhatsThis( i18n( "By pressing this button a dialog opens that allows you to modify the view of the addressbook. There you can add or remove fields that you want to be shown or hidden in the addressbook like the name for example." ) );

  action = coll->addAction( "view_add" );
  action->setIcon( KIcon("window-new") );
  action->setText( i18n( "Add View..." ) );
  connect(action, SIGNAL(triggered(bool) ), SLOT( addView() ));
  action->setWhatsThis( i18n( "You can add a new view by choosing one from the dialog that appears after pressing the button. You have to give the view a name, so that you can distinguish between the different views." ) );

  mActionDeleteView = coll->addAction( "view_delete" );
  mActionDeleteView->setIcon( KIcon("view-close") );
  mActionDeleteView->setText( i18n( "Delete View" ) );
  connect(mActionDeleteView, SIGNAL(triggered(bool) ), SLOT( deleteView() ));
  mActionDeleteView->setWhatsThis( i18n( "By pressing this button you can delete the actual view, which you have added before." ) );

  action = coll->addAction( "view_refresh" );
  action->setIcon( KIcon("view-refresh") );
  action->setText( i18n( "Refresh View" ) );
  connect(action, SIGNAL(triggered(bool) ), SLOT( refreshView() ));
  action->setWhatsThis( i18n( "The view will be refreshed by pressing this button." ) );

  action = coll->addAction( "options_edit_filters" );
  action->setIcon( KIcon("view-filter") );
  action->setText( i18n( "Edit &Filters..." ) );
  connect(action, SIGNAL(triggered(bool) ), SLOT( configureFilters() ));
  action->setWhatsThis( i18n( "Edit the contact filters<p>You will be presented with a dialog, where you can add, remove and edit filters.</p>" ) );
}

void ViewManager::initGUI()
{
  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setSpacing( 0 );
  layout->setMargin( 0 );
  mViewWidgetStack = new QStackedWidget( this );
  layout->addWidget( mViewWidgetStack );
}

#include "viewmanager.moc"
