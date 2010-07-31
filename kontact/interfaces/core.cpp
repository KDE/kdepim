/*
   This file is part of KDE Kontact.

   Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
   Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "core.h"

#include <kparts/part.h>
#include <kparts/componentfactory.h>
#include <kdebug.h>
#include <tqtimer.h>
#include <klocale.h>

using namespace Kontact;

class Core::Private
{
public:
  TQString lastErrorMessage;
};

Core::Core( TQWidget *parent, const char *name )
  : KParts::MainWindow( parent, name )
{
  d = new Private;
  TQTimer* timer = new TQTimer( this );
  mLastDate = TQDate::currentDate();
  connect(timer, TQT_SIGNAL( timeout() ), TQT_SLOT( checkNewDay() ) );
  timer->start( 1000*60 );
}

Core::~Core()
{
  delete d;
}

KParts::ReadOnlyPart *Core::createPart( const char *libname )
{
  kdDebug(5601) << "Core::createPart(): " << libname << endl;

  TQMap<TQCString,KParts::ReadOnlyPart *>::ConstIterator it;
  it = mParts.find( libname );
  if ( it != mParts.end() ) return it.data();

  kdDebug(5601) << "Creating new KPart" << endl;

  int error = 0;
  KParts::ReadOnlyPart *part =
      KParts::ComponentFactory::
          createPartInstanceFromLibrary<KParts::ReadOnlyPart>
              ( libname, this, 0, this, "kontact", TQStringList(), &error );

  KParts::ReadOnlyPart *pimPart = dynamic_cast<KParts::ReadOnlyPart*>( part );
  if ( pimPart ) {
    mParts.insert( libname, pimPart );
    TQObject::connect( pimPart, TQT_SIGNAL( destroyed( TQObject * ) ),
                      TQT_SLOT( slotPartDestroyed( TQObject * ) ) );
  } else {
    // TODO move to KParts::ComponentFactory
    switch( error ) {
    case KParts::ComponentFactory::ErrNoServiceFound:
      d->lastErrorMessage = i18n( "No service found" );
      break;
    case KParts::ComponentFactory::ErrServiceProvidesNoLibrary:
      d->lastErrorMessage = i18n( "Program error: the .desktop file for the service does not have a Library key." );
      break;
    case KParts::ComponentFactory::ErrNoLibrary:
      d->lastErrorMessage = KLibLoader::self()->lastErrorMessage();
      break;
    case KParts::ComponentFactory::ErrNoFactory:
      d->lastErrorMessage = i18n( "Program error: the library %1 does not provide a factory." ).arg( libname );
      break;
    case KParts::ComponentFactory::ErrNoComponent:
      d->lastErrorMessage = i18n( "Program error: the library %1 does not support creating components of the specified type" ).arg( libname );
      break;
    }
    kdWarning(5601) << d->lastErrorMessage << endl;
  }

  return pimPart;
}

void Core::slotPartDestroyed( TQObject * obj )
{
  // the part was deleted, we need to remove it from the part map to not return
  // a dangling pointer in createPart
  TQMap<TQCString, KParts::ReadOnlyPart*>::Iterator end = mParts.end();
  TQMap<TQCString, KParts::ReadOnlyPart*>::Iterator it = mParts.begin();
  for ( ; it != end; ++it ) {
    if ( it.data() == obj ) {
      mParts.remove( it );
      return;
    }
  }
}

void Core::checkNewDay()
{
  if ( mLastDate != TQDate::currentDate() )
    emit dayChanged( TQDate::currentDate() );

  mLastDate = TQDate::currentDate();
}

TQString Core::lastErrorMessage() const
{
  return d->lastErrorMessage;
}

#include "core.moc"
// vim: sw=2 sts=2 et tw=80
