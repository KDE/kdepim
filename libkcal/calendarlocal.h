/*
    This file is part of libkcal.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

// $Id$

#ifndef _CALENDARLOCAL_H
#define _CALENDARLOCAL_H

#include <qintdict.h>
#include <qmap.h>

#include "calendar.h"

#define BIGPRIME 1031 /* should allow for at least 4 appointments 365 days/yr
			 to be almost instantly fast. */

namespace KCal {

/**
    This class provides a calendar stored as a local file.
*/
class CalendarLocal : public Calendar {
    Q_OBJECT
  public:
    /** constructs a new calendar, with variables initialized to sane values. */
    CalendarLocal();
    /** constructs a new calendar, with variables initialized to sane values. */
    CalendarLocal(const QString &timeZoneId);
    virtual ~CalendarLocal();
  
    /**
      loads a calendar on disk in vCalendar or iCalendar format into the current calendar.
      any information already present is lost. Returns true if successful,
      else returns false.
      @param fileName the name of the calendar on disk.
    */
    bool load(const QString &fileName);
    /** writes out the calendar to disk in the specified \a format. Returns true if
     * successful and false on error.
     * @param fileName the name of the file
     */
    bool save(const QString &fileName,CalFormat *format=0);
    /** clears out the current calendar, freeing all used memory etc. etc. */
    void close();
  
    /** Add Event to calendar. */
    void addEvent(Event *anEvent);
    /** deletes an event from this calendar. */
    void deleteEvent(Event *);

    /** retrieves an event on the basis of the unique string ID. */
    Event *getEvent(const QString &UniqueStr);
    /** builds and then returns a list of all events that match for the
     * date specified. useful for dayView, etc. etc. */
    QPtrList<Event> eventsForDate(const QDate &date, bool sorted = FALSE);
    /** Get events for date \a qdt. */
    QPtrList<Event> eventsForDate(const QDateTime &qdt);
    /** Get events in a range of dates. If inclusive is set to true, only events
     * are returned, which are completely included in the range. */
    QPtrList<Event> events(const QDate &start,const QDate &end,
                             bool inclusive=false);
    /** Return all events in calendar */
    QPtrList<Event> getAllEvents();
    /** checks to see if any todos are due now, and if so, returns the list
     * of those todos that have alarms. */
    bool checkTodos(QPtrList<Todo> &, bool append = false);
    /** checks to see if any non-recurring alarms are due now, and if so,
     * returns the list of those events that have alarms. */
    bool checkNonRecurringAlarms(QPtrList<Event> &, bool append = false);
    /** checks to see if any recurring alarms are due now, and if so,
     * returns the list of those events that have alarms. */
    bool checkRecurringAlarms(QPtrList<Event> &, bool append = false);
    /** checks to see if any alarms are due now or have already passed, and
     * if so, returns the list of those events that have alarms.
     * Does not return recurring events. */
    bool checkAlarmsPast(QPtrList<Event> &, bool append = false);
  
    /*
      Returns a QString with the text of the holiday (if any) that falls
      on the specified date.
    */
    QString getHolidayForDate(const QDate &qd);
    
    /** returns the number of events that are present on the specified date. */
    int numEvents(const QDate &qd);
  
    /** add a todo to the todolist. */
    void addTodo(Todo *todo);
    /** remove a todo from the todolist. */
    void deleteTodo(Todo *);
    const QPtrList<Todo> &getTodoList() const;
    /** searches todolist for an event with this unique string identifier,
      returns a pointer or null. */
    Todo *getTodo(const QString &UniqueStr);
    /** Returns list of todos due on the specified date */
    QPtrList<Todo> getTodosForDate(const QDate & date);

    /** Add a Journal entry to calendar */
    virtual void addJournal(Journal *);
    /** Return Journal for given date */
    virtual Journal *journal(const QDate &);
    /** Return Journal with given UID */
    virtual Journal *journal(const QString &UID);
    /** Return list of all Journals stored in calendar */
    QPtrList<Journal> journalList();

    /** Return all alarms, which ocur in the given time interval. */
    Alarm::List alarms( const QDateTime &from, const QDateTime &to );

  signals:
    /** emitted at regular intervals to indicate that the events in the
      list have triggered an alarm. */
    //void alarmSignal(QPtrList<Incidence> &);
    void alarmSignal(QPtrList<Event> &);
    void alarmSignal(QPtrList<Todo> &);
    /** emitted whenever an event in the calendar changes.  Emits a pointer
      to the changed event. */
    void calUpdated(Incidence *);
  
  public slots:
    /** checks to see if any alarms or todos are pending, and if so, returns a list
     * of those events that have alarms. */
    void checkAlarms();
   
  protected slots:
    /** this method should be called whenever a Event is modified directly
     * via it's pointer.  It makes sure that the calendar is internally
     * consistent. */
    void updateEvent(Incidence *incidence);
  
  protected:
    /** inserts an event into its "proper place" in the calendar. */
    void insertEvent(const Event *anEvent);
  
    /** on the basis of a QDateTime, forms a hash key for the dictionary. */
    long int makeKey(const QDateTime &dt);
    /** on the basis of a QDate, forms a hash key for the dictionary */
    long int makeKey(const QDate &d);
    /** Return the date for which the specified key was made. */
    QDate keyToDate(long int key);

    /** Append alarms of incidence in interval to list of alarms. */
    void appendAlarms( Alarm::List &alarms, Incidence *incidence,
                       const QDateTime &from, const QDateTime &to );
  
  private:
    void init();

    QIntDict<QPtrList<Event> > *mCalDict;    // dictionary of lists of events.
    QPtrList<Event> mRecursList;             // list of repeating events.

    QPtrList<Todo> mTodoList;               // list of todo items.

    QMap<QDate,Journal *> mJournalMap;
  
    QDate *mOldestDate;
    QDate *mNewestDate;
};  

}

#endif
