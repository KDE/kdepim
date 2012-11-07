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

#include <libemailfunctions/email.h>

#include <kemailsettings.h>

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

bool CalHelper::hasMyWritableEventsFolders( const QString &family,
                                            CalendarResourceManager *manager )
{
  Q_ASSERT( manager );
  QString myfamily = family;
  if ( family.isEmpty() ) {
    myfamily = "calendar";
  }

  CalendarResourceManager::ActiveIterator it;

  for ( it=manager->activeBegin(); it != manager->activeEnd(); ++it ) {
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

bool CalHelper::hasWritableEventsFolders( const QString &family,
                                          CalendarResourceManager *manager )
{
  Q_ASSERT( manager );
  QString myfamily = family;
  if ( family.isEmpty() ) {
    myfamily = "calendar";
  }

  CalendarResourceManager::ActiveIterator it;

  for ( it=manager->activeBegin(); it != manager->activeEnd(); ++it ) {
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
             !(*it)->subresourceWritable( *subIt ) ) {
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

bool CalHelper::incOrganizerOwnsCalendar( Calendar *calendar, Incidence *incidence )
{
  if ( !calendar || !incidence ) {
    return false;
  }

  QPair<ResourceCalendar *, QString> p =  incSubResourceCalendar( calendar, incidence );
  ResourceCalendar *res = p.first;
  QString subRes = p.second;

  if ( !res ) {
    return false;
  }

  QString orgEmail;
  QString orgName;
  KPIM::getNameAndMail( incidence->organizer().email(), orgName, orgEmail );
  if ( KPIM::isValidEmailAddress( orgEmail ) != KPIM::AddressOk ) {
    return false;
  }

  // first determine if I am the organizer.
  bool iam = false;
  KEMailSettings settings;
  QStringList profiles = settings.profiles();
  for( QStringList::Iterator it=profiles.begin(); it!=profiles.end(); ++it ) {
    settings.setProfile( *it );
    if ( settings.getSetting( KEMailSettings::EmailAddress ) == orgEmail ) {
      iam = true;
      break;
    }
  }

  // if I am the organizer and the incidence is in my calendar
  if ( iam && isMyCalendarIncidence( calendar, incidence ) ) {
    // then we have a winner.
    return true;
  }

  // The organizer is not me.

  if ( ( res->type() == "imap" || res->type() == "kolab" ) && !subRes.isEmpty() ) {
    // KOLAB SPECIFIC:
    // Check if the organizer owns this calendar by looking at the
    // username part of the email encoded in the subresource name,
    // which is of the form "/.../.user.directory/.<name>.directory/..."
    const int atChar = orgEmail.findRev( '@' );
    const QString name = orgEmail.left( atChar );
    QString kolabFolder = "/.user.directory/." + name + ".directory/";
    if ( subRes.contains( kolabFolder ) ) {
      return true;
    }
    // if that fails, maybe the first name of the organizer email name will work
    const int dotChar = name.find( '.' );
    if ( dotChar > 0 ) {
      const QString firstName = name.left( dotChar );
      kolabFolder = "/.user.directory/." + firstName + ".directory/";
      if ( subRes.contains( kolabFolder ) ) {
        return true;
      }
    }
  }

  // TODO: support other resource types

  return false;
}
