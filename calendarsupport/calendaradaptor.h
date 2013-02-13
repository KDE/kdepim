/*
  Copyright (c) 2009 KDAB
  Author: Sebastian Sauer <sebsauer@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef CALENDARSUPPORT_CALENDARADAPTOR_H
#define CALENDARSUPPORT_CALENDARADAPTOR_H

#include "calendarsupport_export.h"
#include "incidencechanger.h"

#include <Akonadi/Item>
#include <Akonadi/Collection>

#include <KCalCore/MemoryCalendar>
#include <KCalCore/ScheduleMessage>

class KJob;

namespace CalendarSupport {

class Calendar;

class CALENDARSUPPORT_EXPORT CalendarAdaptor : public KCalCore::MemoryCalendar
{
  Q_OBJECT
  // prevent warning about hidden virtual method
  using QObject::event;
  using KCalCore::Calendar::addIncidence;
  using KCalCore::Calendar::deleteIncidence;

  public:
    typedef QSharedPointer<CalendarAdaptor> Ptr;

    CalendarAdaptor( CalendarSupport::Calendar *calendar, QWidget *parent,
                     bool storeDefaultCollection = false );

    virtual ~CalendarAdaptor();

    virtual bool save();
    virtual bool reload();
    virtual void close();

    /** reimp */
    void endBatchAdding();

    KCalCore::Incidence::List incidencesFromSchedulingID( const QString &sid ) const;
    KCalCore::Incidence::Ptr incidenceFromSchedulingID( const QString &sid ) const;

    virtual void incidenceUpdate( const QString &uid, const KDateTime &recurrenceId );
    virtual void incidenceUpdated( const QString &uid, const KDateTime &recurrenceId );

    virtual bool addEvent( const KCalCore::Event::Ptr &event );
    virtual bool deleteEvent( const KCalCore::Event::Ptr &event );
    virtual void deleteAllEvents();  //unused

    virtual KCalCore::Event::List rawEvents(
      KCalCore::EventSortField sortField = KCalCore::EventSortUnsorted,
      KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    virtual KCalCore::Event::List rawEventsForDate( const KDateTime &dt ) const;

    virtual KCalCore::Event::List rawEvents(
      const QDate &start, const QDate &end,
      const KDateTime::Spec &timeSpec = KDateTime::Spec(),
      bool inclusive = false ) const;

    virtual KCalCore::Event::List rawEventsForDate(
      const QDate &date,
      const KDateTime::Spec &timeSpec = KDateTime::Spec(),
      KCalCore::EventSortField sortField = KCalCore::EventSortUnsorted,
      KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    virtual KCalCore::Event::Ptr event( const QString &uid,
                                        const KDateTime &recurrenceId = KDateTime() ) const;

    virtual bool addTodo( const KCalCore::Todo::Ptr &todo );

    virtual bool deleteTodo( const KCalCore::Todo::Ptr &todo );

    virtual void deleteAllTodos(); //unused

    virtual KCalCore::Todo::List rawTodos(
      KCalCore::TodoSortField sortField = KCalCore::TodoSortUnsorted,
      KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    virtual KCalCore::Todo::List rawTodosForDate( const QDate &date ) const;

    virtual KCalCore::Todo::Ptr todo( const QString &uid,
                                      const KDateTime &recurrenceId = KDateTime() ) const;

    virtual bool addJournal( const KCalCore::Journal::Ptr &journal );

    virtual bool deleteJournal( const KCalCore::Journal::Ptr &journal );

    virtual void deleteAllJournals(); //unused

    // The following functions aren't used by korganizer, but must be implemented because
    // they are pure virtual in kcalcore, don't bother on cleaning this up, or doing a
    // proper implementation because CalendarAdaptor has it's days numbered.
    bool deleteChildIncidences( const KCalCore::Incidence::Ptr & ) { return true; }

    bool deleteChildEvents( const KCalCore::Event::Ptr & ) { return true; }

    bool deleteChildTodos( const KCalCore::Todo::Ptr & ) { return true; }

    bool deleteChildJournals( const KCalCore::Journal::Ptr & ) { return true; }

    KCalCore::Event::Ptr deletedEvent( const QString &, const KDateTime & = KDateTime() ) const
    { return KCalCore::Event::Ptr(); }

    KCalCore::Todo::Ptr deletedTodo( const QString &, const KDateTime & = KDateTime() ) const
    { return KCalCore::Todo::Ptr(); }

    KCalCore::Journal::Ptr deletedJournal( const QString &, const KDateTime & = KDateTime() ) const
    { return KCalCore::Journal::Ptr(); }

    KCalCore::Event::List deletedEvents( KCalCore::EventSortField, KCalCore::SortDirection ) const
    { return KCalCore::Event::List(); }

    KCalCore::Todo::List deletedTodos( KCalCore::TodoSortField, KCalCore::SortDirection ) const
    { return KCalCore::Todo::List(); }

    KCalCore::Journal::List deletedJournals(
      KCalCore::JournalSortField, KCalCore::SortDirection ) const
    { return KCalCore::Journal::List();}

    KCalCore::Event::List childEvents(
      const KCalCore::Incidence::Ptr &,
      KCalCore::EventSortField,
      KCalCore::SortDirection ) const
    { return KCalCore::Event::List(); }

    KCalCore::Todo::List childTodos(
      const KCalCore::Incidence::Ptr &,
      KCalCore::TodoSortField,
      KCalCore::SortDirection ) const
    { return KCalCore::Todo::List(); }

    KCalCore::Journal::List childJournals(
      const KCalCore::Incidence::Ptr &,
      KCalCore::JournalSortField,
      KCalCore::SortDirection ) const
    { return KCalCore::Journal::List(); }

    KCalCore::Todo::List rawTodos(
      const QDate &, const QDate &, const KDateTime::Spec &, bool ) const
    { return KCalCore::Todo::List(); }

    virtual KCalCore::Journal::List rawJournals(
      KCalCore::JournalSortField sortField = KCalCore::JournalSortUnsorted,
      KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    virtual KCalCore::Journal::List rawJournalsForDate( const QDate &dt ) const;

    virtual KCalCore::Journal::Ptr journal( const QString &uid,
                                            const KDateTime &recurrenceId = KDateTime() ) const;

    virtual KCalCore::Alarm::List alarms( const KDateTime &from, const KDateTime &to ) const;

    // From IncidenceChanger
    bool addIncidence( const KCalCore::Incidence::Ptr &incidence );

    bool deleteIncidence( const Akonadi::Item &aitem, bool deleteCalendar = false );

  private slots:
    void deleteIncidenceFinished( KJob *j );

  private:
    bool sendGroupwareMessage( const Akonadi::Item &aitem,
                               KCalCore::iTIPMethod method,
                               IncidenceChanger::HowChanged action );

    //Coming from CalendarView
    void schedule( KCalCore::iTIPMethod method, const Akonadi::Item &item );

    Akonadi::Collection mDefaultCollection;
    QWidget *mParent;
    bool mDeleteCalendar;
    bool mStoreDefaultCollection;

    class Private;
    Private *const d;

};

}

#endif
