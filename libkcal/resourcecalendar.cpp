/*
    This file is part of libkdepim.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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

#include <kconfig.h>
#include <kdebug.h>

#include "calendar.h"

#include "resourcecalendar.h"

using namespace KCal;

class AddIncidenceVisitor : public Incidence::Visitor
{
  public:
    /** Add incidence to calendar \a calendar. */
    AddIncidenceVisitor( ResourceCalendar *r ) : mResource( r ) {}

    bool visit( Event *e ) { return mResource->addEvent( e ); }
    bool visit( Todo *t ) { return mResource->addTodo( t ); }
    bool visit( Journal *j ) { return mResource->addJournal( j ); }

  private:
    ResourceCalendar *mResource;
};


ResourceCalendar::ResourceCalendar( const KConfig *config )
    : KRES::Resource( config )
{
}

ResourceCalendar::~ResourceCalendar()
{
}

void ResourceCalendar::writeConfig( KConfig* config )
{
  kdDebug() << "ResourceCalendar::writeConfig()" << endl;

  KRES::Resource::writeConfig( config );
}

bool ResourceCalendar::addIncidence( Incidence *incidence )
{
  AddIncidenceVisitor v( this );
  return incidence->accept( v );
}

QPtrList<Incidence> ResourceCalendar::rawIncidences()
{
  QPtrList<Event> events = rawEvents();
  QPtrList<Todo> todos = rawTodos();
  QPtrList<Journal> journal = journals();
  QPtrList<Incidence> incidences;
  Event *ev;
  for ( ev = events.first(); ev; ev = events.next() ) {
    incidences.append(ev);
  }
  Todo *to;
  for ( to = todos.first(); to; to = todos.next() ) {
    incidences.append(to);
  }
   Journal *jo;
   for ( jo = journal.first(); jo; jo = journal.next() ) {
     incidences.append(jo);
   }

  return incidences;
}

#include "resourcecalendar.moc"
