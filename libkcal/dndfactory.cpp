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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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

DndFactory::DndFactory( Calendar *cal ) :
  mCalendar( cal )
{
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
  if ( copyIncidence( selectedInc ) ) {
    mCalendar->deleteIncidence( selectedInc );
  }
}

bool DndFactory::copyIncidence( Incidence *selectedInc )
{
  if ( !selectedInc )
    return false;
  QClipboard *cb = QApplication::clipboard();

  CalendarLocal cal( mCalendar->timeZoneId() );
  Incidence *inc = selectedInc->clone();
  cal.addIncidence( inc );
  cb->setData( new ICalDrag( &cal ) );

  return true;
}

Incidence *DndFactory::pasteIncidence(const QDate &newDate, const QTime *newTime)
{
//  kdDebug(5800) << "DnDFactory::pasteEvent()" << endl;

  CalendarLocal cal( mCalendar->timeZoneId() );

  QClipboard *cb = QApplication::clipboard();

  if ( !ICalDrag::decode( cb->data(), &cal ) &&
       !VCalDrag::decode( cb->data(), &cal ) ) {
    kdDebug(5800) << "Can't parse clipboard" << endl;
    return 0;
  }

  Incidence::List incList = cal.incidences();
  Incidence *inc = incList.first();

  if ( !incList.isEmpty() && inc ) {
    inc = inc->clone();

    inc->recreate();

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

    return inc;

  }

  return 0;
}
