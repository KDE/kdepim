/*
    Copyright (c) 2009 Kevin Krammer <kevin.krammer@gmx.at>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "assignmentvisitor.h"

#include <kcal/event.h>
#include <kcal/freebusy.h>
#include <kcal/journal.h>
#include <kcal/todo.h>

#include <kdebug.h>

using namespace KCal;

class AssignmentVisitor::Private
{
  public:
    Private() : mSource( 0 ) {}

  public:
    const IncidenceBase *mSource;
};

AssignmentVisitor::AssignmentVisitor() : d( new Private() )
{
}

AssignmentVisitor::~AssignmentVisitor()
{
  delete d;
}

bool AssignmentVisitor::assign( IncidenceBase *target, const IncidenceBase *source )
{
  Q_ASSERT( target != 0 );
  Q_ASSERT( source != 0 );

  d->mSource = source;

  bool result = target->accept( *this );

  d->mSource = 0;

  return result;
}

bool AssignmentVisitor::visit( Event *event )
{
  Q_ASSERT( event != 0 );

  const Event *source = dynamic_cast<const Event*>( d->mSource );
  if ( source == 0 ) {
    kError(5800) << "Type mismatch: source is" << d->mSource->type()
                 << "target is" << event->type();
    return false;
  }

  *event = *source;
  return true;
}

bool AssignmentVisitor::visit( Todo *todo )
{
  Q_ASSERT( todo != 0 );

  const Todo *source = dynamic_cast<const Todo*>( d->mSource );
  if ( source == 0 ) {
    kError(5800) << "Type mismatch: source is" << d->mSource->type()
                 << "target is" << todo->type();
    return false;
  }

  *todo = *source;
  return true;
}

bool AssignmentVisitor::visit( Journal *journal )
{
  Q_ASSERT( journal != 0 );

  const Journal *source = dynamic_cast<const Journal*>( d->mSource );
  if ( source == 0 ) {
    kError(5800) << "Type mismatch: source is" << d->mSource->type()
                 << "target is" << journal->type();
    return false;
  }

  *journal = *source;
  return true;
}

bool AssignmentVisitor::visit( FreeBusy *freebusy )
{
  Q_ASSERT( freebusy != 0 );

  const FreeBusy *source = dynamic_cast<const FreeBusy*>( d->mSource );
  if ( source == 0 ) {
    kError(5800) << "Type mismatch: source is" << d->mSource->type()
                 << "target is" << freebusy->type();
    return false;
  }

  *freebusy = *source;
  return true;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
