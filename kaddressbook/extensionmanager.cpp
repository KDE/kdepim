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

#include "addresseeeditorwidget.h"
#include "core.h"
#include "kabprefs.h"

#include "extensionmanager.h"

ExtensionManager::ExtensionManager( KAB::Core *core, QWidget *parent,
                                    const char *name )
  : QHBox( parent, name ), mCore( core ), mCurrentExtensionWidget( 0 )
{
  mActionExtensions = new KSelectAction( i18n( "Show Extension Bar" ), 0,
                                         mCore->actionCollection(),
                                         "options_show_extensions" );

  connect( mActionExtensions, SIGNAL( activated( int ) ),
           SLOT( setActiveExtension( int ) ) );

  createExtensionWidgets();
}

ExtensionManager::~ExtensionManager()
{
}

void ExtensionManager::restoreSettings()
{
  mActionExtensions->setCurrentItem( KABPrefs::instance()->mCurrentExtension );
  setActiveExtension( mActionExtensions->currentItem() );
}

void ExtensionManager::saveSettings()
{
  KABPrefs::instance()->mCurrentExtension = mActionExtensions->currentItem();
}

void ExtensionManager::reconfigure()
{
  saveSettings();
  createExtensionWidgets();
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

    mCurrentExtensionWidget = mExtensionWidgetList.at( id - 1 );
    if ( mCurrentExtensionWidget ) {
      show();
      mCurrentExtensionWidget->show();
    } else {
      hide();
      mCurrentExtensionWidget = 0;
    }
  }
}

void ExtensionManager::createExtensionWidgets()
{
  // clear extension widget list
  mExtensionWidgetList.setAutoDelete( true );
  QPtrListIterator<KAB::ExtensionWidget> wdgIt( mExtensionWidgetList );
  KAB::ExtensionWidget *wdg = 0;
  while ( ( wdg = wdgIt.current() ) != 0 )
    mExtensionWidgetList.remove( wdg );

  mExtensionWidgetList.setAutoDelete( false );

  QStringList extensionNames( i18n( "None" ) );

  // add addressee editor as default
  wdg = new AddresseeEditorWidget( mCore, true, this );
  wdg->hide();
  connect( wdg, SIGNAL( modified( const KABC::Addressee::List& ) ),
           SIGNAL( modified( const KABC::Addressee::List& ) ) );
  mExtensionWidgetList.append( wdg );
  extensionNames.append( wdg->title() );

  // load the other extensions
  QStringList activeExtensions = KABPrefs::instance()->mActiveExtensions;

  KTrader::OfferList plugins = KTrader::self()->query( "KAddressBook/Extension" );
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

    if ( !activeExtensions.contains( extensionFactory->identifier() ) )
      continue;

    wdg = extensionFactory->extension( mCore, this );
    if ( wdg ) {
      wdg->hide();
      connect( wdg, SIGNAL( modified( const KABC::Addressee::List& ) ),
               SIGNAL( modified( const KABC::Addressee::List& ) ) );
      mExtensionWidgetList.append( wdg );
      extensionNames.append( wdg->title() );
    }
  }

  mActionExtensions->setItems( extensionNames );
  mCurrentExtensionWidget = 0;
}

#include "extensionmanager.moc"
