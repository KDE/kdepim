/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001-2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include "calendar.h"

#include "resourcecalendar.h"

using namespace KCal;

ResourceCalendar::ResourceCalendar( const KConfig *config )
  : KRES::Resource( config ), mResolveConflict( false )
{
  mException = 0;
}

ResourceCalendar::~ResourceCalendar()
{
  delete mException;
}

void ResourceCalendar::clearException()
{
  delete mException;
  mException = 0;
}

void ResourceCalendar::setException( ErrorFormat *exception )
{
  delete mException;
  mException = exception;
}

ErrorFormat *ResourceCalendar::exception()
{
  return mException;
}

void ResourceCalendar::setResolveConflict( bool b)
{
 mResolveConflict = b;
}

TQString ResourceCalendar::infoText() const
{
  TQString txt;

  txt += "<b>" + resourceName() + "</b>";
  txt += "<br>";

  KRES::Factory *factory = KRES::Factory::self( "calendar" );
  TQString t = factory->typeName( type() );
  txt += i18n("Type: %1").arg( t );

  addInfoText( txt );

  return txt;
}

void ResourceCalendar::writeConfig( KConfig* config )
{
//  kdDebug(5800) << "ResourceCalendar::writeConfig()" << endl;

  KRES::Resource::writeConfig( config );
}

Incidence *ResourceCalendar::incidence( const TQString &uid )
{
  Incidence *i = event( uid );
  if ( i ) return i;
  i = todo( uid );
  if ( i ) return i;
  i = journal( uid );
  return i;
}

bool ResourceCalendar::addIncidence( Incidence *incidence )
{
  Incidence::AddVisitor<ResourceCalendar> v( this );
  return incidence->accept( v );
}

bool ResourceCalendar::addIncidence( Incidence *incidence, const TQString &subresource )
{
  Incidence::AddSubResourceVisitor<ResourceCalendar> v( this, subresource );
  return incidence->accept( v );
}

bool ResourceCalendar::deleteIncidence( Incidence *incidence )
{
  Incidence::DeleteVisitor<ResourceCalendar> v( this );
  return incidence->accept( v );
}

Incidence::List ResourceCalendar::rawIncidences()
{
  return Calendar::mergeIncidenceList( rawEvents(), rawTodos(), rawJournals() );
}

void ResourceCalendar::setSubresourceActive( const TQString &, bool )
{
}

bool ResourceCalendar::addSubresource( const TQString &, const TQString & )
{
  return true;
}

bool ResourceCalendar::removeSubresource( const TQString & )
{
  return true;
}

bool ResourceCalendar::load()
{
  kdDebug(5800) << "Loading resource " + resourceName() << endl;

  mReceivedLoadError = false;

  bool success = true;
  if ( !isOpen() )
    success = open();
  if ( success )
    success = doLoad();

  if ( !success && !mReceivedLoadError )
    loadError();

  // If the resource is read-only, we need to set its incidences to read-only,
  // too. This can't be done at a lower-level, since the read-only setting
  // happens at this level
  if ( readOnly() ) {
    Incidence::List incidences( rawIncidences() );
    Incidence::List::Iterator it;
    for ( it = incidences.begin(); it != incidences.end(); ++it ) {
      (*it)->setReadOnly( true );
    }
  }

  kdDebug(5800) << "Done loading resource " + resourceName() << endl;

  return success;
}

void ResourceCalendar::loadError( const TQString &err )
{
  kdDebug(5800) << "Error loading resource: " << err << endl;

  mReceivedLoadError = true;

  TQString msg = i18n("Error while loading %1.\n") .arg( resourceName() );
  if ( !err.isEmpty() ) {
    msg += err;
  }
  emit resourceLoadError( this, msg );
}

bool ResourceCalendar::save( Incidence *incidence )
{
  if ( !readOnly() ) {
    kdDebug(5800) << "Save resource " + resourceName() << endl;

    mReceivedSaveError = false;

    if ( !isOpen() ) return true;
    bool success = incidence ? doSave(incidence) : doSave();
    if ( !success && !mReceivedSaveError ) saveError();

    return success;
  } else {
    // Read-only, just don't save...
    kdDebug(5800) << "Don't save read-only resource " + resourceName() << endl;
    return true;
  }
}

bool ResourceCalendar::doSave( Incidence * )
{
  return doSave();
}

void ResourceCalendar::saveError( const TQString &err )
{
  kdDebug(5800) << "Error saving resource: " << err << endl;

  mReceivedSaveError = true;

  TQString msg = i18n("Error while saving %1.\n") .arg( resourceName() );
  if ( !err.isEmpty() ) {
    msg += err;
  }
  emit resourceSaveError( this, msg );
}

bool ResourceCalendar::setValue( const TQString &key, const TQString &value )
{
  Q_UNUSED( key );
  Q_UNUSED( value );
  return false;
}

TQString ResourceCalendar::subresourceType( const TQString &resource )
{
  Q_UNUSED( resource );
  return TQString();
}

bool ResourceCalendar::subresourceWritable( const TQString &resource ) const
{
  if ( resource.isEmpty() ) {
    return !readOnly();
  } else {
    return false;
  }
}

void ResourceCalendar::beginAddingIncidences()
{
}

void ResourceCalendar::endAddingIncidences()
{
}

#include "resourcecalendar.moc"
