/*
  This file is part of the kcal library.

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2001,2002 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>
  Copyright (c) 2010 Laurent Montel <montel@kde.org>

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
/**
  @file
  This file is part of the API for handling calendar data and
  defines the DndFactory class.

  @brief
  vCalendar/iCalendar Drag-and-Drop object factory.

  @author Preston Brown \<pbrown@kde.org\>
  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#include "dndfactory.h"
#include <kcal/vcaldrag.h>
#include <kcal/icaldrag.h>
#include "calendaradaptor.h"
#include <kcal/calendarlocal.h>

#include <kiconloader.h>
#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>

#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QDropEvent>
#include <QtGui/QPixmap>

using namespace Akonadi;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class Akonadi::DndFactory::Private
{
  public:
  Private( CalendarAdaptor *cal, bool deleteCalendar )
    : mCalendar ( cal ), mDeleteCalendar( deleteCalendar )
    {}
  bool mDeleteCalendar;
  CalendarAdaptor *mCalendar;

};
//@endcond
namespace Akonadi {
DndFactory::DndFactory( CalendarAdaptor *cal, bool deleteCalendarHere )
  : d( new Akonadi::DndFactory::Private ( cal, deleteCalendarHere ) )
{
}

DndFactory::~DndFactory()
{
  delete d;
}

QMimeData *DndFactory::createMimeData()
{
  QMimeData *mimeData = new QMimeData;

  ICalDrag::populateMimeData( mimeData, d->mCalendar );
  VCalDrag::populateMimeData( mimeData, d->mCalendar );

  return mimeData;
}

QDrag *DndFactory::createDrag( QWidget *owner )
{
  QDrag *drag = new QDrag( owner );
  drag->setMimeData( createMimeData() );

  return drag;
}

QMimeData *DndFactory::createMimeData( Incidence *incidence )
{
  CalendarLocal cal( d->mCalendar->timeSpec() );
  Incidence *i = incidence->clone();
  cal.addIncidence( i );

  QMimeData *mimeData = new QMimeData;

  ICalDrag::populateMimeData( mimeData, &cal );
  VCalDrag::populateMimeData( mimeData, &cal );

  KUrl uri = i->uri();
  if ( uri.isValid() ) {
    QMap<QString, QString> metadata;
    metadata[QLatin1String( "labels" )] = KUrl::toPercentEncoding( i->summary() );
    uri.populateMimeData( mimeData, metadata );
  }

  return mimeData;
}

QDrag *DndFactory::createDrag( Incidence *incidence, QWidget *owner )
{
  QDrag *drag = new QDrag( owner );
  drag->setMimeData( createMimeData( incidence ) );

  if ( incidence->type() == "Event" ) {
    drag->setPixmap( BarIcon( QLatin1String( "view-calendar-day" ) ) );
  } else if ( incidence->type() == "Todo" ) {
    drag->setPixmap( BarIcon( QLatin1String( "view-calendar-tasks" ) ) );
  }

  return drag;
}

KCal::Calendar *DndFactory::createDropCalendar( const QMimeData *md )
{
  return createDropCalendar( md, d->mCalendar->timeSpec() );
}

KCal::Calendar *DndFactory::createDropCalendar( const QMimeData *md, const KDateTime::Spec &timeSpec )
{
  KCal::Calendar *cal = new KCal::CalendarLocal( timeSpec );

  if ( ICalDrag::fromMimeData( md, cal ) ||
       VCalDrag::fromMimeData( md, cal ) ){
    return cal;
  }
  delete cal;
  return 0;
}

KCal::Calendar *DndFactory::createDropCalendar( QDropEvent *de )
{
  KCal::Calendar *cal = createDropCalendar( de->mimeData() );
  if ( cal ) {
    de->accept();
    return cal;
  }
  return 0;
}

KCal::Event *DndFactory::createDropEvent( const QMimeData *md )
{
  kDebug();
  Event *ev = 0;
  KCal::Calendar *cal = createDropCalendar( md );

  if ( cal ) {
    Event::List events = cal->events();
    if ( !events.isEmpty() ) {
      ev = new Event( *events.first() );
    }
    delete cal;
  }
  return ev;
}

KCal::Event *DndFactory::createDropEvent( QDropEvent *de )
{
  Event *ev = createDropEvent( de->mimeData() );

  if ( ev ) {
    de->accept();
  }

  return ev;
}

KCal::Todo *DndFactory::createDropTodo( const QMimeData *md )
{
  kDebug();
  KCal::Todo *todo = 0;
  KCal::Calendar *cal = createDropCalendar( md );

  if ( cal ) {
    Todo::List todos = cal->todos();
    if ( !todos.isEmpty() ) {
      todo = new KCal::Todo( *todos.first() );
    }
    delete cal;
  }

  return todo;
}

KCal::Todo *DndFactory::createDropTodo( QDropEvent *de )
{
  KCal::Todo *todo = createDropTodo( de->mimeData() );

  if ( todo ) {
    de->accept();
  }

  return todo;
}

void DndFactory::cutIncidence( const Akonadi::Item &selectedInc )
{
  if ( copyIncidence( selectedInc ) ) {
    d->mCalendar->deleteIncidence( selectedInc, d->mDeleteCalendar );
  }
}

bool DndFactory::copyIncidence( const Akonadi::Item &item )
{
  if ( !item.isValid() ) {
    return false;
  }
  const Incidence::Ptr incidence = Akonadi::incidence( item );
  if ( !incidence ) {
    return false;
  }

  QClipboard *cb = QApplication::clipboard();

  KCal::CalendarLocal cal( d->mCalendar->timeSpec() );

  Incidence *inc = incidence.get()->clone();
  cal.addIncidence( inc );

  QMimeData *mimeData = new QMimeData;
  cb->setMimeData( mimeData );

  ICalDrag::populateMimeData( mimeData, &cal );
  VCalDrag::populateMimeData( mimeData, &cal );

  return true;
}

Incidence *DndFactory::pasteIncidence( const QDate &newDate, const QTime *newTime )
{
  QClipboard *cb = QApplication::clipboard();
  KCal::Calendar *cal = createDropCalendar( cb->mimeData() );

  if ( !cal ) {
    kDebug() << "Can't parse clipboard";
    return 0;
  }
  Incidence *ret = 0;

  Incidence::List incList = cal->incidences();
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
      KDateTime endDate( anEvent->dtEnd() );
      endDate.setDate( newDate.addDays( daysOffset ) );

      KDateTime startDate( anEvent->dtStart() );
      startDate.setDate( newDate );
      if ( newTime ) {
        // additional offset for new time of day
        int addSecsOffset( anEvent->dtStart().time().secsTo( *newTime ) );
        endDate=endDate.addSecs( addSecsOffset );
        startDate.setTime( *newTime );
      }
      anEvent->setDtStart( startDate );
      anEvent->setDtEnd( endDate );

    } else if ( inc->type() == "Todo" ) {
      Todo *anTodo = static_cast<Todo*>( inc );
      KDateTime dueDate( anTodo->dtDue() );
      dueDate.setDate( newDate );
      if ( newTime ) {
        dueDate.setTime( *newTime );
      }
      anTodo->setDtDue( dueDate );
    } else if ( inc->type() == "Journal" ) {
      Journal *anJournal = static_cast<Journal*>( inc );
      KDateTime startDate( anJournal->dtStart() );
      startDate.setDate( newDate );
      if ( newTime ) {
        startDate.setTime( *newTime );
      } else {
        startDate.setTime( QTime( 0, 0, 0 ) );
      }
      anJournal->setDtStart( startDate );
    } else {
      kDebug() << "Trying to paste unknown incidence of type" << inc->type();
    }

    ret = inc;
  }
  delete cal;
  return ret;
}
}
