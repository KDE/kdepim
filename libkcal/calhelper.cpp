/*
  This file is part of the kcal library.

  Copyright (c) 2009-2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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
  @file
  This file is part of the API for handling calendar data and provides
  static convenience functions for making decisions about calendar data.

  @brief
  Provides methods for making decisions about calendar data.

  @author Allen Winter \<allen@kdab.net\>
*/

#include "calhelper.h"
#include "calendarresources.h"

using namespace KCal;

bool CalHelper::isMyKolabIncidence( Calendar *calendar, Incidence *incidence )
{
  CalendarResources *cal = dynamic_cast<CalendarResources*>( calendar );
  if ( !cal || !incidence ) {
    return true;
  }

  CalendarResourceManager *manager = cal->resourceManager();
  CalendarResourceManager::Iterator it;
  for ( it = manager->begin(); it != manager->end(); ++it ) {
    QString subRes = (*it)->subresourceIdentifier( incidence );
    if ( !subRes.isEmpty() && !subRes.contains( "/.INBOX.directory/" ) ) {
      return false;
    }
  }
  return true;
}

bool CalHelper::isMyCalendarIncidence( Calendar *calendar, Incidence *incidence )
{
  return isMyKolabIncidence( calendar, incidence );
}

Incidence *CalHelper::findMyCalendarIncidenceByUid( Calendar *calendar, const QString &uid )
{
  // Determine if this incidence is in my calendar (and owned by me)
  Incidence *existingIncidence = 0;
  if ( calendar ) {
    existingIncidence = calendar->incidence( uid );
    if ( !isMyCalendarIncidence( calendar, existingIncidence ) ) {
      existingIncidence = 0;
    }
    if ( !existingIncidence ) {
      const Incidence::List list = calendar->incidences();
      for ( Incidence::List::ConstIterator it = list.begin(), end = list.end(); it != end; ++it ) {
        if ( (*it)->schedulingID() == uid && isMyCalendarIncidence( calendar, *it ) ) {
          existingIncidence = *it;
          break;
        }
      }
    }
  }
  return existingIncidence;
}

bool CalHelper::usingGroupware( Calendar *calendar )
{
  CalendarResources *cal = dynamic_cast<CalendarResources*>( calendar );
  if ( !cal ) {
    return true;
  }

  CalendarResourceManager *manager = cal->resourceManager();
  CalendarResourceManager::Iterator it;
  for ( it = manager->begin(); it != manager->end(); ++it ) {
    QString res = (*it)->type();
    if ( res == "imap" ) {
      return true;
    }
  }
  return false;
}

bool CalHelper::hasMyWritableEventsFolders( const QString &family )
{
  QString myfamily = family;
  if ( family.isEmpty() ) {
    myfamily = "calendar";
  }

  CalendarResourceManager manager( myfamily );
  manager.readConfig();

  CalendarResourceManager::ActiveIterator it;
  for ( it=manager.activeBegin(); it != manager.activeEnd(); ++it ) {
    if ( (*it)->readOnly() ) {
      continue;
    }

    const QStringList subResources = (*it)->subresources();
    if ( subResources.isEmpty() ) {
      return true;
    }

    QStringList::ConstIterator subIt;
    for ( subIt=subResources.begin(); subIt != subResources.end(); ++subIt ) {
      if ( !(*it)->subresourceActive( (*subIt) ) ) {
        continue;
      }
      if ( (*it)->type() == "imap" || (*it)->type() == "kolab" ) {
        if ( (*it)->subresourceType( ( *subIt ) ) == "todo" ||
             (*it)->subresourceType( ( *subIt ) ) == "journal" ||
             !(*subIt).contains( "/.INBOX.directory/" ) ) {
          continue;
        }
      }
      return true;
    }
  }
  return false;
}

ResourceCalendar *CalHelper::incResourceCalendar( Calendar *calendar, Incidence *incidence )
{
  CalendarResources *cal = dynamic_cast<CalendarResources*>( calendar );
  if ( !cal || !incidence ) {
    return 0;
  }

  return cal->resource( incidence );
}

QPair<ResourceCalendar *, QString> CalHelper::incSubResourceCalendar( Calendar *calendar,
                                                                      Incidence *incidence )
{
  QPair<ResourceCalendar *, QString> p( 0, QString() );

  CalendarResources *cal = dynamic_cast<CalendarResources*>( calendar );
  if ( !cal || !incidence ) {
    return p;
  }

  ResourceCalendar *res = cal->resource( incidence );

  QString subRes;
  if ( res && res->canHaveSubresources() ) {
    subRes = res->subresourceIdentifier( incidence );
  }
  p = qMakePair( res, subRes );
  return p;
}
