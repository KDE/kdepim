/*
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

#ifndef CALENDARIMAP_H
#define CALENDARIMAP_H

#include "calendar.h"
#include "icalformat.h"
#include <qobject.h>

class KProcess;

namespace KCal {

class CalendarLocal;

class CalendarIMAP : public QObject, public Calendar
{
  Q_OBJECT

public:
  CalendarIMAP( const QString& organizerEmail );
  CalendarIMAP( const QString& organizerEmail, const QString& timeZoneId );
  ~CalendarIMAP();

  bool load( const QString& filename );
  bool save( const QString& filename, CalFormat* format = 0 );

  virtual void close();

  virtual void addEvent( Event* aEvent );
  virtual void deleteEvent( Event* );
  virtual Event* event( const QString& uniqueStr );
  virtual QPtrList<Event> events();
  virtual QPtrList<Event> rawEvents();
  virtual int numEvents( const QDate& qd );

  virtual void addTodo( Todo* todo );
  virtual void deleteTodo( Todo* todo );
  virtual Todo* todo( const QString& uid );
  virtual QPtrList<Todo> todos();
  virtual QPtrList<Todo> todos( const QDate& date );
  virtual QPtrList<Todo> rawTodos() const;

  virtual void addJournal( Journal* );
  virtual Journal* journal( const QDate& );
  virtual Journal* journal( const QString& UID );
  virtual QPtrList<Journal> journals();

  virtual Alarm::List alarms( const QDateTime &from,
			      const QDateTime &to );

  void setTarget( QObject* target );

  /** this method should be called whenever a Event is modified directly
   * via it's pointer.  It makes sure that the calendar is internally
   * consistent. */
  void update(IncidenceBase *incidence);

signals:
  void signalNewOrUpdatedIncident( const QString& type,
				   const QString& vCal,
				   const QString& uid,
				   const QStringList& recipients,
				   const QString& subject );

protected:
  /** Notification function of IncidenceBase::Observer. */
  void incidenceUpdated( IncidenceBase *i ) { update( i ); }

  virtual QPtrList<Event> rawEventsForDate( const QDateTime &qdt );
  virtual QPtrList<Event> rawEventsForDate( const QDate &date,
					    bool sorted = false );
  virtual QPtrList<Event> rawEvents( const QDate &start, const QDate &end,
				     bool inclusive = false );

private:
  // Incidence initiated or changed by user in KO, passed to KM
  void sendNewOrUpdatedIncident( const QString& type, Incidence* incidence,
				 bool eventDeleted = false );
  void setupAlarm();

  // Disabled because of the dynamically allocated local calendar
  CalendarIMAP( const CalendarIMAP& );
  CalendarIMAP& operator=( const CalendarIMAP& );

  CalendarLocal* mLocalCalendar;
  QObject* mTarget;
  ICalFormat mFormat;
  QString mOrganizerEmail;
};

};

#endif
