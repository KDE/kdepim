/*
    This file is part of libkcal.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
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
   @file calendarresources.h
   Provides a Calendar composed of several Calendar Resources.

   @author Cornelius Schumacher
   @author Reinhold Kainhofer
 */
#ifndef KCAL_CALENDARRESOURCES_H
#define KCAL_CALENDARRESOURCES_H

#include <qintdict.h>
#include <qmap.h>

#include "calendar.h"
#include "resourcecalendar.h"

#include "libkcal_export.h"

#include <kresources/manager.h>

class QWidget;

/**
   @namespace KCal
   Namespace KCal is for global classes, objects and/or functions in libkcal.
*/
namespace KCal {

class CalFormat;

/**
   @class CalendarResources

   This class provides a Calendar which is composed of other Calendars
   known as "Resources".

   Examples of Calendar Resources are:
     - Calendars stored as local ICS formatted files
     - a set of incidences (one-per-file) within a local directory
     - birthdays and anniversaries contained in an addressbook

*/
class LIBKCAL_EXPORT CalendarResources :
      public Calendar,
      public KRES::ManagerObserver<ResourceCalendar>
{
  Q_OBJECT
  public:
    /**
       @class DestinationPolicy
    */
    class DestinationPolicy
    {
      public:
        DestinationPolicy( CalendarResourceManager *manager ) :
          mManager( manager ) {}

        virtual ResourceCalendar *destination( Incidence *incidence ) = 0;

      protected:
        CalendarResourceManager *resourceManager()
         { return mManager; }

      private:
        CalendarResourceManager *mManager;
    };

    /**
       @class StandardDestinationPolicy
    */
    class StandardDestinationPolicy : public DestinationPolicy
    {
      public:
        StandardDestinationPolicy( CalendarResourceManager *manager ) :
          DestinationPolicy( manager ) {}

        ResourceCalendar *destination( Incidence *incidence );

      private:
        class Private;
        Private *d;
    };

    /**
       @class AskDestinationPolicy
    */
    class AskDestinationPolicy : public DestinationPolicy
    {
      public:
        AskDestinationPolicy( CalendarResourceManager *manager,
                              QWidget *parent = 0 ) :
          DestinationPolicy( manager ), mParent( parent ) {}

        ResourceCalendar *destination( Incidence *incidence );

      private:
        QWidget *mParent;

        class Private;
        Private *d;
    };

    /**
       @class Ticket
    */
    class Ticket
    {
        friend class CalendarResources;
      public:
        ResourceCalendar *resource() const
          { return mResource; }

      private:
        Ticket( ResourceCalendar *r ) : mResource( r ) {}

        ResourceCalendar *mResource;

        class Private;
        Private *d;
    };

    /**
       Construct CalendarResource object using a Time Zone and a Family name.

       @param timeZoneId is a string containing a Time Zone ID, which is
       assumed to be valid. The Time Zone Id is used to set the time zone
       for viewing Incidence dates.\n
       On some systems, /usr/share/zoneinfo/zone.tab may be available for
       reference.\n
       @e Example: "Europe/Berlin"

       @warning
       Do Not pass an empty timeZoneId string as this may cause unintended
       consequences when storing Incidences into the Calendar.

       @param family is any QString representing a unique name.
    */
    CalendarResources(
      const QString &timeZoneId,
      const QString &family = QString::fromLatin1( "calendar" ) );

    /**
       Destructor
    */
    ~CalendarResources();

    /**
       Loads all Incidences from the Resources.  The Resources must be added
       first using either readConfig(KConfig *config), which adds the system
       Resources, or manually using resourceAdded(ResourceCalendar *resource).
    */
    void load();

    /**
       Clear out the current Calendar, freeing all used memory etc.
    */
    void close();

    /**
       Save this Calendar.
       If the save is successfull, the Ticket is deleted.  Otherwise, the
       caller must release the Ticket with releaseSaveTicket() to abandon
       the save operation or call save() to try the save again.

       @param ticket is a pointer to the Ticket object.
       @param incidence is a pointer to the Incidence object.
       If incidence is null, save the entire Calendar (all Resources)
       else only the specified Incidence is saved.

       @return true if the save was successful; false otherwise.
    */
    virtual bool save( Ticket *ticket, Incidence *incidence = 0 );

    /**
       Sync changes in memory to persistant storage.
    */
    void save();

    /**
       Determine if the Calendar is currently being saved.

       @return true if the Calendar is currently being saved; false otherwise.
    */
    bool isSaving();

    /**
       Get the CalendarResourceManager used by this calendar.

       @return a pointer to the CalendarResourceManage.
    */
    CalendarResourceManager *resourceManager() const
      { return mManager; }

    /**
       Get the Resource associated with a specified Incidence.

       @param incidence is a pointer to an Incidence whose Resource
       is to be located.

       @return a pointer to the Resource containing the Incidence.
    */
    ResourceCalendar *resource( Incidence *incidence );

    /**
       Read the Resources settings from a config file.

       @param config The KConfig object which points to the config file.
       If no object is given (null pointer) the standard config file is used.

       @note Call this method <em>before</em> load().
    */
    void readConfig( KConfig *config = 0 );

    /**
       Set the destination policy such that Incidences are always added
       to the standard Resource.
    */
    void setStandardDestinationPolicy();

    /**
       Set the destination policy such that Incidences are added to a
       Resource which is queried.
    */
    void setAskDestinationPolicy();

    /**
       Request ticket for saving the Calendar.  If a ticket is returned the
       Calendar is locked for write access until save() or releaseSaveTicket()
       is called.

       @param resource is a pointer to the ResourceCalendar object.

       @return a pointer to a Ticket object indicating that the Calendar
       is locked for write access; otherwise a null pointer.
    */
    Ticket *requestSaveTicket( ResourceCalendar *resource );

    /**
       Release the save Ticket. The Calendar is unlocked without saving.

       @param ticket is a pointer to a Ticket object.
    */
    virtual void releaseSaveTicket( Ticket *ticket );

    /**
       Add a Resource to the Calendar.
       This method must be public, because in-process added Resources
       do not emit the corresponding signal, so this methodd has to be
       called manually!

       @param resource is a pointer to the ResourceCalendar to add.
    */
    void resourceAdded( ResourceCalendar *resource );

// Incidence Specific Methods //

    /**
       Insert an Incidence into the Calendar.

       @param incidence is a pointer to the Incidence to insert.

       @return true if the Incidence was successfully inserted; false otherwise.
    */
    bool addIncidence( Incidence *incidence );

    /**
       Insert an Incidence into a Calendar Resource.

       @param incidence is a pointer to the Incidence to insert.
       @param resource is a pointer to the ResourceCalendar to be added to.

       @return true if the Incidence was successfully inserted; false otherwise.
    */
    bool addIncidence( Incidence *incidence, ResourceCalendar *resource );

    /**
       Flag that a change to a Calendar Incidence is starting.

       @param incidence is a pointer to the Incidence that will be changing.
    */
    bool beginChange( Incidence *incidence );

    /**
       Flag that a change to a Calendar Incidence has completed.

       @param incidence is a pointer to the Incidence that was changed.
    */
    bool endChange( Incidence *incidence );

// Event Specific Methods //

    /**
       Insert an Event into the Calendar.

       @param event is a pointer to the Event to insert.

       @return true if the Event was successfully inserted; false otherwise.

       @note In most cases use
       addIncidence( Incidence *incidence ) instead.
    */
    bool addEvent( Event *event );

    /**
       Insert an Event into a Calendar Resource.

       @param event is a pointer to the Event to insert.
       @param resource is a pointer to the ResourceCalendar to be added to.

       @return true if the Event was successfully inserted; false otherwise.

       @note In most cases use
       addIncidence( Incidence *incidence, ResourceCalendar *resource ) instead.
    */
    bool addEvent( Event *event, ResourceCalendar *resource );

    /**
       Remove an Event from the Calendar.

       @param event is a pointer to the Event to remove.

       @return true if the Event was successfully removed; false otherwise.

       @note In most cases use
       deleteIncidence( Incidence *incidence) instead.
    */
    bool deleteEvent( Event *event );

    /**
       Return a sorted, unfiltered list of all Events.

       @param sortField specifies the EventSortField.
       @param sortDirection specifies the SortDirection.

       @return the list of all unfiltered Events sorted as specified.
    */
    Event::List rawEvents(
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
       Return an unfiltered list of all Events which occur on the given
       timestamp.

       @param qdt request unfiltered Event list for this QDateTime only.

       @return the list of unfiltered Events occuring on the specified
       timestamp.
    */
    Event::List rawEventsForDate( const QDateTime &qdt );

    /**
       Return an unfiltered list of all Events occurring within a date range.

       @param start is the starting date.
       @param end is the ending date.
       @param inclusive if true only Events which are completely included
       within the date range are returned.

       @return the list of unfiltered Events occurring within the specified
       date range.
    */
    Event::List rawEvents( const QDate &start, const QDate &end,
                           bool inclusive = false );

    /**
       Return a sorted, unfiltered list of all Events which occur on the given
       date.  The Events are sorted according to @a sortField and
       @a sortDirection.

       @param date request unfiltered Event list for this QDate only.
       @param sortField specifies the EventSortField.
       @param sortDirection specifies the SortDirection.

       @return the list of sorted, unfiltered Events occuring on @a date.
    */
    Event::List rawEventsForDate(
      const QDate &date,
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
       Returns the Event associated with the given unique identifier.

       @param uid is a unique identifier string.

       @return a pointer to the Event.
       A null pointer is returned if no such Event exists.
    */
    Event *event( const QString &uid );

// Todo Specific Methods //

    /**
       Insert a Todo into a Calendar Resource.

       @param todo is a pointer to the Todo to insert.

       @return true if the Todo was successfully inserted; false otherwise.

       @note In most cases use
       addIncidence( Incidence *incidence ) instead.
    */
    bool addTodo( Todo *todo );

    /**
       Insert an Todo into a Calendar Resource.

       @param todo is a pointer to the Todo to insert.
       @param resource is a pointer to the ResourceCalendar to be added to.

       @return true if the Todo was successfully inserted; false otherwise.

       @note In most cases use
       addIncidence( Incidence *incidence, ResourceCalendar *resource ) instead.
    */
    bool addTodo( Todo *todo, ResourceCalendar *resource );

    /**
       Remove an Todo from the Calendar.

       @param todo is a pointer to the Todo to remove.

       @return true if the Todo was successfully removed; false otherwise.

       @note In most cases use
       deleteIncidence( Incidence *incidence ) instead.
    */
    bool deleteTodo( Todo *todo );

    /**
       Return a sorted, unfiltered list of all Todos for this Calendar.

       @param sortField specifies the TodoSortField.
       @param sortDirection specifies the SortDirection.

       @return the list of all unfiltered Todos sorted as specified.
    */
    Todo::List rawTodos( TodoSortField sortField = TodoSortUnsorted,
                         SortDirection sortDirection = SortDirectionAscending );

    /**
       Return an unfiltered list of all Todos which are due on the specified
       date.

       @param date request unfiltered Todos due on this QDate.

       @return the list of unfiltered Todos due on the specified date.
    */
    Todo::List rawTodosForDate( const QDate &date );

    /**
       Returns the Todo associated with the given unique identifier.

       @param uid is a unique identifier string.

       @return a pointer to the Todo.
       A null pointer is returned if no such Todo exists.
    */
    Todo *todo( const QString &uid );

// Journal Specific Methods //

    /**
       Insert a Journal into the Calendar.

       @param journal is a pointer to the Journal to insert.

       @return true if the Journal was successfully inserted; false otherwise.

       @note In most cases use
       addIncidence( Incidence *incidence ) instead.
    */
    bool addJournal( Journal *journal );

    /**
       Insert a Journal into a Calendar Resource.

       @param journal is a pointer to the Journal to insert.
       @param resource is a pointer to the ResourceCalendar to be added to.

       @return true if the Journal was successfully inserted; false otherwise.

       @note In most cases use
       addIncidence( Incidence *incidence, ResourceCalendar *resource ) instead.
    */
    bool addJournal( Journal *journal, ResourceCalendar *resource );

    /**
       Remove a Journal from the Calendar.

       @param journal is a pointer to the Journal to remove.

       @return true if the Journal was successfully removed; false otherwise.

       @note In most cases use
       deleteIncidence( Incidence *incidence ) instead.
    */
    bool deleteJournal( Journal *journal );

    /**
       Return a sorted, unfiltered list of all Journals for this Calendar.

       @param sortField specifies the JournalSortField.
       @param sortDirection specifies the SortDirection.

       @return the list of all unfiltered Journals sorted as specified.
    */
    Journal::List rawJournals(
      JournalSortField sortField = JournalSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
       Return an unfiltered list of all Journals for on the specifed date.

       @param date request unfiltered Journals for this QDate only.

       @return the list of unfiltered Journals for the specified date.
    */
    Journal::List rawJournalsForDate( const QDate &date );

    /**
       Returns the Journal associated with the given unique identifier.

       @param uid is a unique identifier string.

       @return a pointer to the Journal.
       A null pointer is returned if no such Journal exists.
    */
    Journal *journal( const QString &uid );

// Alarm Specific Methods //

    /**
       Return a list of Alarms within a time range for this Calendar.

       @param from is the starting timestamp.
       @param to is the ending timestamp.

       @return the list of Alarms for the for the specified time range.
    */
    Alarm::List alarms( const QDateTime &from, const QDateTime &to );

    /**
       Return a list of Alarms that occur before the specified timestamp.

       @param to is the ending timestamp.

       @return the list of Alarms occuring before the specified QDateTime.
    */
    Alarm::List alarmsTo( const QDateTime &to );

  signals:
    /**
       Signal that the Resource has been modified.
    */
    void signalResourceModified( ResourceCalendar *resource );

    /**
       Signal that an Incidence has been inserted to the Resource.
    */
    void signalResourceAdded( ResourceCalendar *resource );

    /**
       Signal that an Incidence has been removed from the Resource.
    */
    void signalResourceDeleted( ResourceCalendar *resource );

    /**
       Signal an error message.
    */
    void signalErrorMessage( const QString &err );

  protected:
    void connectResource( ResourceCalendar *resource );
    void resourceModified( ResourceCalendar *resource );
    void resourceDeleted( ResourceCalendar *resource );

    /**
       Let CalendarResource subclasses set the Time Zone ID.

       First parameter is a string containing a Time Zone ID, which is
       assumed to be valid. On some systems, /usr/share/zoneinfo/zone.tab
       may be available for reference.\n
       @e Example: "Europe/Berlin"

       @warning
       Do Not pass an empty timeZoneId string as this may cause unintended
       consequences when storing Incidences into the Calendar.
    */
    virtual void doSetTimeZoneId( const QString &timeZoneId );

    /**
       Increment the number of times this Resource has been changed by 1.

       @param resource is a pointer to the ResourceCalendar to be counted.

       @return the new number of times this Resource has been changed.
    */
    int incrementChangeCount( ResourceCalendar *resource );

    /**
       Decrement the number of times this Resource has been changed by 1.

       @param resource is a pointer to the ResourceCalendar to be counted.

       @return the new number of times this Resource has been changed.
    */
    int decrementChangeCount( ResourceCalendar *resource );

  protected slots:
    void slotLoadError( ResourceCalendar *resource, const QString &err );
    void slotSaveError( ResourceCalendar *resource, const QString &err );

  private:
    /**
       Initialize the Resource object with starting values.
    */
    void init( const QString &family );

    bool mOpen;

    KRES::Manager<ResourceCalendar>* mManager;
    QMap <Incidence*, ResourceCalendar*> mResourceMap;

    DestinationPolicy *mDestinationPolicy;
    StandardDestinationPolicy *mStandardPolicy;
    AskDestinationPolicy *mAskPolicy;

    QMap<ResourceCalendar *, Ticket *> mTickets;
    QMap<ResourceCalendar *, int> mChangeCounts;

    class Private;
    Private *d;
};

}

#endif
