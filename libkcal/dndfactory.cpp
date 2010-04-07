/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001,2002 Cornelius Schumacher <schumacher@kde.org>
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qapplication.h>
#include <qclipboard.h>

#include <kiconloader.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "vcaldrag.h"
#include "icaldrag.h"
#include "calendar.h"
#include "vcalformat.h"
#include "icalformat.h"
#include "calendarlocal.h"

#include "dndfactory.h"

using namespace KCal;

class DndFactory::Private
{
  public:
    Incidence * pasteIncidence( Incidence *inc,
                                const QDate &newDate,
                                const QTime *newTime = 0 )
    {
      if ( inc ) {
        inc = inc->clone();
        inc->recreate();
      }

      if ( inc && newDate.isValid() ) {
        if ( inc->type() == "Event" ) {
          Event *anEvent = static_cast<Event*>( inc );
          // Calculate length of event
          int daysOffset = anEvent->dtStart().date().daysTo(
            anEvent->dtEnd().date() );
          // new end date if event starts at the same time on the new day
          QDateTime endDate( newDate.addDays(daysOffset), anEvent->dtEnd().time() );

          if ( newTime ) {
            // additional offset for new time of day
            int addSecsOffset( anEvent->dtStart().time().secsTo( *newTime ));
            endDate=endDate.addSecs( addSecsOffset );
            anEvent->setDtStart( QDateTime( newDate, *newTime ) );
          } else {
            anEvent->setDtStart( QDateTime( newDate, anEvent->dtStart().time() ) );
          }
          anEvent->setDtEnd( endDate );
        } else if ( inc->type() == "Todo" ) {
          Todo *anTodo = static_cast<Todo*>( inc );
          if ( newTime ) {
            anTodo->setDtDue( QDateTime( newDate, *newTime ) );
          } else {
            anTodo->setDtDue( QDateTime( newDate, anTodo->dtDue().time() ) );
          }
        } else if ( inc->type() == "Journal" ) {
          Journal *anJournal = static_cast<Journal*>( inc );
          if ( newTime ) {
            anJournal->setDtStart( QDateTime( newDate, *newTime ) );
          } else {
            anJournal->setDtStart( QDateTime( newDate ) );
          }
        } else {
          kdDebug(5850) << "Trying to paste unknown incidence of type " << inc->type() << endl;
        }
      }
      return inc;
    }
};

DndFactory::DndFactory( Calendar *cal ) :
  mCalendar( cal ), d( new Private )
{
}

DndFactory::~DndFactory()
{
  delete d;
}

ICalDrag *DndFactory::createDrag( Incidence *incidence, QWidget *owner )
{
  CalendarLocal cal( mCalendar->timeZoneId() );
  Incidence *i = incidence->clone();
  cal.addIncidence( i );

  ICalDrag *icd = new ICalDrag( &cal, owner );
  if ( i->type() == "Event" )
    icd->setPixmap( BarIcon( "appointment" ) );
  else if ( i->type() == "Todo" )
    icd->setPixmap( BarIcon( "todo" ) );

  return icd;
}

Event *DndFactory::createDrop(QDropEvent *de)
{
  kdDebug(5800) << "DndFactory::createDrop()" << endl;

  CalendarLocal cal( mCalendar->timeZoneId() );

  if ( ICalDrag::decode( de, &cal ) || VCalDrag::decode( de, &cal ) ) {
    de->accept();

    Event::List events = cal.events();
    if ( !events.isEmpty() ) {
      Event *event = new Event( *events.first() );
      return event;
    }
  }

  return 0;
}

Todo *DndFactory::createDropTodo(QDropEvent *de)
{
  kdDebug(5800) << "VCalFormat::createDropTodo()" << endl;

  CalendarLocal cal( mCalendar->timeZoneId() );

  if ( ICalDrag::decode( de, &cal ) || VCalDrag::decode( de, &cal ) ) {
    de->accept();

    Todo::List todos = cal.todos();
    if ( !todos.isEmpty() ) {
      Todo *todo = new Todo( *todos.first() );
      return todo;
    }
  }

  return 0;
}

void DndFactory::cutIncidence( Incidence *selectedInc )
{
  Incidence::List list;
  list.append( selectedInc );
  cutIncidences( list );
}

bool DndFactory::cutIncidences( const Incidence::List &incidences )
{
  if ( copyIncidences( incidences ) ) {
    Incidence::List::ConstIterator it;
    for ( it = incidences.constBegin(); it != incidences.constEnd(); ++it ) {
      mCalendar->deleteIncidence( *it );
    }
    return true;
  } else {
    return false;
  }
}

bool DndFactory::copyIncidences( const Incidence::List &incidences )
{
  QClipboard *cb = QApplication::clipboard();
  CalendarLocal cal( mCalendar->timeZoneId() );
  Incidence::List::ConstIterator it;
  for ( it = incidences.constBegin(); it != incidences.constEnd(); ++it ) {
    if ( *it ) {
      cal.addIncidence( ( *it )->clone() );
    }
  }

  if ( cal.incidences().isEmpty() ) {
    return false;
  } else {
    cb->setData( new ICalDrag( &cal ) );
    return true;
  }
}

bool DndFactory::copyIncidence( Incidence *selectedInc )
{
  Incidence::List list;
  list.append( selectedInc );
  return copyIncidences( list );
}

Incidence::List DndFactory::pasteIncidences( const QDate &newDate, const QTime *newTime )
{
  CalendarLocal cal( mCalendar->timeZoneId() );
  QClipboard *cb = QApplication::clipboard();
  Incidence::List list;

  if ( !ICalDrag::decode( cb->data(), &cal ) &&
       !VCalDrag::decode( cb->data(), &cal ) ) {
    kdDebug(5800) << "Can't parse clipboard" << endl;
    return list;
  }

  Incidence::List::ConstIterator it;
  for ( it = cal.incidences().constBegin(); it != cal.incidences().constEnd(); ++it ) {
    Incidence *inc = d->pasteIncidence( *it, newDate, newTime );
    if ( inc ) {
      list.append( inc );
    }
  }
  return list;
}

Incidence *DndFactory::pasteIncidence( const QDate &newDate, const QTime *newTime )
{
  CalendarLocal cal( mCalendar->timeZoneId() );
  QClipboard *cb = QApplication::clipboard();

  if ( !ICalDrag::decode( cb->data(), &cal ) &&
       !VCalDrag::decode( cb->data(), &cal ) ) {
    kdDebug(5800) << "Can't parse clipboard" << endl;
    return 0;
  }

  Incidence::List incList = cal.incidences();
  Incidence *inc = incList.isEmpty() ? 0 : incList.first();

  return d->pasteIncidence( inc, newDate, newTime );
}
