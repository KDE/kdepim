/*
    This file was originally from libkcal, then moved into korganizer.
    This version has been hacked for use by konsolekalendar.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2005 Allen Winter <winter@kde.org>

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

#include "stdcalendar.h"

#include <libkcal/resourcecalendar.h>
#include <libkdepim/kpimprefs.h>

#include <kconfig.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kurl.h>

using namespace KCal;

StdCalendar::StdCalendar( const QString &fileName, const QString &resName )
  : CalendarResources( KPimPrefs::timezone() )
{
  mManager = resourceManager();
  if ( mManager->isEmpty() ) {
    addFileResource( fileName, resName );
  } else {
    CalendarResourceManager::ActiveIterator it;
    for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
      (*it)->load();
    }
  }
}

StdCalendar::StdCalendar()
  : CalendarResources( KPimPrefs::timezone() )
{
  readConfig();

  mManager = resourceManager();
  if ( mManager->isEmpty() ) {
    KConfig config( "korganizerrc" );
    config.setGroup( "General" );
    QString fileName = config.readPathEntry( "Active Calendar" );

    if ( !fileName.isEmpty() ) {
      addFileResource( fileName, i18n( "Active Calendar" ) );

    } else {
      // No resource created, i.e. no path found in config => use default path
      addFileResource( locateLocal( "data", "korganizer/std.ics" ),
                                    i18n( "Default Calendar" ) );
    }
  }
}

void StdCalendar::addFileResource( const QString &fileName,
                                   const QString &resName )
{
  KCal::ResourceCalendar *resource = 0;

  if ( !fileName.isEmpty() ) {
    KURL url( fileName );
    if ( url.isLocalFile() ) {
      kdDebug() << "Local resource at " << url << endl;
      resource = mManager->createResource( "file" );
      if ( resource )
        resource->setValue( "File", url.path() );
    } else {
      kdDebug() << "Remote Resource at " << url << endl;
      resource = mManager->createResource( "remote" );
      if ( resource )
        resource->setValue( "URL", url.url() );
    }

    if ( resource ) {
      resource->setTimeZoneId( KPimPrefs::timezone() );
      resource->setResourceName( resName );
      mManager->add( resource );
      mManager->setStandardResource( resource );
    }
  }
}

StdCalendar::~StdCalendar()
{
}
