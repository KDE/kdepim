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

#include <kactioncollection.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kservicetypetrader.h>
#include <ktoggleaction.h>
#include <kvbox.h>

#include <QHBoxLayout>
#include <QSignalMapper>
#include <QSplitter>
#include <QStackedWidget>
#include <QTimer>

#include "addresseeeditorextension.h"
#include "core.h"
#include "kabprefs.h"

#include "extensionmanager.h"

ExtensionData::ExtensionData() : action( 0 ), widget( 0 ), weight( 0 ), isDetailsExtension( false )
{
}

ExtensionManager::ExtensionManager( QWidget* extensionBar, QStackedWidget* detailsStack, KAB::Core *core, QObject *parent )
    : QObject( parent ), mExtensionBar( extensionBar ), mCore( core ),
    mMapper( 0 ), mDetailsStack( detailsStack ), mActiveDetailsWidget( 0 )
{
  Q_ASSERT( mCore );
  Q_ASSERT( mExtensionBar );
  Q_ASSERT( mDetailsStack );
  QBoxLayout *layout = new QHBoxLayout( mExtensionBar );
  layout->setMargin( 0 );
  layout->setSpacing( 0 );
  mSplitter = new QSplitter( mExtensionBar );
  mSplitter->setOrientation( Qt::Vertical );
  layout->addWidget( mSplitter );

  createExtensionWidgets();

  mActionCollection = new KActionCollection( this );
  mActionCollection->setObjectName( "ActionCollection" );
  QTimer::singleShot( 0, this, SLOT( createActions() ) );
}

ExtensionManager::~ExtensionManager()
{
}


void ExtensionManager::restoreSettings() 
{
  const QStringList activeExtensions = KABPrefs::instance()->activeExtensions();
  Q_FOREACH ( const ExtensionData data, mExtensionMap ) {
      if ( activeExtensions.contains( data.identifier ) ) {
      KToggleAction *action = static_cast<KToggleAction*>( data.action );
      if ( action )
        action->setChecked( true );
      setExtensionActive( data.identifier, true );
    }
  }
  //const QList<int> sizes = KABPrefs::instance()->extensionsSplitterSizes();
  //mSplitter->setSizes( sizes );
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
  updateExtensionBarVisibility();
}

bool ExtensionManager::isQuickEditVisible() const
{
  return mActiveExtensions.contains( "contact_editor" );
}

void ExtensionManager::setSelectionChanged()
{
  foreach ( const QString i, mActiveExtensions ) {
    if ( mExtensionMap.contains( i ) && mExtensionMap[i].widget )
      mExtensionMap[i].widget->contactsSelectionChanged();
  } 
}

void ExtensionManager::activationToggled( const QString &extid )
{
  if ( !mExtensionMap.contains( extid ) )
    return;
  setExtensionActive( extid, mExtensionMap[ extid ].action->isChecked() );
}

void ExtensionManager::updateExtensionBarVisibility()
{
  foreach ( const QString i, mActiveExtensions ) {
    if ( mExtensionMap[i].widget && !mExtensionMap[i].isDetailsExtension ) {
      mExtensionBar->setVisible( true );
      return;
    }
  }

  mExtensionBar->setVisible( false );
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
          data.widget->setVisible( true );
      }
      data.widget->contactsSelectionChanged();
    }
  } else {
    mActiveExtensions.removeAll( extid );
    if ( data.isDetailsExtension ) {
      mActiveDetailsWidget = 0;
      emit detailsWidgetDeactivated( data.widget );
    } else {
        data.widget->setVisible( false );
    }
  }
  updateExtensionBarVisibility();
}
 
void ExtensionManager::createActions()
{
  mCore->guiClient()->unplugActionList( "extensions_list" );
  qDeleteAll( mActionList );
  mActionList.clear();

  delete mMapper;
  mMapper = new QSignalMapper( this );
  mMapper->setObjectName( "SignalMapper" );
  connect( mMapper, SIGNAL( mapped( const QString& ) ),
           this, SLOT( activationToggled( const QString& ) ) );

  foreach ( const QString i, mExtensionMap.keys() ) {
    ExtensionData& data = mExtensionMap[i];
    data.action = mActionCollection->add<KToggleAction>( QString( data.identifier + "_extension" ) );
    data.action->setText( data.title );
    connect( data.action, SIGNAL(triggered(bool) ), mMapper, SLOT( map() ));
    mMapper->setMapping( data.action, data.identifier );
    mActionList.append( data.action );
    if ( mActiveExtensions.contains( data.identifier ) )
      data.action->setChecked( true );
  }

  mCore->guiClient()->plugActionList( "extensions_list", mActionList );
  updateExtensionBarVisibility();
}

QWidget* ExtensionManager::activeDetailsWidget() const
{
    return mActiveDetailsWidget;
}

void ExtensionManager::createExtensionWidgets()
{
  // clean up
  foreach ( const ExtensionData i, mExtensionMap ) 
    delete i.widget;
  mExtensionMap.clear();

  {
    // add addressee editor as default
    KAB::ExtensionWidget *wdg = new AddresseeEditorExtension( mCore, 0 );
    mDetailsStack->addWidget( wdg );
    wdg->setVisible( false );

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
  const KService::List plugins = KServiceTypeTrader::self()->query( "KAddressBook/Extension",
    QString( "[X-KDE-KAddressBook-ExtensionPluginVersion] == %1" ).arg( KAB_EXTENSIONWIDGET_PLUGIN_VERSION ) );

  KService::List::ConstIterator it;
  for ( it = plugins.begin(); it != plugins.end(); ++it ) {
    KLibFactory *factory = KLibLoader::self()->factory( (*it)->library().toLatin1() );
    if ( !factory ) {
      kDebug(5720) <<"ExtensionManager::loadExtensions(): Factory creation failed";
      continue;
    }

    KAB::ExtensionFactory *extensionFactory = static_cast<KAB::ExtensionFactory*>( factory );

    if ( !extensionFactory ) {
      kDebug(5720) <<"ExtensionManager::loadExtensions(): Cast failed";
      continue;
    }

    KAB::ExtensionWidget *wdg = extensionFactory->extension( mCore, 0 );
    if ( wdg ) {
      mSplitter->addWidget( wdg );
      wdg->setVisible( false );
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
