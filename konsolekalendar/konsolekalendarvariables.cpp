/*******************************************************************************
 * konsolekalendarvariables.cpp                                                *
 *                                                                             *
 * KonsoleKalendar is a command line interface to KDE calendars                *
 * Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>             *
 * Copyright (C) 2003-2005  Allen Winter <winter@kde.org>                      *
 *                                                                             *
 * This program is free software; you can redistribute it and/or modify        *
 * it under the terms of the GNU General Public License as published by        *
 * the Free Software Foundation; either version 2 of the License, or           *
 * (at your option) any later version.                                         *
 *                                                                             *
 * This program is distributed in the hope that it will be useful,             *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                *
 * GNU General Public License for more details.                                *
 *                                                                             *
 * You should have received a copy of the GNU General Public License           *
 * along with this program; if not, write to the Free Software                 *
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. *
 *                                                                             *
 * As a special exception, permission is given to link this program            *
 * with any edition of Qt, and distribute the resulting executable,            *
 * without including the source code for Qt in the source distribution.        *
 *                                                                             *
 ******************************************************************************/
/**
 * @file konsolekalendarvariables.cpp
 * Provides the KonsoleKalendarVariables class definition.
 * @author Tuukka Pasanen
 * @author Allen Winter
 */
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kpimprefs.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "konsolekalendarvariables.h"

using namespace KCal;
using namespace std;

KonsoleKalendarVariables::KonsoleKalendarVariables()
{
  m_TimeZoneId = "";
  m_bIsTimeZoneId = false;
  m_bIsUID = false;
  m_bIsStartDateTime = false;
  m_bIsEndDateTime = false;
  m_bNext = false;
  m_bVerbose = false;
  m_bDryRun = false;
  m_bUseEvents = false;
  m_bUseTodos = false;
  m_bUseJournals = false;
  m_bDescription = false;
  m_description = "";
  m_bSummary = false;
  m_summary = "Default summary";
  m_bFloating = true;
  m_exportType = ExportTypeText;
  m_bIsExportFile = false;
  m_bIsDefault = false;
  m_bIsCalendarResources = false;
}

void KonsoleKalendarVariables::setTimeZoneId()
{
  m_bIsTimeZoneId = true;
  m_TimeZoneId = KPimPrefs::timezone();
}

QString KonsoleKalendarVariables::getTimeZoneId()
{
  return m_TimeZoneId;
}

bool KonsoleKalendarVariables::isTimeZoneId()
{
  return m_bIsTimeZoneId;
}

KonsoleKalendarVariables::~KonsoleKalendarVariables()
{
  // delete m_resource;
}

void KonsoleKalendarVariables::setUID( QString uid )
{
  m_bIsUID = true;
  m_UID = uid;
}

QString KonsoleKalendarVariables::getUID()
{
  return m_UID;
}

bool KonsoleKalendarVariables::isUID()
{
  return m_bIsUID;
}

void KonsoleKalendarVariables::setStartDateTime( QDateTime start )
{
  m_bIsStartDateTime = true;
  m_startDateTime = start;
}

QDateTime KonsoleKalendarVariables::getStartDateTime()
{
  return m_startDateTime;
}

bool KonsoleKalendarVariables::isStartDateTime()
{
  return m_bIsStartDateTime;
}

void KonsoleKalendarVariables::setEndDateTime( QDateTime end )
{
  m_bIsEndDateTime = true;
  m_endDateTime = end;
}

QDateTime KonsoleKalendarVariables::getEndDateTime()
{
  return m_endDateTime;
}

bool KonsoleKalendarVariables::isEndDateTime()
{
  return m_bIsEndDateTime;
}

void KonsoleKalendarVariables::setNext( bool next )
{
  m_bNext = next;
}

bool KonsoleKalendarVariables::isNext()
{
  return m_bNext;
}

void KonsoleKalendarVariables::setVerbose( bool verbose )
{
  m_bVerbose = verbose;
}

bool KonsoleKalendarVariables::isVerbose()
{
  return m_bVerbose;
}

void KonsoleKalendarVariables::setDryRun( bool dryrun )
{
  m_bDryRun = dryrun;
}

bool KonsoleKalendarVariables::isDryRun()
{
  return m_bDryRun;
}

void KonsoleKalendarVariables::setUseEvents( bool useEvents )
{
  m_bUseEvents = useEvents;
}

bool KonsoleKalendarVariables::getUseEvents()
{
  return( m_bUseEvents );
}

void KonsoleKalendarVariables::setUseTodos( bool useTodos )
{
  m_bUseTodos = useTodos;
}

bool KonsoleKalendarVariables::getUseTodos()
{
  return( m_bUseTodos );
}

void KonsoleKalendarVariables::setUseJournals( bool useJournals )
{
  m_bUseJournals = useJournals;
}

bool KonsoleKalendarVariables::getUseJournals()
{
  return( m_bUseJournals );
}

void KonsoleKalendarVariables::setCalendarFile( QString calendar )
{
  m_calendar = calendar;
}

QString KonsoleKalendarVariables::getCalendarFile()
{
  return m_calendar;
}

void KonsoleKalendarVariables::setImportFile( QString calendar )
{
  m_import = calendar;
}

QString KonsoleKalendarVariables::getImportFile()
{
  return m_import;
}

void KonsoleKalendarVariables::setCalendar( CalendarLocal *calendar )
{
  m_calendarLocal = calendar;
}

CalendarLocal *KonsoleKalendarVariables::getCalendar()
{
  return m_calendarLocal;
}

