/*
    $Id$
    This file is part of libkcal.
    Copyright (c) 2002 Klarälvdalens Datakonsult AB <info@klaralvdalens-datakonsult.se>

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

#include "calendarimap.h"
#include "calendarimap.moc"

#include "calendarlocal.h"
// Just for some enums
#include "scheduler.h"

#include "journal.h"

#include <kdebug.h>
#include <kprocess.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kinstance.h>
#include <kglobal.h>
#include <dcopclient.h>
#include <qfile.h>
#include <qdatastream.h>
#include <stdlib.h>

using namespace KCal;

CalendarIMAP::CalendarIMAP( const QString& organizerEmail ) : mTarget(0)
{
  mLocalCalendar = new CalendarLocal();
  mOrganizerEmail = organizerEmail;
}


CalendarIMAP::CalendarIMAP( const QString& organizerEmail, const QString& timeZoneId )
  : Calendar( timeZoneId ), mTarget(0)
{
  mLocalCalendar = new CalendarLocal( timeZoneId );
  mOrganizerEmail = organizerEmail;
}


CalendarIMAP::~CalendarIMAP()
{
  delete mLocalCalendar;
}


void CalendarIMAP::setTarget( QObject* target )
{
  if( mTarget ) {
    // Disconnect the old one
    disconnect( this, SIGNAL( signalNewOrUpdatedIncident( const QString&,
							  const QString&,
							  const QString&,
							  const QStringList&,
							  const QString& ) ) );
  }

  mTarget = target;

  QObject::connect( this,
		    SIGNAL( signalNewOrUpdatedIncident( const QString&,
							const QString&,
							const QString&,
							const QStringList&,
							const QString& ) ),
		    mTarget,
		    SLOT( slotNewOrUpdatedIncident( const QString&,
						    const QString&,
						    const QString&,
						    const QStringList&,
						    const QString& ) ) );
}

void CalendarIMAP::close()
{
  if( mLocalCalendar )
    mLocalCalendar->close();
}


void CalendarIMAP::addEvent( Event* aEvent )
{

  mLocalCalendar->addEvent( aEvent );

  // Tell mail client about the change
  sendNewOrUpdatedIncident( "Calendar", aEvent );

  setupAlarm();
}


void CalendarIMAP::deleteEvent( Event* aEvent )
{
  // Tell mail client about the deletion
  sendNewOrUpdatedIncident( "Calendar", aEvent, true );
  mLocalCalendar->deleteEvent( aEvent );

  setupAlarm();
}


Event* CalendarIMAP::event( const QString& uniqueStr )
{
  return mLocalCalendar->event( uniqueStr );
}


QPtrList<Event> CalendarIMAP::events()
{
  return mLocalCalendar->events();
}


QPtrList<Event> CalendarIMAP::rawEvents()
{
  return mLocalCalendar->rawEvents();
}


int CalendarIMAP::numEvents( const QDate& qd )
{
  return mLocalCalendar->numEvents( qd );
}


void CalendarIMAP::addTodo( Todo* aTodo )
{
  mLocalCalendar->addTodo( aTodo );

  // Tell mail client about the change
  sendNewOrUpdatedIncident( "Task", aTodo );

  setModified( true );

  setupAlarm();
}


void CalendarIMAP::deleteTodo( Todo* aTodo )
{
  // Tell mail client about the deletion
  sendNewOrUpdatedIncident( "Task", aTodo, true );

  mLocalCalendar->deleteTodo( aTodo );

  setModified( true );

  setupAlarm();
}


QPtrList<Todo> CalendarIMAP::todos()
{
  return mLocalCalendar->todos();
}


Todo* CalendarIMAP::todo( const QString& uid )
{
  return mLocalCalendar->todo( uid );
}


QPtrList<Todo> CalendarIMAP::todos( const QDate& date )
{
  return mLocalCalendar->todos( date );
}


QPtrList<Todo> CalendarIMAP::rawTodos() const
{
  return mLocalCalendar->rawTodos();
}


void CalendarIMAP::addJournal( Journal* journal )
{
  mLocalCalendar->addJournal( journal );

  setupAlarm();
}


Journal* CalendarIMAP::journal( const QDate& date )
{
  return mLocalCalendar->journal( date );
}


Journal* CalendarIMAP::journal( const QString& UID )
{
  return mLocalCalendar->journal( UID );
}


QPtrList<Journal> CalendarIMAP::journals()
{
  return mLocalCalendar->journals();
}


Alarm::List CalendarIMAP::alarms( const QDateTime &from,
                                  const QDateTime &to )
{
  return mLocalCalendar->alarms( from, to );
}

// after changes are made to an event, this should be called.
void CalendarIMAP::update( IncidenceBase *incidence )
{
  mLocalCalendar->update( incidence );
}

QPtrList<Event> CalendarIMAP::rawEventsForDate( const QDateTime &qdt )
{
  return mLocalCalendar->rawEventsForDate( qdt );
}


QPtrList<Event> CalendarIMAP::rawEventsForDate( const QDate &date,
                                                bool sorted )
{
  return mLocalCalendar->rawEventsForDate( date, sorted );
}


QPtrList<Event> CalendarIMAP::rawEvents( const QDate &start, const QDate &end,
                                         bool inclusive )
{
  return mLocalCalendar->rawEvents( start, end, inclusive );
}


/*!
  This method passes a new or updated event that has been created in
  KOrganizer to KMail for storage or sending to the recipients.
*/

