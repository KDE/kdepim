/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@klaralvdalens-datakonsult.se>

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

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef RESOURCEKOLAB_H
#define RESOURCEKOLAB_H

#include <libkcal/calendarlocal.h>
#include <libkcal/resourcecalendar.h>
#include <resourcekolabbase.h>

namespace Kolab {

class ResourceKolab : public KCal::ResourceCalendar,
                      public KCal::IncidenceBase::Observer,
                      public ResourceKolabBase
{
  Q_OBJECT

public:
  ResourceKolab( const KConfig* );
  virtual ~ResourceKolab();

  /// Load resource data.
  bool doLoad();

  /// Save resource data.
  bool doSave();

  /// Open the notes resource.
  bool doOpen();
  /// Close the notes resource.
  void doClose();

  /** Add Event to calendar. */
  bool addEvent( KCal::Event* anEvent );
  /** deletes an event from this calendar. */
  void deleteEvent( KCal::Event* );

  /**
     Retrieves an event on the basis of the unique string ID.
  */
  KCal::Event* event( const QString &UniqueStr );
  /**
     Return filtered list of all events in calendar.
  */
//    Event::List events();
  /**
     Return unfiltered list of all events in calendar.
  */
  KCal::Event::List rawEvents();
  /**
     Builds and then returns a list of all events that match for the
     date specified. useful for dayView, etc. etc.
  */
  KCal::Event::List rawEventsForDate( const QDate& date, bool sorted = false );
  /**
     Get unfiltered events for date \a qdt.
  */
  KCal::Event::List rawEventsForDate( const QDateTime& qdt );
  /**
     Get unfiltered events in a range of dates. If inclusive is set to true,
     only events are returned, which are completely included in the range.
  */
  KCal::Event::List rawEvents( const QDate& start, const QDate& end,
                               bool inclusive = false );

  /*
    Returns a QString with the text of the holiday (if any) that falls
    on the specified date.
  */
  // QString getHolidayForDate(const QDate &qd);

  /**
     Add a todo to the todolist.
  */
  bool addTodo( KCal::Todo* todo );
  bool addTodo( KCal::Todo* todo, const QString& resource );
  /**
     Remove a todo from the todolist.
  */
  void deleteTodo( KCal::Todo* );
  /**
     Searches todolist for an event with this unique string identifier,
     returns a pointer or null.
  */
  KCal::Todo* todo( const QString& uid );
  /**
     Return list of all todos.
  */
  KCal::Todo::List rawTodos();
  /**
     Returns list of todos due on the specified date.
  */
  KCal::Todo::List rawTodosForDate( const QDate& date );
  /** Add a Journal entry to calendar */
  bool addJournal( KCal::Journal* );
  bool addJournal( KCal::Journal*, const QString& resource);
  /**
     Remove a journal entry from the journal.
  */
  void deleteJournal( KCal::Journal* );
  /** Return Journal for given date */
  virtual KCal::Journal* journal( const QDate& );
  /** Return Journal with given UID */
  virtual KCal::Journal* journal( const QString& uid );
  /** Return list of all Journals stored in calendar */
  KCal::Journal::List journals();

  /** Return all alarms, which ocur in the given time interval. */
  KCal::Alarm::List alarms( const QDateTime& from, const QDateTime& to );

  /** Return all alarms, which ocur before given date. */
  KCal::Alarm::List alarmsTo( const QDateTime& to );

  void setTimeZoneId( const QString& tzid );


  /// The ResourceKolabBase methods called by KMail
  bool fromKMailAddIncidence( const QString& type, const QString& resource,
                              Q_UINT32 sernum, const QString& xml );
  void fromKMailDelIncidence( const QString& type, const QString& resource,
                              const QString& uid );
  void slotRefresh( const QString& type, const QString& resource );

  /// Listen to KMail changes in the amount of sub resources
  void fromKMailAddSubresource( const QString& type, const QString& resource,
                                bool writable );
  void fromKMailDelSubresource( const QString& type, const QString& resource );

  /** Return the list of subresources. */
  QStringList subresources() const;

  /** Is this subresource active? */
  bool subresourceActive( const QString& ) const;

  KABC::Lock* lock();

signals:
  void signalSubresourceAdded( Resource*, const QString&, const QString& );
  void signalSubresourceRemoved( Resource*, const QString&, const QString& );

private:
  void addIncidence( const char* mimetype, const QString& xml,
                     const QString& subResource, Q_UINT32 sernum );

  void addEvent( const QString& xml, const QString& subresource,
                 Q_UINT32 sernum );
  bool addEvent( KCal::Event* anEvent, const QString& subresource,
                 Q_UINT32 sernum );

  bool loadAllEvents();
  bool loadAllTodos();
  bool loadAllJournals();

  bool doLoadAll( ResourceMap& map, const char* mimetype );

  /// Reimplemented from IncidenceBase::Observer to know when a note was changed
  void incidenceUpdated( KCal::IncidenceBase* );

  bool addNote( KCal::Journal* journal, const QString& resource,
                Q_UINT32 sernum );
  KCal::Journal* addNote( const QString& xml, const QString& subresource,
                          Q_UINT32 sernum );

  bool openResource( KConfig& config, const char* contentType,
                     ResourceMap& map );
  void loadSubResourceConfig( KConfig& config, const QString& name,
                              bool writable, ResourceMap& subResource );
  bool loadSubResource( const QString& resource, const char* mimetype );

  QString configFile() const {
    return ResourceKolabBase::configFile( "kcal" );
  }

  KCal::CalendarLocal mCalendar;

  // The list of subresources
  ResourceMap mEventSubResources, mTodoSubResources, mJournalSubResources;

  bool mOpen; // If the resource is open, this is true
};

}

#endif // RESOURCEKOLAB_H
