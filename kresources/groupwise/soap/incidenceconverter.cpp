/*
    This file is part of kdepim.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#include "incidenceconverter.h"

#include <libkdepim/kpimprefs.h>

#include <kdebug.h>

IncidenceConverter::IncidenceConverter( struct soap* soap )
  : GWConverter( soap )
{
  mTimezone = KPimPrefs::timezone();
}

KCal::Event* IncidenceConverter::convertFromAppointment( ns1__Appointment* appointment )
{
  if ( !appointment )
    return 0;

  KCal::Event *event = new KCal::Event();

  if ( !convertFromCalendarItem( appointment, event ) ) {
    delete event;
    return 0;
  }

  if ( appointment->endDate != 0 )
    event->setDtEnd( charToQDateTime( appointment->endDate, mTimezone ) );

  if ( appointment->alarm ) {
    KCal::Alarm *alarm = event->newAlarm();
    alarm->setStartOffset( appointment->alarm->__item * -1 );
    alarm->setEnabled( appointment->alarm->enabled );
  }

  if ( appointment->allDayEvent && (*appointment->allDayEvent) == true )
    event->setFloats( true );
  else event->setFloats( false );

  if ( appointment->place )
    event->setLocation( stringToQString( appointment->place ) );

  return event;
}

ns1__Appointment* IncidenceConverter::convertToAppointment( KCal::Event* event )
{
  if ( !event )
    return 0;

  ns1__Appointment* appointment = soap_new_ns1__Appointment( soap(), -1 );

  if ( !convertToCalendarItem( event, appointment ) ) {
    soap_dealloc( soap(), appointment );
    return 0;
  }

  if ( event->hasEndDate() )
    appointment->endDate = qDateTimeToChar( event->dtEnd(), mTimezone );

  KCal::Alarm::List alarms = event->alarms();
  if ( !alarms.isEmpty() ) {
    ns1__Alarm* alarm = soap_new_ns1__Alarm( soap(), -1 );
    alarm->__item = alarms.first()->startOffset().asSeconds() * -1;
    alarm->enabled = alarms.first()->enabled();

    appointment->alarm = alarm;
  } else
    appointment->alarm = 0;

  if ( event->doesFloat() ) {
    bool *allDayEvent = (bool*)soap_malloc( soap(), 1 );
    (*allDayEvent ) = true;

    appointment->allDayEvent = allDayEvent;
  } else
    appointment->allDayEvent = 0;

  if ( !event->location().isEmpty() ) {
    std::string* location = qStringToString( event->location() );

    appointment->place = location;
  } else
    appointment->place = 0;

  appointment->timezone = 0;

  return appointment;
}

KCal::Todo* IncidenceConverter::convertFromTask( ns1__Task* task )
{
  if ( !task )
    return 0;

  KCal::Todo *todo = new KCal::Todo();

  if ( !convertFromCalendarItem( task, todo ) ) {
    delete todo;
    return 0;
  }

  if ( task->dueDate )
    todo->setDtDue( QDate::fromString( stringToQString( task->dueDate ), Qt::ISODate ) );

  if ( task->taskPriority ) {
    QString priority = stringToQString( task->taskPriority );

    // TODO: set priority
  }

  if ( task->completed && (*task->completed) == true )
    todo->setCompleted( true );

  return todo;
}

ns1__Task* IncidenceConverter::convertToTask( KCal::Todo* todo )
{
  if ( !todo )
    return 0;

  ns1__Task* task = soap_new_ns1__Task( soap(), -1 );

  if ( !convertToCalendarItem( todo, task ) ) {
    soap_dealloc( soap(), task );
    return 0;
  }

  if ( todo->hasDueDate() ) {
    task->dueDate = qDateTimeToChar( todo->dtDue() );
  } else
    task->dueDate = 0;

/*
  if ( task->taskPriority ) {
    QString priority = s2q( task->taskPriority );

    // TODO: set priority
  } else
*/
  task->taskPriority = 0;

  task->completed = (bool*)soap_malloc( soap(), 1 );
  if ( todo->isCompleted() )
    (*task->completed) = true;
  else
    (*task->completed) = false;

  return task;
}

