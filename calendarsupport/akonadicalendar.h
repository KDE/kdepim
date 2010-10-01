/*
  This file is part of the kcalcore library.

  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  Author: Sérgio Martins <iamsergio@gmail.com>

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


#ifndef AKONADI_CALENDAR_H
#define AKONADI_CALENDAR_H

#include "todo.h"
#include "event.h"
#include "journal.h"
#include "kcalcore_export.h"

#include <QtCore/QObject>

namespace AkonadiCalendar {

class CALENDARSUPPORT_EXPORT Calendar : public KCalCore::Calendar
{
  Q_OBJECT

  public:
    explicit AkonadiCalendar( const KDateTime::Spec &timeSpec );

    explicit AkonadiCalendar( const QString &timeZoneId );

    virtual ~AkonadiCalendar();

    virtual void close();

    virtual bool save();

    virtual bool reload();

    virtual bool isSaving() const;

    virtual bool addIncidence( const Incidence::Ptr &incidence );

    virtual bool deleteIncidence( const Incidence::Ptr &incidence );

    virtual Incidence::List incidences() const;

    virtual Incidence::List incidences( const QDate &date ) const;

    virtual Incidence::List rawIncidences() const;

    virtual Incidence::List instances( const Incidence::Ptr &incidence ) const;

    virtual bool deleteIncidenceInstances( const Incidence::Ptr &incidence );

    virtual bool beginChange( const Incidence::Ptr &incidence );

    virtual bool endChange( const Incidence::Ptr &incidence );

    virtual bool addEvent( const Event::Ptr &event );

    virtual bool deleteEvent( const Event::Ptr &event );

    virtual bool deleteEventInstances( const Event::Ptr &event );

    virtual void deleteAllEvents();

    static Event::List sortEvents( const Event::List &eventList,
                                   EventSortField sortField,
                                   SortDirection sortDirection );

    virtual Event::List events( EventSortField sortField = EventSortUnsorted,
                                SortDirection sortDirection = SortDirectionAscending ) const;

    virtual Event::List rawEvents(
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) const;

    virtual Event::List rawEventsForDate( const KDateTime &dt ) const;

    virtual Event::List rawEvents( const QDate &start, const QDate &end,
                                   const KDateTime::Spec &timeSpec = KDateTime::Spec(),
                                   bool inclusive = false ) const;

    virtual Event::List rawEventsForDate(
      const QDate &date,
      const KDateTime::Spec &timeSpec = KDateTime::Spec(),
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) const;

    virtual Event::Ptr event( const QString &uid,
                              const KDateTime &recurrenceId = KDateTime() ) const;

    virtual Event::Ptr deletedEvent( const QString &uid,
                                     const KDateTime &recurrenceId = KDateTime() ) const;

    virtual Event::List eventInstances(
      const Incidence::Ptr &event,
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) const = 0;

    virtual bool addTodo( const Todo::Ptr &todo );

    virtual bool deleteTodo( const Todo::Ptr &todo );

    virtual bool deleteTodoInstances( const Todo::Ptr &todo );

    virtual void deleteAllTodos();

    virtual Todo::List todos( TodoSortField sortField = TodoSortUnsorted,
                              SortDirection sortDirection = SortDirectionAscending ) const;

    virtual Todo::List todos( const QDate &date ) const;

    virtual Todo::List todos( const QDate &start, const QDate &end,
                              const KDateTime::Spec &timespec = KDateTime::Spec(),
                              bool inclusive = false ) const;

    virtual Todo::List rawTodos(
      TodoSortField sortField = TodoSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) const;

    virtual Todo::List rawTodosForDate( const QDate &date ) const;

    virtual Todo::List rawTodos( const QDate &start, const QDate &end,
                                 const KDateTime::Spec &timespec = KDateTime::Spec(),
                                 bool inclusive = false ) const;

    virtual Todo::Ptr todo( const QString &uid,
                            const KDateTime &recurrenceId = KDateTime() ) const;

    virtual Todo::Ptr deletedTodo( const QString &uid,
                                   const KDateTime &recurrenceId = KDateTime() ) const;

    virtual Todo::List deletedTodos(
      TodoSortField sortField = TodoSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) const;

    virtual Todo::List todoInstances(
      const Incidence::Ptr &todo,
      TodoSortField sortField = TodoSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) const;

    virtual bool addJournal( const Journal::Ptr &journal );

    virtual bool deleteJournal( const Journal::Ptr &journal );

    virtual bool deleteJournalInstances( const Journal::Ptr &journal );

    virtual void deleteAllJournals();

    virtual Journal::List journals(
      JournalSortField sortField = JournalSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) const;

    virtual Journal::List journals( const QDate &date ) const;

    virtual Journal::List rawJournals(
      JournalSortField sortField = JournalSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) const;

    virtual Journal::List rawJournalsForDate( const QDate &date ) const;

    virtual Journal::Ptr journal( const QString &uid,
                                  const KDateTime &recurrenceId = KDateTime() ) const;

    virtual Journal::Ptr deletedJournal( const QString &uid,
                                         const KDateTime &recurrenceId = KDateTime() ) const;

    virtual Journal::List deletedJournals(
      JournalSortField sortField = JournalSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) const;

    virtual Journal::List journalInstances(
      const Incidence::Ptr &journal,
      JournalSortField sortField = JournalSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) const;

    virtual void setupRelations( const Incidence::Ptr &incidence );

    virtual void removeRelations( const Incidence::Ptr &incidence );

    virtual Alarm::List alarms( const KDateTime &from, const KDateTime &to ) const;

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond

    Q_DISABLE_COPY( AkonadiCalendar )
};

}

#endif
