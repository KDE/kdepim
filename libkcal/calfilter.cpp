/*
    This file is part of libkcal.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (C) 2004 Bram Schoenmakers <bramschoenmakers@kde.nl>

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

#include <kdebug.h>

#include "calfilter.h"

using namespace KCal;

CalFilter::CalFilter()
{
  mEnabled = true;
  mCriteria = 0;
  mCompletedTimeSpan = 0;
}

CalFilter::CalFilter(const QString &name)
{
  mName = name;
  mEnabled = true;
  mCriteria = 0;
  mCompletedTimeSpan = 0;
}

CalFilter::~CalFilter()
{
}

void CalFilter::apply( Event::List *eventlist ) const
{
  if ( !mEnabled ) return;

//  kdDebug(5800) << "CalFilter::apply()" << endl;

  Event::List::Iterator it = eventlist->begin();
  while( it != eventlist->end() ) {
    if ( !filterIncidence( *it ) ) {
      it = eventlist->remove( it );
    } else {
      ++it;
    }
  }

//  kdDebug(5800) << "CalFilter::apply() done" << endl;
}

// TODO: avoid duplicating apply() code
void CalFilter::apply( Todo::List *todolist ) const
{
  if ( !mEnabled ) return;

//  kdDebug(5800) << "CalFilter::apply()" << endl;

  Todo::List::Iterator it = todolist->begin();
  while( it != todolist->end() ) {
    if ( !filterIncidence( *it ) ) {
      it = todolist->remove( it );
    } else {
      ++it;
    }
  }

//  kdDebug(5800) << "CalFilter::apply() done" << endl;
}

void CalFilter::apply( Journal::List *journallist ) const
{
  if ( !mEnabled ) return;

  Journal::List::Iterator it = journallist->begin();
  while( it != journallist->end() ) {
    if ( !filterIncidence( *it ) ) {
      it = journallist->remove( it );
    } else {
      ++it;
    }
  }
}

bool CalFilter::filterIncidence(Incidence *incidence) const
{
//  kdDebug(5800) << "CalFilter::filterIncidence(): " << incidence->summary() << endl;

  if ( !mEnabled ) return true;

  Todo *todo = dynamic_cast<Todo *>(incidence);
  if( todo ) {
    if ( (mCriteria & HideCompleted) && todo->isCompleted() ) {
      // Check if completion date is suffently long ago:
      if ( todo->completed().addDays( mCompletedTimeSpan ) <
           QDateTime::currentDateTime() ) {
        return false;
      }
    }

    if( ( mCriteria & HideInactiveTodos ) &&
        ( todo->hasStartDate() &&
          QDateTime::currentDateTime() < todo->dtStart() ||
          todo->isCompleted() ) )
      return false;
  }


  if (mCriteria & HideRecurring) {
    if (incidence->doesRecur()) return false;
  }

  if (mCriteria & ShowCategories) {
    for (QStringList::ConstIterator it = mCategoryList.constBegin();
         it != mCategoryList.constEnd(); ++it ) {
      QStringList incidenceCategories = incidence->categories();
      for (QStringList::ConstIterator it2 = incidenceCategories.constBegin();
           it2 != incidenceCategories.constEnd(); ++it2 ) {
        if ((*it) == (*it2)) {
          return true;
        }
      }
    }
    return false;
  } else {
    for (QStringList::ConstIterator it = mCategoryList.constBegin();
         it != mCategoryList.constEnd(); ++it ) {
      QStringList incidenceCategories = incidence->categories();
      for (QStringList::ConstIterator it2 = incidenceCategories.constBegin();
           it2 != incidenceCategories.constEnd(); ++it2 ) {
        if ((*it) == (*it2)) {
          return false;
        }
      }
    }
    return true;
  }

//  kdDebug(5800) << "CalFilter::filterIncidence(): passed" << endl;

  return true;
}

void CalFilter::setEnabled(bool enabled)
{
  mEnabled = enabled;
}

bool CalFilter::isEnabled() const
{
  return mEnabled;
}

void CalFilter::setCriteria(int criteria)
{
  mCriteria = criteria;
}

int CalFilter::criteria() const
{
  return mCriteria;
}

void CalFilter::setCategoryList(const QStringList &categoryList)
{
  mCategoryList = categoryList;
}

QStringList CalFilter::categoryList() const
{
  return mCategoryList;
}

void CalFilter::setCompletedTimeSpan( int timespan )
{
  mCompletedTimeSpan = timespan;
}

int CalFilter::completedTimeSpan() const
{
  return mCompletedTimeSpan;
}
