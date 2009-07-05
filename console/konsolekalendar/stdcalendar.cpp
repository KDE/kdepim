/*
    This file was originally from libkcal, then moved into korganizer.
    This version has been hacked for use by konsolekalendar.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2005,2008-2009 Allen Winter <winter@kde.org>

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
/**
 * @file stdcalendar.cpp
 * Provides a class for Calendar Resources.
 * @author Cornelius Schumacher
 * @author Allen Winter
 */
#include "stdcalendar.h"

#include <kconfig.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kurl.h>

#include <kcal/resourcecalendar.h>

#include "libkdepim/kpimprefs.h"

using namespace KCal;

StdCalendar::StdCalendar( const QString &fileName, const QString &resName )
  : CalendarResources( KPIM::KPimPrefs::timeSpec() )
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

StdCalendar::StdCalendar( bool allowGui )
  : CalendarResources( KPIM::KPimPrefs::timeSpec() )
{
  readConfig();

  mManager = resourceManager();

  // By default, only permit resource types "file" and "localdir".
  // Other resource types (eg. "kolab") require an X server and
  // interaction with KMail or login/password prompts, etc.
  // More types can be added as long as we are very sure they
  // can be run ok in a non-GUI environment, like cron.
  // See X-KDE-ResourceType in the resource .desktop file.
  if ( allowGui ) {
    KCal::CalendarResourceManager::Iterator it;
    for ( it = mManager->begin(); it != mManager->end(); ++it ) {
      (*it)->load();
    }
  } else {
    KCal::CalendarResourceManager::Iterator it;
    for ( it = mManager->begin(); it != mManager->end(); ++it ) {
      if ( (*it)->type() == QLatin1String( "file" ) ||
           (*it)->type() == QLatin1String( "localdir" ) ) {
        (*it)->load();
      }
    }
  }

  if ( mManager->isEmpty() ) {
    KConfig _config( "korganizerrc" );
    KConfigGroup config(&_config, "General" );
    QString fileName = config.readPathEntry( "Active Calendar", QString() );

    if ( !fileName.isEmpty() ) {
      addFileResource( fileName, i18n( "Active Calendar" ) );

    } else {
      // No resource created, i.e. no path found in config => use default path
      addFileResource( KStandardDirs::locateLocal( "data", "korganizer/std.ics" ),
                                                   i18n( "Default Calendar" ) );
    }
  }
}

void StdCalendar::addFileResource( const QString &fileName,
                                   const QString &resName )
{
  KCal::ResourceCalendar *resource = 0;

  if ( !fileName.isEmpty() ) {
    KUrl url( fileName );
    if ( url.isLocalFile() ) {
      kDebug() << "Local resource at" << url;
      resource = mManager->createResource( "file" );
      if ( resource ) {
        resource->setValue( "File", url.toLocalFile() );
      }
    } else {
      kDebug() << "Remote Resource at" << url;
      resource = mManager->createResource( "remote" );
      if ( resource ) {
        resource->setValue( "URL", url.url() );
      }
    }

    if ( resource ) {
      resource->setTimeSpec( KPIM::KPimPrefs::timeSpec() );
      resource->setResourceName( resName );
      mManager->add( resource );
      mManager->setStandardResource( resource );
    }
  }
}

StdCalendar::~StdCalendar()
{
}
