/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001,2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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

#include <QApplication>
#include <QClipboard>
//Added by qt3to4:
#include <QDropEvent>
#include <QPixmap>

#include <kiconloader.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <k3multipledrag.h>
#include <klocale.h>
#include <k3urldrag.h>

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

K3MultipleDrag *DndFactory::createDrag( Incidence *incidence, QWidget *owner )
{
  CalendarLocal cal( mCalendar->timeZoneId() );
  Incidence *i = incidence->clone();
  cal.addIncidence( i );

  K3MultipleDrag *kmd = new K3MultipleDrag( owner );
  kmd->addDragObject( new ICalDrag( &cal, 0 ) );
  QMap<QString, QString> metadata;
  metadata["labels"] = KUrl::toPercentEncoding( i->summary() );
  kmd->addDragObject( new K3URLDrag( i->uri(), metadata, 0 ) );
  
  if ( i->type() == "Event" )
    kmd->setPixmap( BarIcon( "appointment" ) );
  else if ( i->type() == "Todo" )
    kmd->setPixmap( BarIcon( "todo" ) );

  return kmd;
}

Event *DndFactory::createDrop(QDropEvent *de)
{
  kDebug(5800) << "DndFactory::createDrop()" << endl;

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
  kDebug(5800) << "VCalFormat::createDropTodo()" << endl;

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
//  kDebug(5800) << "DnDFactory::pasteEvent()" << endl;

  CalendarLocal cal( mCalendar->timeZoneId() );

  QClipboard *cb = QApplication::clipboard();

  if ( !ICalDrag::decode( cb->data(), &cal ) &&
       !VCalDrag::decode( cb->data(), &cal ) ) {
    kDebug(5800) << "Can't parse clipboard" << endl;
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
      kDebug(5850) << "Trying to paste unknown incidence of type " << inc->type() << endl;
    }

    return inc;

  }

  return 0;
}