void KonsoleKalendarVariables::setExportType( ExportType exportType )
{
  m_exportType = exportType;
}

ExportType KonsoleKalendarVariables::getExportType()
{
  return m_exportType;
}

void KonsoleKalendarVariables::setExportFile( QString export_file )
{
  m_exportFile = export_file;
  m_bIsExportFile = true;
}

bool KonsoleKalendarVariables::isExportFile()
{
  return m_bIsExportFile;
}

QString KonsoleKalendarVariables::getExportFile()
{
  return m_exportFile;
}

bool KonsoleKalendarVariables::isAll()
{
  return m_bAll;
}

void KonsoleKalendarVariables::setAll( bool all )
{
  m_bAll = all;
}

bool KonsoleKalendarVariables::getAll()
{
  return m_bAll;
}

void KonsoleKalendarVariables::setDefault( bool def )
{
  m_bIsDefault = def;
}

bool KonsoleKalendarVariables::isDefault()
{
  return m_bIsDefault;
}

void KonsoleKalendarVariables::setDescription( QString description )
{
  m_bDescription = true;
  m_description = description;
}

QString KonsoleKalendarVariables::getDescription()
{
  return m_description;
}

bool KonsoleKalendarVariables::isDescription()
{
  return m_bDescription;
}

void KonsoleKalendarVariables::setLocation( QString location )
{
  m_bLocation = true;
  m_location = location;
}

QString KonsoleKalendarVariables::getLocation()
{
  return m_location;
}

bool KonsoleKalendarVariables::isLocation()
{
  return m_bLocation;
}

void KonsoleKalendarVariables::setSummary( QString summary )
{
  m_bSummary = true;
  m_summary = summary;
}

QString KonsoleKalendarVariables::getSummary()
{
  return m_summary;
}

bool KonsoleKalendarVariables::isSummary()
{
  return m_bSummary;
}

void KonsoleKalendarVariables::setFloating( bool floating )
{
  m_bFloating = floating;
}

bool KonsoleKalendarVariables::getFloating()
{
  return m_bFloating;
}

void KonsoleKalendarVariables::setDaysCount( int count )
{
  m_daysCount = count;
  m_bDaysCount = true;
}

int KonsoleKalendarVariables::getDaysCount()
{
  return m_daysCount;
}

bool KonsoleKalendarVariables::isDaysCount()
{
  return m_bDaysCount;
}

bool KonsoleKalendarVariables::addCalendarResources( ResourceCalendar
                                                     *resource )
{
  if ( m_resource ) {
    // In current state we support only one calendar
    // that's a fact and we have to live with that!
    kdDebug() << "konsolekalendarvariables.cpp::addCalendarResources() | "
              << "Add to calendar resource!"
              << endl;

    CalendarResourceManager::ActiveIterator it;
    CalendarResourceManager *manager = getCalendarResourceManager();
    QString fileName = NULL;

    for ( it = manager->activeBegin(); it != manager->activeEnd(); ++it ) {
      kdDebug() << "Resource name: " + (*it)->resourceName()
                << endl;

      if ( !strcmp( (*it)->resourceName().local8Bit(),
                    getCalendarFile().local8Bit() ) ) {
	kdDebug() << "konsolekalendarvariables.cpp::addCalendarResources() | "
                  << "We allready have this resource"
                  << endl;
        return true;
      }

    }

    manager->add( resource );

    if ( isDefault() ) {
      kdDebug() << "konsolekalendarvariables.cpp::addCalendarResources() | "
                << "Make it default"
                << endl;
      manager->setStandardResource( resource );
    }

  } else {
    kdDebug() << "konsolekalendarvariables.cpp::addCalendarResources() | "
              << "Cannot add to calendar resources (Not created!)"
              << endl;
    return false;
  }

  return true;
}

bool KonsoleKalendarVariables::isCalendarResources()
{
  return m_bIsCalendarResources;
}

void KonsoleKalendarVariables::setCalendarResources( CalendarResources
                                                     *resource )
{
  m_resource = resource;
  setCalendar( (CalendarLocal *) m_resource );
  m_bIsCalendarResources = true;
}

CalendarResources *KonsoleKalendarVariables::getCalendarResources()
{
  return m_resource;
}

CalendarResourceManager *KonsoleKalendarVariables::getCalendarResourceManager( )
{
  return m_resource->resourceManager();
}

bool KonsoleKalendarVariables::loadCalendarResources( KConfig *config )
{

  if ( m_resource ) {
    kdDebug() << "konsolekalendarvariables.cpp::loadCalendarResources() | "
              << "loading resources"
              << endl;

    CalendarResourceManager *manager = m_resource->resourceManager();

    if ( manager->isEmpty() == true ) {

      config->setGroup( "General" );
      QString fileName = config->readPathEntry( "Active Calendar" );

      QString resourceName;
      if ( fileName.isEmpty() ) {
        fileName = locateLocal( "appdata", "std.ics" );
        resourceName = i18n( "Default KOrganizer resource" );
      } else {
        resourceName = i18n( "Active Calendar" );
      }

      kdDebug() << "konsolekalendarvariables.cpp::loadCalendarResources() | "
                << "Using as default resource: '"
                << fileName
                << "'"
                << endl;

      ResourceCalendar *defaultResource = new ResourceLocal( fileName );
      //defaultResource->setTimeZoneId);
      defaultResource->setResourceName( resourceName );

      manager->add( defaultResource );
      manager->setStandardResource( defaultResource );
    }
  }

  return true;
}