bool IncidenceConverter::convertToCalendarItem( KCal::Incidence* incidence, ns1__CalendarItem* item )
{
  // null pointer initialization
  item->originalSubject = 0;
  item->distribution = 0;
  item->message = 0;
  item->attachments = 0;
  item->options = 0;
  item->status = 0;
  item->thread = 0;
  item->msgid = 0;
  item->source = 0;
  item->class_ = 0;
  item->categories = 0;
  item->customs = 0;
  item->changes = 0;
  item->type = 0;
  item->rdate = 0;
  item->isRecurring = 0;
  item->iCalId = 0;
  item->delivered = 0;

  item->id = incidence->customProperty( "GWRESOURCE", "UID" ).utf8();

  // Container
  if ( !incidence->customProperty( "GWRESOURCE", "CONTAINER" ).isEmpty() ) {
    std::vector<ns1__ContainerRef*>* container = soap_new_std__vectorTemplateOfPointerTons1__ContainerRef( soap(), -1 );
    ns1__ContainerRef* containerRef = soap_new_ns1__ContainerRef( soap(), -1 );
    containerRef->deleted = 0;
    containerRef->__item = incidence->customProperty( "GWRESOURCE", "CONTAINER" ).utf8();
    container->push_back( containerRef );

    item->container = container;
  } else
    item->container = 0;

  if ( !incidence->summary().isEmpty() )
    item->subject = std::string( incidence->summary().utf8() );

  if ( incidence->created().isValid() ) {
    item->created = qDateTimeToChar( incidence->created(), mTimezone );
  } else
    item->created = 0;

  if ( incidence->lastModified().isValid() )
    item->modified = qDateTimeToChar( incidence->lastModified(), mTimezone );

  if ( incidence->dtStart().isValid() )
    item->startDate = qDateTimeToChar( incidence->dtStart(), mTimezone );

  return true;
}

bool IncidenceConverter::convertFromCalendarItem( ns1__CalendarItem* item, KCal::Incidence* incidence )
{
  incidence->setCustomProperty( "GWRESOURCE", "UID",
                                stringToQString( item->id ) );

  if ( !item->subject.empty() )
    incidence->setSummary( stringToQString( item->subject ) );

//  kdDebug() << "SUMMARY: " << incidence->summary() << endl;

  if ( item->created )
    incidence->setCreated( charToQDateTime( item->created, mTimezone ) );

  if ( item->modified != 0 )
    incidence->setLastModified( charToQDateTime( item->modified, mTimezone ) );

  if ( item->startDate != 0 )
    incidence->setDtStart( charToQDateTime( item->startDate, mTimezone ) );
/*
  if ( item->rdate && item->rdate->date ) {
    std::vector<xsd__date>* dateList = item->rdate->date;

    std::vector<xsd__date>::const_iterator it;
    for ( it = dateList->begin(); it != dateList->end(); ++it ) {
      QDate date = QDate::fromString( s2q( *it ), Qt::ISODate );
      if ( date.isValid() )
    }
  }
*/
/*
    bool*                                isRecurring                    0;
    std::string*                         iCalId                         0;
*/

  return true;
}

#if 0
time_t IncidenceConverter::qDateTimeToGW( const QDateTime &dt )
{
  time_t ticks = -dt.secsTo( QDateTime( QDate( 1970, 1, 1 ), QTime( 0, 0 ) ) );

  return ticks;
}

time_t IncidenceConverter::qDateTimeToGW( const QDateTime &dt,
                                          const QString &timeZoneId )
{
  kdDebug() << "IncidenceConverter::qDateTimeToGW() " << timeZoneId << endl;

  kdDebug() << "  QDateTime: " << dt.toString() << endl;

  QDateTime utc = KPimPrefs::localTimeToUtc( dt, timeZoneId );

  kdDebug() << "  QDateTime UTC: " << utc.toString() << endl;

  time_t ticks = -utc.secsTo( QDateTime( QDate( 1970, 1, 1 ), QTime( 0, 0 ) ) );

  kdDebug() << "  Ticks: " << ticks << endl;

  return ticks;
}

QDateTime IncidenceConverter::gwToQDateTime( time_t ticks )
{
  QDateTime dt;
  dt.setTime_t( ticks, Qt::UTC );

  return dt;
}

QDateTime IncidenceConverter::gwToQDateTime( time_t ticks,
                                             const QString &timeZoneId )
{
  QDateTime dt;
  dt.setTime_t( ticks, Qt::UTC );

  return KPimPrefs::utcToLocalTime( dt, timeZoneId );
}
#endif
