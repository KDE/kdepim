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

#include "calendarsupport_export.h"

#include <KCalCore/Todo>
#include <KCalCore/Event>
#include <KCalCore/Journal>
#include <KCalCore/Calendar>

#include <QtCore/QObject>

namespace CalendarSupport {

class CALENDARSUPPORT_EXPORT AkonadiCalendar : public KCalCore::Calendar
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

    virtual bool addIncidence( const KCalCore::Incidence::Ptr &incidence );

    virtual bool deleteIncidence( const KCalCore::Incidence::Ptr &incidence );

    virtual KCalCore::Incidence::List incidences() const;

    virtual KCalCore::Incidence::List incidences( const QDate &date ) const;

    virtual KCalCore::Incidence::List incidences( const QString & ) const;

    virtual KCalCore::Incidence::List rawIncidences() const;

    virtual KCalCore::Incidence::List instances( const KCalCore::Incidence::Ptr &incidence ) const;

    virtual bool deleteIncidenceInstances( const KCalCore::Incidence::Ptr &incidence );

    virtual bool beginChange( const KCalCore::Incidence::Ptr &incidence );

    virtual bool endChange( const KCalCore::Incidence::Ptr &incidence );

    virtual bool addEvent( const KCalCore::Event::Ptr &event );

    virtual bool deleteEvent( const KCalCore::Event::Ptr &event );

    virtual bool deleteEventInstances( const KCalCore::Event::Ptr &event );

    virtual void deleteAllEvents();

    static KCalCore::Event::List sortEvents( const KCalCore::Event::List &eventList,
                                             KCalCore::EventSortField sortField,
                                             KCalCore::SortDirection sortDirection );

    virtual KCalCore::Event::List events( KCalCore::EventSortField sortField = KCalCore::EventSortUnsorted,
                                          KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    virtual KCalCore::Event::List rawEvents( KCalCore::EventSortField sortField = KCalCore::EventSortUnsorted,
                                             KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    virtual KCalCore::Event::List rawEventsForDate( const KDateTime &dt ) const;

    virtual KCalCore::Event::List rawEvents( const QDate &start, const QDate &end,
                                             const KDateTime::Spec &timeSpec = KDateTime::Spec(),
                                             bool inclusive = false ) const;

    virtual KCalCore::Event::List rawEventsForDate(  const QDate &date,
                                                     const KDateTime::Spec &timeSpec = KDateTime::Spec(),
                                                     KCalCore::EventSortField sortField = KCalCore::EventSortUnsorted,
                                                     KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    virtual KCalCore::Event::Ptr event( const QString &uid,
                                        const KDateTime &recurrenceId = KDateTime() ) const;
    using QObject::event;

    virtual KCalCore::Event::Ptr deletedEvent( const QString &uid,
                                               const KDateTime &recurrenceId = KDateTime() ) const;

    virtual KCalCore::Event::List eventInstances( const KCalCore::Incidence::Ptr &event,
                                                  KCalCore::EventSortField sortField = KCalCore::EventSortUnsorted,
                                                  KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const = 0;

    virtual bool addTodo( const KCalCore::Todo::Ptr &todo );

    virtual bool deleteTodo( const KCalCore::Todo::Ptr &todo );

    virtual bool deleteTodoInstances( const KCalCore::Todo::Ptr &todo );

    virtual void deleteAllTodos();

    virtual KCalCore::Todo::List todos( KCalCore::TodoSortField sortField = KCalCore::TodoSortUnsorted,
                                        KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    virtual KCalCore::Todo::List todos( const QDate &date ) const;

    virtual KCalCore::Todo::List todos( const QDate &start, const QDate &end,
                                        const KDateTime::Spec &timespec = KDateTime::Spec(),
                                        bool inclusive = false ) const;

    virtual KCalCore::Todo::List rawTodos( KCalCore::TodoSortField sortField = KCalCore::TodoSortUnsorted,
                                           KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    virtual KCalCore::Todo::List rawTodosForDate( const QDate &date ) const;

    virtual KCalCore::Todo::List rawTodos( const QDate &start, const QDate &end,
                                           const KDateTime::Spec &timespec = KDateTime::Spec(),
                                           bool inclusive = false ) const;

    virtual KCalCore::Todo::Ptr todo( const QString &uid,
                                      const KDateTime &recurrenceId = KDateTime() ) const;

    virtual KCalCore::Todo::Ptr deletedTodo( const QString &uid,
                                             const KDateTime &recurrenceId = KDateTime() ) const;

    virtual KCalCore::Todo::List deletedTodos( KCalCore::TodoSortField sortField = KCalCore::TodoSortUnsorted,
                                               KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    virtual KCalCore::Todo::List todoInstances( const KCalCore::Incidence::Ptr &todo,
                                                KCalCore::TodoSortField sortField = KCalCore::TodoSortUnsorted,
                                                KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    virtual bool addJournal( const KCalCore::Journal::Ptr &journal );

    virtual bool deleteJournal( const KCalCore::Journal::Ptr &journal );

    virtual bool deleteJournalInstances( const KCalCore::Journal::Ptr &journal );

    virtual void deleteAllJournals();

    virtual KCalCore::Journal::List journals( KCalCore::JournalSortField sortField = KCalCore::JournalSortUnsorted,
                                              KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    virtual KCalCore::Journal::List journals( const QDate &date ) const;

    virtual KCalCore::Journal::List rawJournals( KCalCore::JournalSortField sortField = KCalCore::JournalSortUnsorted,
                                                 KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    virtual KCalCore::Journal::List rawJournalsForDate( const QDate &date ) const;

    virtual KCalCore::Journal::Ptr journal( const QString &uid,
                                            const KDateTime &recurrenceId = KDateTime() ) const;

    virtual KCalCore::Journal::Ptr deletedJournal( const QString &uid,
                                                   const KDateTime &recurrenceId = KDateTime() ) const;

    virtual KCalCore::Journal::List deletedJournals( KCalCore::JournalSortField sortField = KCalCore::JournalSortUnsorted,
                                                     KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    virtual KCalCore::Journal::List journalInstances( const KCalCore::Incidence::Ptr &journal,
                                                      KCalCore::JournalSortField sortField = KCalCore::JournalSortUnsorted,
                                                      KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending ) const;

    virtual void setupRelations( const KCalCore::Incidence::Ptr &incidence );

    virtual void removeRelations( const KCalCore::Incidence::Ptr &incidence );

    virtual KCalCore::Alarm::List alarms( const KDateTime &from, const KDateTime &to ) const;

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond

    Q_DISABLE_COPY( AkonadiCalendar )
};

}

#endif
