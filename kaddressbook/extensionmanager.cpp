/*
    This file is part of KAddressbook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <kactionclasses.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <ktrader.h>

#include <qlayout.h>
#include <qobjectlist.h>
#include <qsignalmapper.h>
#include <qsplitter.h>
#include <qtimer.h>
#include <qwidgetstack.h>

#include "addresseeeditorextension.h"
#include "core.h"
#include "kabprefs.h"

#include "extensionmanager.h"

ExtensionData::ExtensionData() : action( 0 ), widget( 0 ), weight( 0 ), isDetailsExtension( false )
{
}

ExtensionManager::ExtensionManager( QWidget* extensionBar, QWidgetStack* detailsStack, KAB::Core *core, QObject *parent,
                                    const char *name )
    : QObject( parent, name ), mExtensionBar( extensionBar ), mCore( core ),
      mMapper( 0 ), mDetailsStack( detailsStack ), mActiveDetailsWidget( 0 )
{
  Q_ASSERT( mExtensionBar );
  QVBoxLayout* layout = new QVBoxLayout( mExtensionBar );
  mSplitter = new QSplitter( mExtensionBar );
  mSplitter->setOrientation( QSplitter::Vertical );
  layout->addWidget( mSplitter );

  createExtensionWidgets();

  mActionCollection = new KActionCollection( this, "ActionCollection" );

  extensionBar->setShown( false );
  QTimer::singleShot( 0, this, SLOT( createActions() ) );
}

ExtensionManager::~ExtensionManager()
{
}


void ExtensionManager::restoreSettings()
{
  const QStringList activeExtensions = KABPrefs::instance()->activeExtensions();

  typedef QMap<QString, ExtensionData>::ConstIterator ConstIterator;
  for ( ConstIterator it = mExtensionMap.begin(), end = mExtensionMap.end(); it != end; ++it ) {
    if ( activeExtensions.contains( it.data().identifier ) ) {
      KToggleAction *action = static_cast<KToggleAction*>( it.data().action );
      if ( action )
        action->setChecked( true );
      setExtensionActive( it.data().identifier, true );
    }
  }
  const QValueList<int> sizes = KABPrefs::instance()->extensionsSplitterSizes();
  mSplitter->setSizes( sizes );
}

void ExtensionManager::saveSettings()
{
  KABPrefs::instance()->setActiveExtensions( mActiveExtensions );
  KABPrefs::instance()->setExtensionsSplitterSizes( mSplitter->sizes() );
}

void ExtensionManager::reconfigure()
{
  saveSettings();
  createExtensionWidgets();
  createActions();
  restoreSettings();
  mExtensionBar->setShown( !mActiveExtensions.isEmpty() );
}

bool ExtensionManager::isQuickEditVisible() const
{
  return mActiveExtensions.contains( "contact_editor" );
}

void ExtensionManager::setSelectionChanged()
{
  for ( QStringList::ConstIterator it = mActiveExtensions.begin(), end = mActiveExtensions.end(); it != end; ++it ) {
    if ( mExtensionMap.contains( *it ) && mExtensionMap[*it].widget )
      mExtensionMap[*it].widget->contactsSelectionChanged();
  }
}

void ExtensionManager::activationToggled( const QString &extid )
{
  if ( !mExtensionMap.contains( extid ) )
    return;
  const ExtensionData data = mExtensionMap[ extid ];
  const bool activated = data.action->isChecked();
  setExtensionActive( extid, activated );
}

void ExtensionManager::setExtensionActive( const QString& extid, bool active )
{
  if ( !mExtensionMap.contains( extid ) )
    return;
  if ( mActiveExtensions.contains( extid ) == active )
    return;
  const ExtensionData data = mExtensionMap[ extid ];
  if ( active ) {
    mActiveExtensions.append( extid );
    if ( data.widget ) {
      if ( data.isDetailsExtension ) {
        mActiveDetailsWidget = data.widget;
        emit detailsWidgetActivated( data.widget );
      } else {
          data.widget->show();
      }
      data.widget->contactsSelectionChanged();
    }
  } else {
    mActiveExtensions.remove( extid );
    if ( data.widget && !data.isDetailsExtension ) {
      data.widget->hide();
    }
    if ( data.isDetailsExtension ) {
      mActiveDetailsWidget = 0;
      emit detailsWidgetDeactivated( data.widget );
    }
  }
  mExtensionBar->setShown( !mActiveExtensions.isEmpty() );
}

void ExtensionManager::createActions()
{
  mCore->guiClient()->unplugActionList( "extensions_list" );
  mActionList.setAutoDelete( true );
  mActionList.clear();
  mActionList.setAutoDelete( false );

  delete mMapper;
  mMapper = new QSignalMapper( this, "SignalMapper" );
  connect( mMapper, SIGNAL( mapped( const QString& ) ),
           this, SLOT( activationToggled( const QString& ) ) );

  ExtensionData::List::ConstIterator it;
  for ( QMap<QString, ExtensionData>::Iterator it = mExtensionMap.begin(), end = mExtensionMap.end(); it != end; ++it ) {
    ExtensionData& data = it.data();
    data.action = new KToggleAction( data.title, 0, mMapper, SLOT( map() ),
                                               mActionCollection,
                                               QString( data.identifier + "_extension" ).latin1() );
    mMapper->setMapping( data.action, data.identifier );
    mActionList.append( data.action );

    if ( mActiveExtensions.contains( data.identifier ) )
      data.action->setChecked( true );
  }

  mActionList.append( new KActionSeparator( mActionCollection ) );
  mCore->guiClient()->plugActionList( "extensions_list", mActionList );
}

QWidget* ExtensionManager::activeDetailsWidget() const
{
    return mActiveDetailsWidget;
}

void ExtensionManager::createExtensionWidgets()
{
  // clean up
  for ( QMap<QString, ExtensionData>::ConstIterator it = mExtensionMap.begin(), end = mExtensionMap.end(); it != end; ++it ) {
    delete it.data().widget;
  }
  mExtensionMap.clear();

  KAB::ExtensionWidget *wdg = 0;

  {
    // add addressee editor as default
    wdg = new AddresseeEditorExtension( mCore, mDetailsStack );
    wdg->hide();

    connect( wdg, SIGNAL( modified( const KABC::Addressee::List& ) ),
             SIGNAL( modified( const KABC::Addressee::List& ) ) );
    connect( wdg, SIGNAL( deleted( const QStringList& ) ),
             SIGNAL( deleted( const QStringList& ) ) );

    ExtensionData data;
    data.identifier = wdg->identifier();
    data.title = wdg->title();
    data.widget = wdg;
    data.isDetailsExtension = true;
    mExtensionMap.insert( data.identifier, data );
  }

  // load the other extensions
  const KTrader::OfferList plugins = KTrader::self()->query( "KAddressBook/Extension",
    QString( "[X-KDE-KAddressBook-ExtensionPluginVersion] == %1" ).arg( KAB_EXTENSIONWIDGET_PLUGIN_VERSION ) );

  KTrader::OfferList::ConstIterator it;
  for ( it = plugins.begin(); it != plugins.end(); ++it ) {
    KLibFactory *factory = KLibLoader::self()->factory( (*it)->library().latin1() );
    if ( !factory ) {
      kdDebug(5720) << "ExtensionManager::loadExtensions(): Factory creation failed" << endl;
      continue;
    }

    KAB::ExtensionFactory *extensionFactory = static_cast<KAB::ExtensionFactory*>( factory );

    if ( !extensionFactory ) {
      kdDebug(5720) << "ExtensionManager::loadExtensions(): Cast failed" << endl;
      continue;
    }

    wdg = extensionFactory->extension( mCore, mSplitter );
    if ( wdg ) {
      if ( wdg->identifier() == "distribution_list_editor_ng" )
          mSplitter->moveToFirst( wdg );
      wdg->hide();
      connect( wdg, SIGNAL( modified( const KABC::Addressee::List& ) ),
               SIGNAL( modified( const KABC::Addressee::List& ) ) );
      connect( wdg, SIGNAL( deleted( const QStringList& ) ),
               SIGNAL( deleted( const QStringList& ) ) );

      ExtensionData data;
      data.identifier = wdg->identifier();
      data.title = wdg->title();
      data.widget = wdg;
      mExtensionMap.insert( data.identifier, data );
    }
  }
}

#include "extensionmanager.moc"
