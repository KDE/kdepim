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
  cal.setTimeZone( mCalendar->getTimeZoneStr() );
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
  cal.setTimeZone( mCalendar->getTimeZoneStr() );

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
  cal.setTimeZone( mCalendar->getTimeZoneStr() );

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

bool DndFactory::copyEvent( Event *selectedEv )
{
  QClipboard *cb = QApplication::clipboard();

  CalendarLocal cal;
  cal.setTimeZone( mCalendar->getTimeZoneStr() );
  Event *ev = new Event( *selectedEv );
  cal.addEvent(ev);
  cb->setData( new ICalDrag( &cal ) );

  return true;
}

Event *DndFactory::pasteEvent(const QDate &newDate, const QTime *newTime)
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
  if ( ev ) {
    anEvent = new Event( *ev );

    anEvent->recreate();

    int daysOffset = anEvent->dtEnd().date().dayOfYear() -
      anEvent->dtStart().date().dayOfYear();

    if ( newTime ) {
      anEvent->setDtStart( QDateTime( newDate, *newTime ) );
    } else {
      anEvent->setDtStart( QDateTime( newDate, anEvent->dtStart().time() ) );
    }

    anEvent->setDtEnd( QDateTime( newDate.addDays( daysOffset ),
				  anEvent->dtEnd().time() ) );
    mCalendar->addEvent( anEvent );
  } else {
    Todo::List toList = cal.todos();
    Todo *to = toList.first();
    if (to) {
      //anTodo = new Todo(*to);
      kdDebug(5800) << "Trying to paste a Todo." << endl;
      // TODO: check, if todos can be pasted
      //    Todo *aTodo = VTodoToEvent(curVO);
      //    mCalendar->addTodo(aTodo);
    } else {
      kdDebug(5800) << "unknown event type in paste!!!" << endl;
    }
  }

  return anEvent;
}
