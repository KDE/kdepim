/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001,2003,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
/**
   @file calendar.h
   Provides the main "calendar" object class.

   @author Preston Brown
   @author Cornelius Schumacher
   @author Reinhold Kainhofer
 */
#ifndef KCAL_CALENDAR_H
#define KCAL_CALENDAR_H

#include <qobject.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qdict.h>
#include <kdepimmacros.h>

#include "customproperties.h"
#include "event.h"
#include "todo.h"
#include "journal.h"
#include "kcalversion.h"
#include "person.h"

/**
   @namespace KCal
   Namespace KCal is for global classes, objects and/or functions in libkcal.
*/
namespace KCal {

class CalFilter;

/**
   @enum SortDirection
   Sort direction.
*/
enum SortDirection
{
  /** Sort in ascending order (first to last) */
  SortDirectionAscending,
  /** Sort in descending order (last to first) */
  SortDirectionDescending
};

/**
   @enum EventSortField
   How Events are to be sorted.
*/
enum EventSortField
{
  /** Events are to be unsorted */
  EventSortUnsorted,
  /** Sort Events chronologically, by start date */
  EventSortStartDate,
  /** Sort Events chronologically, by end date */
  EventSortEndDate,
  /** Sort Events alphabetically, by summary */
  EventSortSummary
};

/**
   @enum TodoSortField
   How Todos are to be sorted.
*/
enum TodoSortField
{
  /** Todos are to be unsorted */
  TodoSortUnsorted,
  /** Sort Todos chronologically, by start date */
  TodoSortStartDate,
  /** Sort Todos chronologically, by due date */
  TodoSortDueDate,
  /** Sort Todos by priority */
  TodoSortPriority,
  /** Sort Todos by percentage completed */
  TodoSortPercentComplete,
  /** Sort Todos alphabetically, by summary */
  TodoSortSummary
};

/**
   @enum JournalSortField
   How Journals are to be sorted.
*/
enum JournalSortField
{
  /** Journals are to be unsorted */
  JournalSortUnsorted,
  /** Sort Journals chronologically by date */
  JournalSortDate,
  /** Sort Journals alphabetically, by summary */
  JournalSortSummary
};

/**
   @class Calendar

   This is the main "calendar" object class.  It holds information like
   Incidences(Events, To-dos, Journals), user information, etc. etc.

   This is an abstract base class defining the interface to a calendar. It is
   implemented by subclasses like CalendarLocal, which use different
   methods to store and access the data.

   <b>Ownership of Incidences</b>:

   Incidence ownership is handled by the following policy: As soon as an
   Incidence (or any other subclass of IncidenceBase) object is added to the
   Calendar by an add...() method it is owned by the Calendar object.
   The Calendar takes care of deleting it.  All Incidences returned by the
   query functions are returned as pointers so that changes to the returned
   Incidences are immediately visible in the Calendar.  Do <em>Not</em>
   delete any Incidence object you get from Calendar.

   <b>Time Zone Handling</b>:

   - Incidence Storing:
      - By default, (when LocalTime is unset) Incidence dates will have the
        "UTC" time zone when stored into a calendar file.
      - To store Incidence dates without a time zone (i.e, "floating time
        zone") LocalTime must be set using the setLocalTime() method.

   - Incidence Viewing:
      - By default, Incidence dates will have the "UTC" time zone when
        read from a calendar.
      - To view Incidence dates using another time zone TimeZoneId must
        be set using the setTimeZoneId() method, or the TimeZoneId can
        be passed to the Calendar constructor.
      - It is permitted to switch viewing time zones using setTimeZoneId()
        as desired after the Calendar object has been constructed.

   - Note that:
      - The Calendar class doesn't do anything with TimeZoneId: it simply
        saves it for later use by the ICalFormat class.
      - The ICalFormat class takes TimeZoneId and applies it to loaded
        Incidences before returning them in ICalFormat::load().
      - Each Incidence can have its own time zone (or have a floating
        time zone).
      - Once an Incidence is loaded it is adjusted to use the viewing
        time zone, TimeZoneId.
      - Depending on the LocalTime setting, all loaded Incidences are stored
        either in UTC or without a time zone (floating time zone).
*/
class LIBKCAL_EXPORT Calendar : public QObject, public CustomProperties,
                                public IncidenceBase::Observer
{
    Q_OBJECT

  public:

    /**
       Construct Calendar object using a Time Zone.

       @param timeZoneId is a string containing a Time Zone ID, which is
       assumed to be valid. The Time Zone Id is used to set the time zone
       for viewing Incidence dates.\n
       On some systems, /usr/share/zoneinfo/zone.tab may be available for
       reference.\n
       @e Example: "Europe/Berlin"

       @warning
       Do Not pass an empty timeZoneId string as this may cause unintended
       consequences when storing Incidences into the Calendar.
    */
    Calendar( const QString &timeZoneId = QString::fromLatin1( "UTC" ) );

    /**
       Destructor
    */
    virtual ~Calendar();

    /**
       Set the Calendar Product ID.

       @param productId is a QString containing the Product ID.
    */
    void setProductId( const QString &productId );

    /**
       Get the Calendar's Product ID.

       @return the string containing the Product ID
    */
    QString productId();

    /**
       Set the owner of the Calendar.

       @param owner is a Person object.
    */
    void setOwner( const Person &owner );

    /**
       Get the owner of the Calendar.

       @return the owner Person object.
    */
    const Person &getOwner() const;

    /**
       Set the Time Zone Id for the Calendar.

       @param timeZoneId is a string containing a Time Zone ID, which is
       assumed to be valid. The Time Zone Id is used to set the time zone
       for viewing Incidence dates.\n
       On some systems, /usr/share/zoneinfo/zone.tab may be available for
       reference.\n
       @e Example: "Europe/Berlin"

       @warning
       Do Not pass an empty timeZoneId string as this may cause unintended
       consequences when storing Incidences into the Calendar.
    */
    void setTimeZoneId( const QString &timeZoneId );

    /**
       Get the Time Zone ID for the Calendar.

       @return the string containg the Time Zone ID.
    */
    QString timeZoneId() const;

    /**
       Set to store calendar Incidences without a time zone.
    */
    void setLocalTime();

    /**
       Determine if Calendar Incidences are to be written without a time zone.

       @return true if the Calendar is set to write Incidences withoout
       a time zone; false otherwise.
    */
    bool isLocalTime() const;

    /**
       Set if the Calendar had been modified.

       @param modified is true if the Calendar has been modified since open
       or last save.
    */
    void setModified( bool modified );

    /**
       Determine the Calendar's modification status.

       @return true if the Calendar has been modified since open or last save.
    */
    bool isModified() const { return mModified; }

    /**
       Clears out the current Calendar, freeing all used memory etc.
    */
    virtual void close() = 0;

    /**
       Sync changes in memory to persistant storage.
    */
    virtual void save() = 0;

    /**
       Determine if the Calendar is currently being saved.

       @return true if the Calendar is currently being saved; false otherwise.
    */
    virtual bool isSaving() { return false; }

    /**
       Return a list of all categories used by Incidences in this Calendar.

       @return a QStringList containing all the categories.
    */
    //TODO: need a better method name.  how about categories()?
    QStringList incidenceCategories();

// Incidence Specific Methods //

    /**
       Insert an Incidence into the Calendar.

       @param incidence is a pointer to the Incidence to insert.

       @return true if the Incidence was successfully inserted; false otherwise.
    */
    virtual bool addIncidence( Incidence *incidence );

    /**
       Remove an Incidence from the Calendar.

       @param incidence is a pointer to the Incidence to remove.

       @return true if the Incidence was successfully removed; false otherwise.
    */
    virtual bool deleteIncidence( Incidence *incidence );

    /**
       Return a filtered list of all Incidences for this Calendar.

       @return the list of all filtered Incidences.
    */
    virtual Incidence::List incidences();

    /**
       Return a filtered list of all Incidences which occur on the given date.

       @param date request filtered Incidence list for this QDate only.

       @return the list of filtered Incidences occuring on the specified date.
    */
    virtual Incidence::List incidences( const QDate &date );

    /**
       Return an unfiltered list of all Incidences for this Calendar.

       @return the list of all unfiltered Incidences.
    */
    virtual Incidence::List rawIncidences();

    /**
       Returns the Incidence associated with the given unique identifier.

       @param uid is a unique identifier string.

       @return a pointer to the Incidence.
       A null pointer is returned if no such Incidence exists.
    */
    Incidence *incidence( const QString &uid );

    /**
       Returns the Incidence associated with the given scheduling identifier.

       @param sid is a unique scheduling identifier string.

       @return a pointer to the Incidence.
       A null pointer is returned if no such Incidence exists.
    */
    Incidence *incidenceFromSchedulingID( const QString &sid );

    /**
       Create a merged list of Events, Todos, and Journals.

       @param events is an Event list to merge.
       @param todos is a Todo list to merge.
       @param journals is a Journal list to merge.

       @return a list of merged Incidences.
    */
    static Incidence::List mergeIncidenceList( const Event::List &events,
                                               const Todo::List &todos,
                                               const Journal::List &journals );

    /**
       Flag that a change to a Calendar Incidence is starting.

       @param incidence is a pointer to the Incidence that will be changing.
    */
    virtual bool beginChange( Incidence *incidence );

    /**
       Flag that a change to a Calendar Incidence has completed.

       @param incidence is a pointer to the Incidence that was changed.
    */
    virtual bool endChange( Incidence *incidence );

    /**
       Dissociate an Incidence from a recurring Incidence.
       By default, only one single Incidence for the specified @a date
       will be dissociated and returned.  If @a single is false, then
       the recurrence will be split at @a date, the old Incidence will
       have its recurrence ending at @a date and the new Incidence
       will have all recurrences past the @a date.

       @param incidence is a pointer to a recurring Incidence.
       @param date is the QDate within the recurring Incidence on which
       the dissociation will be performed.
       @param single is a flag meaning that a new Incidence should be created
       from the recurring Incidences after @a date.

       @return a pointer to a new recurring Incidence if @a single is false.
    */
    Incidence *dissociateOccurrence( Incidence *incidence, QDate date,
                                     bool single = true );

// Event Specific Methods //

    /**
       Insert an Event into the Calendar.

       @param event is a pointer to the Event to insert.

       @return true if the Event was successfully inserted; false otherwise.
    */
    virtual bool addEvent( Event *event ) = 0;

    /**
       Remove an Event from the Calendar.

       @param event is a pointer to the Event to remove.

       @return true if the Event was successfully remove; false otherwise.
    */
    virtual bool deleteEvent( Event *event ) = 0;

    /**
       Sort a list of Events.

       @param eventList is a pointer to a list of Events.
       @param sortField specifies the EventSortField.
       @param sortDirection specifies the SortDirection.

       @return a list of Events sorted as specified.
    */
    static Event::List sortEvents( Event::List *eventList,
                                   EventSortField sortField,
                                   SortDirection sortDirection );
    /**
       Return a sorted, filtered list of all Events for this Calendar.

       @param sortField specifies the EventSortField.
       @param sortDirection specifies the SortDirection.

       @return the list of all filtered Events sorted as specified.
    */
    virtual Event::List events(
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
       Return a filtered list of all Events which occur on the given timestamp.

       @param qdt request filtered Event list for this QDateTime only.

       @return the list of filtered Events occuring on the specified timestamp.
    */
    Event::List events( const QDateTime &qdt );

    /**
       Return a filtered list of all Events occurring within a date range.

       @param start is the starting date.
       @param end is the ending date.
       @param inclusive if true only Events which are completely included
       within the date range are returned.

       @return the list of filtered Events occurring within the specified
       date range.
    */
    Event::List events( const QDate &start, const QDate &end,
                        bool inclusive = false);

    /**
       Return a sorted, filtered list of all Events which occur on the given
       date.  The Events are sorted according to @a sortField and
       @a sortDirection.

       @param date request filtered Event list for this QDate only.
       @param sortField specifies the EventSortField.
       @param sortDirection specifies the SortDirection.

       @return the list of sorted, filtered Events occuring on @a date.
    */
    Event::List events(
      const QDate &date,
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
       Return a sorted, unfiltered list of all Events for this Calendar.

       @param sortField specifies the EventSortField.
       @param sortDirection specifies the SortDirection.

       @return the list of all unfiltered Events sorted as specified.
    */
    virtual Event::List rawEvents(
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) = 0;

    /**
       Return an unfiltered list of all Events which occur on the given
       timestamp.

       @param qdt request unfiltered Event list for this QDateTime only.

       @return the list of unfiltered Events occuring on the specified
       timestamp.
    */
    virtual Event::List rawEventsForDate( const QDateTime &qdt ) = 0;

    /**
       Return an unfiltered list of all Events occurring within a date range.

       @param start is the starting date.
       @param end is the ending date.
       @param inclusive if true only Events which are completely included
       within the date range are returned.

       @return the list of unfiltered Events occurring within the specified
       date range.
    */
    virtual Event::List rawEvents( const QDate &start, const QDate &end,
                                   bool inclusive = false ) = 0;

    /**
       Return a sorted, unfiltered list of all Events which occur on the given
       date.  The Events are sorted according to @a sortField and
       @a sortDirection.

       @param date request unfiltered Event list for this QDate only.
       @param sortField specifies the EventSortField.
       @param sortDirection specifies the SortDirection.

       @return the list of sorted, unfiltered Events occuring on @a date.
    */
    virtual Event::List rawEventsForDate(
      const QDate &date,
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) = 0;

    /**
       Returns the Event associated with the given unique identifier.

       @param uid is a unique identifier string.

       @return a pointer to the Event.
       A null pointer is returned if no such Event exists.
    */
    virtual Event *event( const QString &uid ) = 0;

// Todo Specific Methods //

    /**
       Insert a Todo into the Calendar.

       @param todo is a pointer to the Todo to insert.

       @return true if the Todo was successfully inserted; false otherwise.
    */
    virtual bool addTodo( Todo *todo ) = 0;

    /**
       Remove a Todo from the Calendar.

       @param todo is a pointer to the Todo to remove.

       @return true if the Todo was successfully removed; false otherwise.
    */
    virtual bool deleteTodo( Todo *todo ) = 0;

    /**
       Sort a list of Todos.

       @param todoList is a pointer to a list of Todos.
       @param sortField specifies the TodoSortField.
       @param sortDirection specifies the SortDirection.

       @return a list of Todos sorted as specified.
    */
    static Todo::List sortTodos( Todo::List *todoList,
                                 TodoSortField sortField,
                                 SortDirection sortDirection );

    /**
       Return a sorted, filtered list of all Todos for this Calendar.

       @param sortField specifies the TodoSortField.
       @param sortDirection specifies the SortDirection.

       @return the list of all filtered Todos sorted as specified.
    */
    virtual Todo::List todos(
      TodoSortField sortField = TodoSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
       Return a filtered list of all Todos which are due on the specified date.

       @param date request filtered Todos due on this QDate.

       @return the list of filtered Todos due on the specified date.
    */
    virtual Todo::List todos( const QDate &date );

    /**
       Return a sorted, unfiltered list of all Todos for this Calendar.

       @param sortField specifies the TodoSortField.
       @param sortDirection specifies the SortDirection.

       @return the list of all unfiltered Todos sorted as specified.
    */
    virtual Todo::List rawTodos(
      TodoSortField sortField = TodoSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) = 0;

    /**
       Return an unfiltered list of all Todos which due on the specified date.

       @param date request unfiltered Todos due on this QDate.

       @return the list of unfiltered Todos due on the specified date.
    */
    virtual Todo::List rawTodosForDate( const QDate &date ) = 0;

    /**
       Returns the Todo associated with the given unique identifier.

       @param uid is a unique identifier string.

       @return a pointer to the Todo.
       A null pointer is returned if no such Todo exists.
    */
    virtual Todo *todo( const QString &uid ) = 0;

// Journal Specific Methods //

    /**
       Insert a Journal into the Calendar.

       @param journal is a pointer to the Journal to insert.

       @return true if the Journal was successfully inserted; false otherwise.
    */
    virtual bool addJournal( Journal *journal ) = 0;

    /**
       Remove a Journal from the Calendar.

       @param journal is a pointer to the Journal to remove.

       @return true if the Journal was successfully removed; false otherwise.
    */
    virtual bool deleteJournal( Journal *journal ) = 0;

    /**
       Sort a list of Journals.

       @param journalList is a pointer to a list of Journals.
       @param sortField specifies the JournalSortField.
       @param sortDirection specifies the SortDirection.

       @return a list of Journals sorted as specified.
    */
    static Journal::List sortJournals( Journal::List *journalList,
                                       JournalSortField sortField,
                                       SortDirection sortDirection );
    /**
       Return a sorted, filtered list of all Journals for this Calendar.

       @param sortField specifies the JournalSortField.
       @param sortDirection specifies the SortDirection.

       @return the list of all filtered Journals sorted as specified.
    */
    virtual Journal::List journals(
      JournalSortField sortField = JournalSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
       Return a filtered list of all Journals for on the specifed date.

       @param date request filtered Journals for this QDate only.

       @return the list of filtered Journals for the specified date.
    */
    virtual Journal::List journals( const QDate &date );

    /**
       Return a sorted, unfiltered list of all Journals for this Calendar.

       @param sortField specifies the JournalSortField.
       @param sortDirection specifies the SortDirection.

       @return the list of all unfiltered Journals sorted as specified.
    */
    virtual Journal::List rawJournals(
      JournalSortField sortField = JournalSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) = 0;

    /**
       Return an unfiltered list of all Journals for on the specifed date.

       @param date request unfiltered Journals for this QDate only.

       @return the list of unfiltered Journals for the specified date.
    */
    virtual Journal::List rawJournalsForDate( const QDate &date ) = 0;

    /**
       Returns the Journal associated with the given unique identifier.

       @param uid is a unique identifier string.

       @return a pointer to the Journal.
       A null pointer is returned if no such Journal exists.
    */
    virtual Journal *journal( const QString &uid ) = 0;

// Relations Specific Methods //

    /**
       Setup Relations for an Incidence.

       @param incidence is a pointer to the Incidence to have a
       Relation setup.
    */
    virtual void setupRelations( Incidence *incidence );

    /**
       Remove all Relations from an Incidence.

       @param incidence is a pointer to the Incidence to have a
       Relation removed.
    */
    virtual void removeRelations( Incidence *incidence );

// Filter Specific Methods //

    /**
       Set the Calendar filter.

       @param filter a pointer to a CalFilter object which will be
       used to filter Calendar Incidences.
    */
    void setFilter( CalFilter *filter );

    /**
       Return the Calendar filter.

       @return a pointer to the Calendar CalFilter.
       A null pointer is returned if no such CalFilter exists.
    */
    CalFilter *filter();

// Alarm Specific Methods //

    /**
       Return a list of Alarms within a time range for this Calendar.

       @param from is the starting timestamp.
       @param to is the ending timestamp.

       @return the list of Alarms for the for the specified time range.
    */
    virtual Alarm::List alarms( const QDateTime &from,
                                const QDateTime &to ) = 0;

// Observer Specific Methods //

    /**
       @class Observer

       The Observer class.
    */
    class Observer
    {
      public:
        /**
           Notify the Observer that a Calendar has been modified.

           First parameter is true if the Calendar has been modified.\n
           Second parameter is a pointer to the Calendar object that
           is being observed.
        */
        virtual void calendarModified( bool /*modified*/,
                                       Calendar * /*calendar*/ ) {};

        /**
           Notify the Observer that an Incidence has been inserted.

           First parameter is a pointer to the Incidence that was inserted.
        */
        virtual void calendarIncidenceAdded( Incidence * /*incidence*/ ) {}

        /**
           Notify the Observer that an Incidence has been modified.

           First parameter is a pointer to the Incidence that was modified.
        */
        virtual void calendarIncidenceChanged( Incidence * /*incidence*/ ) {}

        /**
           Notify the Observer that an Incidence has been removed.

           First parameter is a pointer to the Incidence that was removed.
        */
          virtual void calendarIncidenceDeleted( Incidence * /*incidence*/ ) {}
    };

    /**
       Register an Observer for this Calendar.

       @param observer is a pointer to an Observer object that will be
       watching this Calendar.
     */
    void registerObserver( Observer *observer );

    /**
       Unregister an Observer for this Calendar.

       @param observer is a pointer to an Observer object that has been
       watching this Calendar.
     */
    void unregisterObserver( Observer *observer );

  signals:
    /**
       Signal that the Calendar has been modified.
     */
    void calendarChanged();

    /**
       Signal that the Calendar has been saved.
     */
    void calendarSaved();

    /**
       Signal that the Calendar has been loaded into memory.
     */
    void calendarLoaded();

  protected:
    /**
       The Observer interface. So far not implemented.

       @param incidenceBase is a pointer an IncidenceBase object.
    */
    void incidenceUpdated( IncidenceBase *incidenceBase );

    /**
       Let Calendar subclasses set the Time Zone ID.

       First parameter is a string containing a Time Zone ID, which is
       assumed to be valid. On some systems, /usr/share/zoneinfo/zone.tab
       may be available for reference.\n
       @e Example: "Europe/Berlin"

       @warning
       Do Not pass an empty timeZoneId string as this may cause unintended
       consequences when storing Incidences into the Calendar.
    */
    virtual void doSetTimeZoneId( const QString &/*timeZoneId*/ ) {}

    /**
       Let Calendar subclasses notify that they inserted an Incidence.

       @param incidence is a pointer to the Incidence object that was inserted.
    */
    void notifyIncidenceAdded( Incidence *incidence );

    /**
       Let Calendar subclasses notify that they modified an Incidence.

       @param incidence is a pointer to the Incidence object that was modified.
    */
    void notifyIncidenceChanged( Incidence *incidence );

    /**
       Let Calendar subclasses notify that they removed an Incidence.

       @param incidence is a pointer to the Incidence object that was removed.
    */
    void notifyIncidenceDeleted( Incidence *incidence );

    /**
       Let Calendar subclasses notify that they enabled an Observer.

       @param enabled if true tells the Calendar that a subclass has
       enabled an Observer.
    */
    void setObserversEnabled( bool enabled );

    //TODO: Move appendAlarms() and appendRecurringAlarms() from
    //      calendarlocal here, as protected static methods
    //      returning static Alarm::List

  private:
    /**
       Intialize a Calendar object with starting values.
    */
    void init();

    QString mProductId;
    Person mOwner;
    QString mTimeZoneId;
    bool mLocalTime;

    bool mModified;

    CalFilter *mFilter;
    CalFilter *mDefaultFilter;

    QPtrList<Observer> mObservers;
    bool mNewObserver;
    bool mObserversEnabled;

    // This list is used to put together related Todos
    QDict<Incidence> mOrphans;
    QDict<Incidence> mOrphanUids;

    class Private;
    Private *d;
  };

}

#endif
