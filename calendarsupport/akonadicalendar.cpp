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

#include "akonadicalendar.h"

using namespace CalendarSupport;
using namespace KCalCore;

class AkonadiCalendar::Private
{
  public:
    Private( AkonadiCalendar *qq ) : q( qq )
    {
    }

    ~Private()
    {
    }

  private:
    AkonadiCalendar *q;

};


AkonadiCalendar::AkonadiCalendar( const KDateTime::Spec &timeSpec )
  : KCalCore::Calendar( timeSpec ), d( new Private( this ) )
{

}

AkonadiCalendar::AkonadiCalendar( const QString &timeZoneId )
  : KCalCore::Calendar( timeZoneId ), d( new Private( this ) )
{

}

AkonadiCalendar::~AkonadiCalendar()
{
  delete d;
}

void AkonadiCalendar::close()
{
  // Not applicable
}

bool AkonadiCalendar::save()
{
  // Not applicable
  return true;
}

bool AkonadiCalendar::reload()
{
  // Not applicable
  return true;
}

bool AkonadiCalendar::isSaving() const
{
  // TODO: we can return if there's a job running?
  return false;
}

bool AkonadiCalendar::addIncidence( const Incidence::Ptr &incidence )
{
  // TODO: Create a job to add an item.
  Q_UNUSED( incidence );
  return true;
}

bool AkonadiCalendar::deleteIncidence( const Incidence::Ptr &incidence )
{
  //TODO: Create a job to delete an item.
  Q_UNUSED( incidence );
  return true;
}

Incidence::List AkonadiCalendar::incidences() const
{
  return Incidence::List();
}

Incidence::List AkonadiCalendar::incidences( const QString &notebook ) const
{
  Q_UNUSED( notebook );
  return Incidence::List();
}

Incidence::List AkonadiCalendar::incidences( const QDate &date ) const
{
  Q_UNUSED( date );
  return Incidence::List();
}

Incidence::List AkonadiCalendar::rawIncidences() const
{
  return Incidence::List();
}

Incidence::List AkonadiCalendar::instances( const Incidence::Ptr &incidence ) const
{
  Q_UNUSED( incidence );
  return Incidence::List();
}

bool AkonadiCalendar::deleteIncidenceInstances( const Incidence::Ptr &incidence )
{
  Q_UNUSED( incidence );
  return true;
}

bool AkonadiCalendar::beginChange( const Incidence::Ptr &incidence )
{
  Q_UNUSED( incidence );
  return true;
}

bool AkonadiCalendar::endChange( const Incidence::Ptr &incidence )
{
  Q_UNUSED( incidence );
  return true;
}

bool AkonadiCalendar::addEvent( const Event::Ptr &event )
{
  Q_UNUSED( event );
  return true;
}

bool AkonadiCalendar::deleteEvent( const Event::Ptr &event )
{
  Q_UNUSED( event );
  return true;
}

bool AkonadiCalendar::deleteEventInstances( const Event::Ptr &event )
{
  Q_UNUSED( event );
  return true;
}

void AkonadiCalendar::deleteAllEvents()
{
}

Event::List AkonadiCalendar::events( EventSortField sortField,
                                     SortDirection sortDirection) const
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Event::List();
}

Event::List AkonadiCalendar::rawEvents( EventSortField sortField,
                                        SortDirection sortDirection ) const
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Event::List();
}

Event::List AkonadiCalendar::rawEventsForDate( const KDateTime &dt ) const
{
  Q_UNUSED( dt );
  return Event::List();
}

Event::List AkonadiCalendar::rawEvents( const QDate &start, const QDate &end,
                                        const KDateTime::Spec &timeSpec,
                                        bool inclusive ) const
{
  Q_UNUSED( start );
  Q_UNUSED( end );
  Q_UNUSED( timeSpec );
  Q_UNUSED( inclusive );
  return Event::List();
}

Event::List AkonadiCalendar::rawEventsForDate( const QDate &date,
                                               const KDateTime::Spec &timeSpec,
                                               EventSortField sortField,
                                               SortDirection sortDirection ) const
{
  Q_UNUSED( date );
  Q_UNUSED( timeSpec );
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Event::List();
}

Event::Ptr AkonadiCalendar::event( const QString &uid,
                                   const KDateTime &recurrenceId ) const
{
  Q_UNUSED( uid );
  Q_UNUSED( recurrenceId );
  return Event::Ptr();
}

Event::Ptr AkonadiCalendar::deletedEvent( const QString &uid,
                                          const KDateTime &recurrenceId ) const
{
  Q_UNUSED( uid );
  Q_UNUSED( recurrenceId );
  return Event::Ptr();
}

Event::List AkonadiCalendar::eventInstances( const Incidence::Ptr &event,
                                             EventSortField sortField,
                                             SortDirection sortDirection ) const
{
  Q_UNUSED( event );
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Event::List();
}

bool AkonadiCalendar::addTodo( const Todo::Ptr &todo )
{
  Q_UNUSED( todo );
  return true;
}

