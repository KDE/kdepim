/*
   This file is part of KDE Kontact.

   Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
   Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <tqobjectlist.h>

#include <dcopclient.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <kparts/componentfactory.h>
#include <kdebug.h>
#include <kinstance.h>
#include <krun.h>

#include "core.h"
#include "plugin.h"

using namespace Kontact;

class Plugin::Private
{
  public:
    Kontact::Core *core;
    DCOPClient *dcopClient;
    TQPtrList<KAction> *newActions;
    TQPtrList<KAction> *syncActions;
    TQString identifier;
    TQString title;
    TQString icon;
    TQString executableName;
    TQCString partLibraryName;
    bool hasPart;
    KParts::ReadOnlyPart *part;
    bool disabled;
};


Plugin::Plugin( Kontact::Core *core, TQObject *parent, const char *name )
  : KXMLGUIClient(  core ), TQObject(  parent, name ), d(  new Private )
{
  core->factory()->addClient( this );
  KGlobal::locale()->insertCatalogue(name);

  d->core = core;
  d->dcopClient = 0;
  d->newActions = new TQPtrList<KAction>;
  d->syncActions = new TQPtrList<KAction>;
  d->hasPart = true;
  d->part = 0;
  d->disabled = false;
}


Plugin::~Plugin()
{
  delete d->part;
  delete d->dcopClient;
  delete d;
}

void Plugin::setIdentifier( const TQString &identifier )
{
  d->identifier = identifier;
}

TQString Plugin::identifier() const
{
  return d->identifier;
}

void Plugin::setTitle( const TQString &title )
{
  d->title = title;
}

TQString Plugin::title() const
{
  return d->title;
}

void Plugin::setIcon( const TQString &icon )
{
  d->icon = icon;
}

TQString Plugin::icon() const
{
  return d->icon;
}

void Plugin::setExecutableName( const TQString& bin )
{
  d->executableName = bin;
}

TQString Plugin::executableName() const
{
  return d->executableName;
}

void Plugin::setPartLibraryName( const TQCString &libName )
{
  d->partLibraryName = libName;
}

KParts::ReadOnlyPart *Plugin::loadPart()
{
  return core()->createPart( d->partLibraryName );
}

const KAboutData *Plugin::aboutData()
{
  kdDebug(5601) << "Plugin::aboutData(): libname: " << d->partLibraryName << endl;

  const KInstance *instance =
    KParts::Factory::partInstanceFromLibrary( d->partLibraryName );

  if ( instance ) {
    return instance->aboutData();
  } else {
    kdError() << "Plugin::aboutData(): Can't load instance for "
              << title() << endl;
    return 0;
  }
}

KParts::ReadOnlyPart *Plugin::part()
{
  if ( !d->part ) {
    d->part = createPart();
    if ( d->part ) {
      connect( d->part, TQT_SIGNAL( destroyed() ), TQT_SLOT( partDestroyed() ) );
      core()->partLoaded( this, d->part );
    }
  }
  return d->part;
}

TQString Plugin::tipFile() const
{
  return TQString::null;
}


DCOPClient* Plugin::dcopClient() const
{
  if ( !d->dcopClient ) {
    d->dcopClient = new DCOPClient();
    // ### Note: maybe we could use executableName().latin1() instead here.
    // But this requires that dcopClient is NOT called by the constructor,
    // and is called by some new virtual void init() later on.
    d->dcopClient->registerAs( name(), false );
  }

  return d->dcopClient;
}

void Plugin::insertNewAction( KAction *action )
{
  d->newActions->append( action );
}

void Plugin::insertSyncAction( KAction *action )
{
  d->syncActions->append( action );
}

TQPtrList<KAction> *Plugin::newActions() const
{
  return d->newActions;
}

TQPtrList<KAction> *Plugin::syncActions() const
{
  return d->syncActions;
}

Kontact::Core *Plugin::core() const
{
  return d->core;
}

void Plugin::select()
{
}

void Plugin::configUpdated()
{
}

void Plugin::partDestroyed()
{
  d->part = 0;
}

void Plugin::slotConfigUpdated()
{
  configUpdated();
}

void Plugin::bringToForeground()
{
  if (!d->executableName.isEmpty())
    KRun::runCommand(d->executableName);
}

bool Kontact::Plugin::showInSideBar() const
{
  return d->hasPart;
}

void Kontact::Plugin::setShowInSideBar( bool hasPart )
{
  d->hasPart = hasPart;
}

void Kontact::Plugin::setDisabled( bool disabled )
{
    d->disabled = disabled;
}

bool Kontact::Plugin::disabled() const
{
    return d->disabled;
}

void Kontact::Plugin::loadProfile( const TQString& )
{
}

void Kontact::Plugin::saveToProfile( const TQString& ) const
{
}

void Plugin::virtual_hook( int, void* ) {
	//BASE::virtual_hook( id, data );
}

#include "plugin.moc"

// vim: sw=2 et sts=2 tw=80
