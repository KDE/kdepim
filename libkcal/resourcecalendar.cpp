/*
    This file is part of libkdepim.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2001-2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include "calendar.h"

#include "resourcecalendar.h"

using namespace KCal;

ResourceCalendar::ResourceCalendar( const KConfig *config )
    : KRES::Resource( config )
{
}

ResourceCalendar::~ResourceCalendar()
{
}

void ResourceCalendar::writeConfig( KConfig* config )
{
//  kdDebug(5800) << "ResourceCalendar::writeConfig()" << endl;

  KRES::Resource::writeConfig( config );
}

bool ResourceCalendar::addIncidence( Incidence *incidence )
{
  Incidence::AddVisitor<ResourceCalendar> v( this );
  return incidence->accept( v );
}

Incidence::List ResourceCalendar::rawIncidences()
{
  return Calendar::mergeIncidenceList( rawEvents(), rawTodos(), journals() );
}

void ResourceCalendar::setSubresourceActive( const QString &, bool )
{
}

bool ResourceCalendar::load()
{
  kdDebug(5800) << "Loading resource " + resourceName() << endl;

  // FIXME: test if resource is opened and remove this test from concrete
  // resources

  mReceivedLoadError = false;

  bool success = true;
  if ( !isOpen() ) success = open();
  if ( success ) {
    success = doLoad();
  }
  if ( !success && !mReceivedLoadError ) loadError();

  kdDebug(5800) << "Done loading resource " + resourceName() << endl;

  return success;
}

void ResourceCalendar::loadError( const QString &err )
{
  kdDebug(5800) << "Error loading resource: " << err << endl;

  mReceivedLoadError = true;

  QString msg = i18n("Error while loading %1.\n") .arg( resourceName() );
  if ( !err.isEmpty() ) {
    msg += err;
  }
  emit resourceLoadError( this, msg );
}

bool ResourceCalendar::save()
{
  kdDebug(5800) << "Save resource " + resourceName() << endl;

  mReceivedSaveError = false;

  bool success = doSave();
  if ( !success && !mReceivedSaveError ) saveError();

  return success;
}

void ResourceCalendar::saveError( const QString &err )
{
  kdDebug(5800) << "Error saving resource: " << err << endl;

  mReceivedSaveError = true;

  QString msg = i18n("Error while saving %1.\n") .arg( resourceName() );
  if ( !err.isEmpty() ) {
    msg += err;
  }
  emit resourceSaveError( this, msg );
}


#include "resourcecalendar.moc"