bool AkonadiCalendar::deleteTodo( const Todo::Ptr &todo )
{
  Q_UNUSED( todo );
  return true;
}

bool AkonadiCalendar::deleteTodoInstances( const Todo::Ptr &todo )
{
  Q_UNUSED( todo );
  return true;
}

void AkonadiCalendar::deleteAllTodos()
{
}

Todo::List AkonadiCalendar::todos( TodoSortField sortField,
                                   SortDirection sortDirection ) const
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Todo::List();
}

Todo::List AkonadiCalendar::todos( const QDate &date ) const
{
  Q_UNUSED( date );
  return Todo::List();
}

Todo::List AkonadiCalendar::todos( const QDate &start, const QDate &end,
                                   const KDateTime::Spec &timespec,
                                   bool inclusive ) const
{
  Q_UNUSED( start );
  Q_UNUSED( end );
  Q_UNUSED( timespec );
  Q_UNUSED( inclusive );
  return Todo::List();
}

Todo::List AkonadiCalendar::rawTodos( TodoSortField sortField,
                                      SortDirection sortDirection ) const
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Todo::List();
}

Todo::List AkonadiCalendar::rawTodosForDate( const QDate &date ) const
{
  Q_UNUSED( date );
  return Todo::List();
}

Todo::List AkonadiCalendar::rawTodos( const QDate &start, const QDate &end,
                                      const KDateTime::Spec &timespec,
                                      bool inclusive ) const
{
  Q_UNUSED( start );
  Q_UNUSED( end );
  Q_UNUSED( timespec );
  Q_UNUSED( inclusive );
  return Todo::List();
}

Todo::Ptr AkonadiCalendar::todo( const QString &uid,
                                 const KDateTime &recurrenceId ) const
{
  Q_UNUSED( uid );
  Q_UNUSED( recurrenceId );
  return Todo::Ptr();
}

Todo::Ptr AkonadiCalendar::deletedTodo( const QString &uid,
                                        const KDateTime &recurrenceId ) const
{
  Q_UNUSED( uid );
  Q_UNUSED( recurrenceId );
  return Todo::Ptr();
}

Todo::List AkonadiCalendar::deletedTodos( TodoSortField sortField,
                                          SortDirection sortDirection ) const
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Todo::List();
}

Todo::List AkonadiCalendar::todoInstances( const Incidence::Ptr &todo,
                                           TodoSortField sortField,
                                           SortDirection sortDirection ) const
{
  Q_UNUSED( todo );
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Todo::List();
}

bool AkonadiCalendar::addJournal( const Journal::Ptr &journal )
{
  Q_UNUSED( journal );
  return true;
}

bool AkonadiCalendar::deleteJournal( const Journal::Ptr &journal )
{
  Q_UNUSED( journal );
  return true;
}

bool AkonadiCalendar::deleteJournalInstances( const Journal::Ptr &journal )
{
  Q_UNUSED( journal );
  return true;
}

void AkonadiCalendar::deleteAllJournals()
{
}

Journal::List AkonadiCalendar::journals( JournalSortField sortField,
                                         SortDirection sortDirection ) const
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Journal::List();
}

Journal::List AkonadiCalendar::journals( const QDate &date ) const
{
  Q_UNUSED( date );
  return Journal::List();
}

Journal::List AkonadiCalendar::rawJournals( JournalSortField sortField,
                                            SortDirection sortDirection ) const
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Journal::List();
}

Journal::List AkonadiCalendar::rawJournalsForDate( const QDate &date ) const
{
  Q_UNUSED( date );
  return Journal::List();
}

Journal::Ptr AkonadiCalendar::journal( const QString &uid,
                                       const KDateTime &recurrenceId ) const
{
  Q_UNUSED( uid );
  Q_UNUSED( recurrenceId );
  return Journal::Ptr();
}

Journal::Ptr AkonadiCalendar::deletedJournal( const QString &uid,
                                              const KDateTime &recurrenceId ) const
{
  Q_UNUSED( uid );
  Q_UNUSED( recurrenceId );
  return Journal::Ptr();
}

Journal::List AkonadiCalendar::deletedJournals( JournalSortField sortField,
                                                SortDirection sortDirection ) const
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Journal::List();
}

Journal::List AkonadiCalendar::journalInstances( const Incidence::Ptr &journal,
                                                 JournalSortField sortField,
                                                 SortDirection sortDirection ) const
{
  Q_UNUSED( journal );
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Journal::List();
}

void AkonadiCalendar::setupRelations( const Incidence::Ptr &incidence )
{
  Q_UNUSED( incidence );
}

void AkonadiCalendar::removeRelations( const Incidence::Ptr &incidence )
{
  Q_UNUSED( incidence );
}

Alarm::List AkonadiCalendar::alarms( const KDateTime &from, const KDateTime &to ) const
{
  Q_UNUSED( from );
  Q_UNUSED( to );
  return Alarm::List();
}
