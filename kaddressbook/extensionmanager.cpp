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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kactionclasses.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <ktrader.h>

#include <qsignalmapper.h>
#include <qtimer.h>

#include "addresseeeditorwidget.h"
#include "simpleaddresseeeditor.h"
#include "core.h"
#include "kabprefs.h"

#include "extensionmanager.h"

ExtensionManager::ExtensionManager( KAB::Core *core, QWidget *parent,
                                    const char *name )
  : QHBox( parent, name ), mCore( core ), mCurrentExtensionWidget( 0 ),
    mMapper( 0 )
{
  createExtensionWidgets();

  mActionCollection = new KActionCollection( this, "ActionCollection" );

  QTimer::singleShot( 0, this, SLOT( createActions() ) );
}

ExtensionManager::~ExtensionManager()
{
}

void ExtensionManager::restoreSettings()
{
  for ( uint index = 0; index < mExtensionList.size(); ++index ) {
    ExtensionData data = mExtensionList[ index ];
    if ( data.identifier == KABPrefs::instance()->currentExtension() ) {
      KToggleAction *action = static_cast<KToggleAction*>( mActionList.at( index ) );
      if ( action )
        action->setChecked( true );
      setActiveExtension( index );
      return;
    }
  }

  if ( mActionList.first() )
    static_cast<KToggleAction*>( mActionList.first() )->setChecked( true );
  setActiveExtension( 0 );
}

void ExtensionManager::saveSettings()
{
  KAction *action;
  uint index = 0;
  for ( action = mActionList.first(); action; action = mActionList.next(), index++ )
    if ( static_cast<KToggleAction*>( action )->isChecked() )
      break;

  Q_ASSERT( index < mExtensionList.size() );

  KABPrefs::instance()->setCurrentExtension( mExtensionList[ index ].identifier );
}

void ExtensionManager::reconfigure()
{
  saveSettings();
  createExtensionWidgets();
  createActions();
  restoreSettings();
}

bool ExtensionManager::isQuickEditVisible() const
{
  return ( mCurrentExtensionWidget &&
      mCurrentExtensionWidget->identifier() == "contact_editor" );
}

void ExtensionManager::setSelectionChanged()
{
  if ( mCurrentExtensionWidget )
    mCurrentExtensionWidget->contactsSelectionChanged();
}

void ExtensionManager::setActiveExtension( int id )
{
  if ( id == 0 ) {
    hide();
    if ( mCurrentExtensionWidget )
      mCurrentExtensionWidget->hide();
    mCurrentExtensionWidget = 0;
  } else if ( id > 0 ) {
    if ( mCurrentExtensionWidget )
      mCurrentExtensionWidget->hide();

    mCurrentExtensionWidget = mExtensionList[ id ].widget;
    if ( mCurrentExtensionWidget ) {
      show();
      mCurrentExtensionWidget->show();
      mCurrentExtensionWidget->contactsSelectionChanged();
    } else {
      hide();
      mCurrentExtensionWidget = 0;
    }
  }
}

void ExtensionManager::createActions()
{
  mCore->guiClient()->unplugActionList( "extensions_list" );
  mActionList.setAutoDelete( true );
  mActionList.clear();
  mActionList.setAutoDelete( false );

  delete mMapper;
  mMapper = new QSignalMapper( this, "SignalMapper" );
  connect( mMapper, SIGNAL( mapped( int ) ),
           this, SLOT( setActiveExtension( int ) ) );

  int actionCounter = 0;
  ExtensionData::List::ConstIterator it;
  for ( it = mExtensionList.begin(); it != mExtensionList.end(); ++it ) {
    ExtensionData data = *it;
    KToggleAction *action = new KToggleAction( data.title, 0, mMapper, SLOT( map() ),
                                               mActionCollection,
                                               QString( data.identifier + "_extension" ).latin1() );
    action->setExclusiveGroup( "extensions" );
    mMapper->setMapping( action, actionCounter++ );
    mActionList.append( action );

    if ( data.widget == mCurrentExtensionWidget )
      action->setChecked( true );
  }

  mCore->guiClient()->plugActionList( "extensions_list", mActionList );

  if ( mCurrentExtensionWidget == 0 && mActionList.first() )
    static_cast<KToggleAction*>( mActionList.first() )->setChecked( true );
}

void ExtensionManager::createExtensionWidgets()
{
  // clean up
  ExtensionData::List::ConstIterator dataIt;
  for ( dataIt = mExtensionList.begin(); dataIt != mExtensionList.end(); ++dataIt )
    delete (*dataIt).widget;
  mExtensionList.clear();

  KAB::ExtensionWidget *wdg = 0;

  {
    // add 'None' entry
    ExtensionData data;
    data.identifier = "none";
    data.title = i18n( "None" );
    data.widget = 0;
    mExtensionList.append( data );
  }

  {
    // add addressee editor as default
    if ( KABPrefs::instance()->editorType() == KABPrefs::SimpleEditor ) {
      wdg = new SimpleAddresseeEditor( mCore, true, this );
    } else {
      wdg = new AddresseeEditorWidget( mCore, true, this );
    }
    wdg->hide();

    connect( wdg, SIGNAL( modified( const KABC::Addressee::List& ) ),
             SIGNAL( modified( const KABC::Addressee::List& ) ) );

    ExtensionData data;
    data.identifier = wdg->identifier();
    data.title = wdg->title();
    data.widget = wdg;
    mExtensionList.append( data );
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

    wdg = extensionFactory->extension( mCore, this );
    if ( wdg ) {
      wdg->hide();
      connect( wdg, SIGNAL( modified( const KABC::Addressee::List& ) ),
               SIGNAL( modified( const KABC::Addressee::List& ) ) );

      ExtensionData data;
      data.identifier = wdg->identifier();
      data.title = wdg->title();
      data.widget = wdg;
      mExtensionList.append( data );
    }
  }

  mCurrentExtensionWidget = 0;
}

#include "extensionmanager.moc"
