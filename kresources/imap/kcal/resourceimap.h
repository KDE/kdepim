 /*
    This file is part of the libkcal IMAP resource.

    Copyright (c) 2003 Steffen Hansen <steffen@klaralvdalens-datakonsult.se>
    Copyright (c) 2003 - 2004 Bo Thorsen <bo@sonofthor.dk>

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

#ifndef KCAL_RESOURCEIMAP_H
#define KCAL_RESOURCEIMAP_H

#include <libkcal/icalformat.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/resourcecalendar.h>

#include <resourceimapshared.h>

namespace KCal {

/**
  This class provides a calendar stored on an IMAP-server via kmail
*/
class ResourceIMAP : public ResourceCalendar, public IncidenceBase::Observer,
                     public ResourceIMAPBase::ResourceIMAPShared
{
  Q_OBJECT

public:
  explicit ResourceIMAP( const KConfig* config = 0 );
  virtual ~ResourceIMAP();

  KABC::Lock *lock();

  /** Add Event to calendar. */
  bool addEvent( Event *anEvent );
  bool addEvent( Event *anEvent, const QString& subresource );
  /** deletes an event from this calendar. */
  void deleteEvent(Event *);

  /**
     Retrieves an event on the basis of the unique string ID.
  */
  Event *event(const QString &UniqueStr);
  /**
     Return filtered list of all events in calendar.
  */
//    Event::List events();
  /**
     Return unfiltered list of all events in calendar.
  */
  Event::List rawEvents( EventSortField sortField = EventSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
  /**
     Builds and then returns a list of all events that match for the
     date specified. useful for dayView, etc. etc.
  */
  Event::List rawEventsForDate( const QDate &date, bool sorted = false );
  /**
     Get unfiltered events for date \a qdt.
  */
  Event::List rawEventsForDate( const QDateTime &qdt );
  /**
     Get unfiltered events in a range of dates. If inclusive is set to true,
     only events are returned, which are completely included in the range.
  */
  Event::List rawEvents( const QDate &start, const QDate &end,
                         bool inclusive = false );

  /*
    Returns a QString with the text of the holiday (if any) that falls
    on the specified date.
  */
  // QString getHolidayForDate(const QDate &qd);

  /**
     Add a todo to the todolist.
  */
  bool addTodo( Todo *todo );
  bool addTodo( Todo *todo, const QString& resource );
  /**
     Remove a todo from the todolist.
  */
  void deleteTodo( Todo * );
  /**
     Searches todolist for an event with this unique string identifier,
     returns a pointer or null.
  */
  Todo *todo( const QString &uid );
  /**
     Return list of all todos.
  */
  Todo::List rawTodos( TodoSortField sortField = TodoSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
  /**
     Returns list of todos due on the specified date.
  */
  Todo::List rawTodosForDate( const QDate &date );
  /** Add a Journal entry to calendar */
  bool addJournal(Journal *);
  bool addJournal(Journal *,  const QString& resource);
  /**
     Remove a journal entry from the journal.
  */
  void deleteJournal( Journal * );
  /** Return Journal for given date */
  virtual Journal *journal(const QDate &);
  /** Return Journal with given UID */
  virtual Journal *journal(const QString &UID);
  /** Return list of all Journals stored in calendar */
  Journal::List rawJournals( JournalSortField sortField = JournalSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
  /** Return the journal for the given date */
  Journal *rawJournalForDate( const QDate &date );

  /** Return all alarms, which ocur in the given time interval. */
  Alarm::List alarms( const QDateTime &from, const QDateTime &to );

  /** Return all alarms, which ocur before given date. */
  Alarm::List alarmsTo( const QDateTime &to );


  friend class ResourceIMAPConfig;

  void setTimeZoneId( const QString& tzid );

  /**
     If this resource has subresources, return a QStringList of them.
     In the normal case, resources do not have subresources, so this is
     by default just empty.
  */
  virtual QStringList subresources() const;

  // Listen to KMail changes
  bool addIncidence( const QString& type, const QString& resource,
                     const QString& ical );
  void deleteIncidence( const QString& type, const QString& resource,
                        const QString& uid );
  void slotRefresh( const QString& type, const QString& resource );
  void subresourceAdded( const QString& type, const QString& id );
  void subresourceDeleted( const QString& type, const QString& id );
  void asyncLoadResult( const QStringList&, const QString&, const QString& );
  bool subresourceActive( const QString& ) const;

public slots:
  /**
     (De-)activate a subresource.
  */
  virtual void setSubresourceActive( const QString& subresource,
                                     bool active );

protected:
  /** Notification function of IncidenceBase::Observer. */
  virtual void incidenceUpdated( IncidenceBase *i );
  /** Append alarms of incidence in interval to list of alarms. */

  void deleteIncidence( const QString& type, const QString& uid,
                        bool silence );

  bool doOpen();
  void doClose();
  bool doLoad();
  bool doSave();

private:
  bool loadResource( const QString& type, const QString& folder );
  bool loadAllEvents();
  bool loadAllTasks();
  bool loadAllJournals();

  // parse a list of incidences of a certain type and add them
  void populate( const QStringList &, const QString& type,
                 const QString& folder );

  KCal::Incidence* parseIncidence( const QString& str );

  QString configFile() const {
    return ResourceIMAPBase::ResourceIMAPShared::configFile( "kcal" );
  }

  ICalFormat mFormat;

  // The default calendar
  CalendarLocal mCalendar;

  // The list of subresources
  QMap<QString, bool> mEventResources, mTaskResources, mJournalResources;
  // Mapping from uid to resource
  QMap<QString, QString> mUidmap;
};

}

#endif
