/*
  Copyright 2009 Ingo Klöcker <kloecker@kde.org>

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

#include "comparisonvisitor.h"
#include "event.h"
#include "freebusy.h"
#include "journal.h"
#include "todo.h"

using namespace KCal;

class ComparisonVisitor::Private
{
  public:
    Private() : mReference( 0 ) {}

  public:
    const IncidenceBase *mReference;
};

ComparisonVisitor::ComparisonVisitor() : d( new Private() )
{
}

ComparisonVisitor::~ComparisonVisitor()
{
  delete d;
}

bool ComparisonVisitor::compare( IncidenceBase *incidence, const IncidenceBase *reference )
{
  d->mReference = reference;

  const bool result = incidence ? incidence->accept( *this ) : reference == 0;

  d->mReference = 0;

  return result;
}

bool ComparisonVisitor::visit( Event *event )
{
  Q_ASSERT( event != 0 );

  const Event *refEvent = dynamic_cast<const Event*>( d->mReference );
  if ( refEvent ) {
    return *event == *refEvent;
  } else {
    // refEvent is no Event and thus cannot be equal to event
    return false;
  }
}

bool ComparisonVisitor::visit( Todo *todo )
{
  Q_ASSERT( todo != 0 );

  const Todo *refTodo = dynamic_cast<const Todo*>( d->mReference );
  if ( refTodo ) {
    return *todo == *refTodo;
  } else {
    // refTodo is no Todo and thus cannot be equal to todo
    return false;
  }
}

bool ComparisonVisitor::visit( Journal *journal )
{
  Q_ASSERT( journal != 0 );

  const Journal *refJournal = dynamic_cast<const Journal*>( d->mReference );
  if ( refJournal ) {
    return *journal == *refJournal;
  } else {
    // refJournal is no Journal and thus cannot be equal to journal
    return false;
  }
}

bool ComparisonVisitor::visit( FreeBusy *freebusy )
{
  Q_ASSERT( freebusy != 0 );

  const FreeBusy *refFreeBusy = dynamic_cast<const FreeBusy*>( d->mReference );
  if ( refFreeBusy ) {
    return *freebusy == *refFreeBusy;
  } else {
    // refFreeBusy is no FreeBusy and thus cannot be equal to freebusy
    return false;
  }
}
