/*
    This file is part of libkcal.
    Copyright (c) 1998 Preston Brwon
    Copyright (c) 2001,2002 Cornelius Schumacher <schumacher@kde.org>

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
  CalendarLocal cal;
  if (mCalendar) cal.setTimeZone( mCalendar->getTimeZoneStr() );
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
  kdDebug() << "DndFactory::createDrop()" << endl;

  CalendarLocal cal;
  if (mCalendar) cal.setTimeZone( mCalendar->getTimeZoneStr() );

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

  CalendarLocal cal;
  if (mCalendar) cal.setTimeZone( mCalendar->getTimeZoneStr() );

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


void DndFactory::cutEvent(Event *selectedEv)
{
  if (copyEvent(selectedEv)) {
    mCalendar->deleteEvent(selectedEv);
  }
}

void DndFactory::cutTodo(Todo *selectedTodo)
{
  if (copyTodo(selectedTodo)) {
    mCalendar->deleteTodo(selectedTodo);
  }
}

bool DndFactory::copyEvent( Event *selectedEv )
{
  QClipboard *cb = QApplication::clipboard();

  CalendarLocal cal;
  if (mCalendar) cal.setTimeZone( mCalendar->getTimeZoneStr() );
  Event *ev = new Event( *selectedEv );
  cal.addEvent(ev);
  cb->setData( new ICalDrag( &cal ) );

  return true;
}

bool DndFactory::copyTodo( Todo *selectedTodo )
{
  QClipboard *cb = QApplication::clipboard();

  CalendarLocal cal;
  if (mCalendar) cal.setTimeZone( mCalendar->getTimeZoneStr() );
  Todo *todo = new Todo( *selectedTodo );
  cal.addTodo(todo);
  cb->setData( new ICalDrag( &cal ) );

  return true;
}

Incidence *DndFactory::pasteIncidence(const QDate &newDate, const QTime *newTime)
{
//  kdDebug() << "DnDFactory::pasteEvent()" << endl;

  CalendarLocal cal;

  Event *anEvent = 0;

  QClipboard *cb = QApplication::clipboard();

  if ( !ICalDrag::decode( cb->data(), &cal ) &&
       !VCalDrag::decode( cb->data(), &cal ) ) {
    kdDebug(5800) << "Can't parse clipboard" << endl;
    return 0;
  }

  Event::List evList = cal.events();
  Event *ev = evList.first();
  if ( !evList.isEmpty() && ev ) {
    anEvent = new Event( *ev );

    anEvent->recreate();

    // Calculate length of event
    int daysOffset = anEvent->dtStart().date().daysTo(
      anEvent->dtEnd().date() );
    // new end date if event starts at the same time on the new day
    QDateTime endDate(newDate.addDays(daysOffset), anEvent->dtEnd().time() );

    if ( newTime ) {
      // additional offset for new time of day
      int addSecsOffset( anEvent->dtStart().time().secsTo( *newTime ));
      endDate=endDate.addSecs( addSecsOffset );
      anEvent->setDtStart( QDateTime( newDate, *newTime ) );
    } else {
      anEvent->setDtStart( QDateTime( newDate, anEvent->dtStart().time() ) );
    }

    anEvent->setDtEnd( endDate );
    if (mCalendar) mCalendar->addEvent( anEvent );
		return anEvent;
		
  } else {
	
    Todo::List toList = cal.todos();
    Todo *to = toList.first();
    if ( !toList.isEmpty() && to ) {
      Todo *anTodo = new Todo(*to);
			anTodo->recreate();
			
	    if ( newTime ) {
        anTodo->setDtDue( QDateTime( newDate, *newTime ) );
	    } else {
  	    anTodo->setDtDue( QDateTime( newDate, anTodo->dtDue().time() ) );
    	}
      if (mCalendar) mCalendar->addTodo(anTodo);
			return anTodo;
    } else {
      kdDebug(5800) << "unknown event type in paste!!!" << endl;
    }
  }

  return 0;
}