void CalendarIMAP::sendNewOrUpdatedIncident( const QString& type,
                                             Incidence* incidence,
                                             bool eventDeleted )
{
  mFormat.setTimeZone( timeZoneId(), !isLocalTime() );

  // The method is always Request, never Refresh. At least, that's
  // what Outlook does.
  QString messageText( mFormat.createScheduleMessage(incidence,
						     eventDeleted ?
						     Scheduler::Cancel :
						     Scheduler::Request) );
  QString uid( incidence->uid() );
  if( eventDeleted )
    uid.prepend("DELETE ME:");

  QPtrList<Attendee> attendees = incidence->attendees();
  QStringList attendeeMailAddresses;
  for( Attendee* attendee = attendees.first(); attendee;
       attendee = attendees.next() ) {
    // Omit the organizer, so that we don't send an email to ourselves
    if( attendee->email() != mOrganizerEmail )
      attendeeMailAddresses.append( attendee->email() );
  }

  emit signalNewOrUpdatedIncident( type, messageText, uid,
				   attendeeMailAddresses,
				   incidence->summary() );
}


/*!
  Loads calendar data into the local calendar.
  PENDING(kalle) This needs to go back into KMail.
*/
bool CalendarIMAP::load( const QString& filename )
{
  return mLocalCalendar->load( filename );
}


/*!
  Saves the calendar data in the local calendar into a file.
*/

bool CalendarIMAP::save( const QString& filename, CalFormat* format )
{
  return mLocalCalendar->save( filename, format );
}


void CalendarIMAP::setupAlarm()
{
  static bool isInSetupAlarm = false;

  if( isInSetupAlarm )
    return;
  isInSetupAlarm = true;

  // Store the calendar in a file
  QString calFile = locateLocal( "appdata", "kmailalarms.ics" );

  if( save( calFile ) ) {
    DCOPClient* dcopClient = kapp->dcopClient();
    QByteArray params;
    QDataStream ds( params, IO_WriteOnly );

    QByteArray params2;
    QDataStream ds2( params2, IO_WriteOnly );
    ds2 << QCString( "korgac" ) << calFile;
    if( !dcopClient->send( "kalarmd", "ad", "reloadCal(QCString,QString)", params2 ) )
      kdDebug() << "Could not send reloadCal() DCOP call to kalarmd" << endl;
    else
      kdDebug() << "Send reloadCal() to kalarmd" << endl;
  }
  isInSetupAlarm = false;
}
