/*
    This file is part of libkcal.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <qdatetime.h>
#include <qstring.h>
#include <qptrlist.h>

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>

#include "exceptions.h"
#include "incidence.h"
#include "event.h"
#include "todo.h"
#include "journal.h"

#include "resourcecached.h"

using namespace KCal;

ResourceCached::ResourceCached( const KConfig* config )
  : ResourceCalendar( config )
{
}

ResourceCached::~ResourceCached()
{
}

bool ResourceCached::addEvent(Event *event)
{
  return mCalendar.addEvent( event );
}

// probably not really efficient, but...it works for now.
void ResourceCached::deleteEvent(Event *event)
{
  kdDebug(5800) << "ResourceCached::deleteEvent" << endl;

  mCalendar.deleteEvent( event );
}


Event *ResourceCached::event( const QString &uid )
{
  return mCalendar.event( uid );
}

QPtrList<Event> ResourceCached::rawEventsForDate(const QDate &qd, bool sorted)
{
  return mCalendar.rawEventsForDate( qd, sorted );
}


QPtrList<Event> ResourceCached::rawEvents( const QDate &start, const QDate &end,
                                          bool inclusive )
{
  return mCalendar.rawEvents( start, end, inclusive );
}

QPtrList<Event> ResourceCached::rawEventsForDate(const QDateTime &qdt)
{
  return mCalendar.rawEventsForDate( qdt.date() );
}

QPtrList<Event> ResourceCached::rawEvents()
{
  return mCalendar.rawEvents();
}

bool ResourceCached::addTodo(Todo *todo)
{
  return mCalendar.addTodo( todo );
}

void ResourceCached::deleteTodo(Todo *todo)
{
  mCalendar.deleteTodo( todo );
}


QPtrList<Todo> ResourceCached::rawTodos()
{
  return mCalendar.rawTodos();
}

Todo *ResourceCached::todo( const QString &uid )
{
  return mCalendar.todo( uid );
}

QPtrList<Todo> ResourceCached::todos( const QDate &date )
{
  return mCalendar.todos( date );
}


bool ResourceCached::addJournal(Journal *journal)
{
  kdDebug(5800) << "Adding Journal on " << journal->dtStart().toString() << endl;

  return mCalendar.addJournal( journal );
}

Journal *ResourceCached::journal(const QDate &date)
{
//  kdDebug(5800) << "ResourceCached::journal() " << date.toString() << endl;

  return mCalendar.journal( date );
}

Journal *ResourceCached::journal(const QString &uid)
{
  return mCalendar.journal( uid );
}

QPtrList<Journal> ResourceCached::journals()
{
  return mCalendar.journals();
}


Alarm::List ResourceCached::alarmsTo( const QDateTime &to )
{
  return mCalendar.alarmsTo( to );
}

Alarm::List ResourceCached::alarms( const QDateTime &from, const QDateTime &to )
{
//  kdDebug(5800) << "ResourceCached::alarms(" << from.toString() << " - " << to.toString() << ")\n";

  return mCalendar.alarms( from, to );
}
