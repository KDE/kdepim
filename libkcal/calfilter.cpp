/*
    This file is part of libkcal.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
}

CalFilter::CalFilter(const QString &name)
{
  mName = name;
}

CalFilter::~CalFilter()
{
}

void CalFilter::apply( Event::List *eventlist )
{
  if ( !mEnabled ) return;

//  kdDebug(5800) << "CalFilter::apply()" << endl;

  Event::List::Iterator it = eventlist->begin();
  while( it != eventlist->end() ) {
    if ( !filterEvent( *it ) ) {
      it = eventlist->remove( it );
    } else {
      ++it;
    }
  }

//  kdDebug(5800) << "CalFilter::apply() done" << endl;
}

// TODO: avoid duplicating apply() code
void CalFilter::apply( Todo::List *eventlist )
{
  if ( !mEnabled ) return;

//  kdDebug(5800) << "CalFilter::apply()" << endl;

  Todo::List::Iterator it = eventlist->begin();
  while( it != eventlist->end() ) {
    if ( !filterTodo( *it ) ) {
      it = eventlist->remove( it );
    } else {
      ++it;
    }
  }

//  kdDebug(5800) << "CalFilter::apply() done" << endl;
}

bool CalFilter::filterEvent(Event *event)
{
//  kdDebug(5800) << "CalFilter::filterEvent(): " << event->getSummary() << endl;

  if ( !mEnabled ) return true;

  if (mCriteria & HideRecurring) {
    if (event->doesRecur()) return false;
  }

  return filterIncidence(event);
}

bool CalFilter::filterTodo(Todo *todo)
{
//  kdDebug(5800) << "CalFilter::filterEvent(): " << event->getSummary() << endl;

  if ( !mEnabled ) return true;

  if (mCriteria & HideCompleted) {
    if (todo->isCompleted()) return false;
  }

  return filterIncidence(todo);
}

bool CalFilter::filterIncidence(Incidence *incidence)
{
//  kdDebug(5800) << "CalFilter::filterEvent(): " << event->getSummary() << endl;

  if ( !mEnabled ) return true;

  if (mCriteria & ShowCategories) {
    for (QStringList::Iterator it = mCategoryList.begin();
         it != mCategoryList.end(); ++it ) {
      QStringList incidenceCategories = incidence->categories();
      for (QStringList::Iterator it2 = incidenceCategories.begin();
           it2 != incidenceCategories.end(); ++it2 ) {
        if ((*it) == (*it2)) {
          return true;
        }
      }
    }
    return false;
  } else {
    for (QStringList::Iterator it = mCategoryList.begin();
         it != mCategoryList.end(); ++it ) {
      QStringList incidenceCategories = incidence->categories();
      for (QStringList::Iterator it2 = incidenceCategories.begin();
           it2 != incidenceCategories.end(); ++it2 ) {
        if ((*it) == (*it2)) {
          return false;
        }
      }
    }
    return true;
  }
    
//  kdDebug(5800) << "CalFilter::filterEvent(): passed" << endl;
  
  return true;
}

void CalFilter::setEnabled(bool enabled)
{
  mEnabled = enabled;
}

bool CalFilter::isEnabled()
{
  return mEnabled;
}

void CalFilter::setCriteria(int criteria)
{
  mCriteria = criteria;
}

int CalFilter::criteria()
{
  return mCriteria;
}

void CalFilter::setCategoryList(const QStringList &categoryList)
{
  mCategoryList = categoryList;
}

QStringList CalFilter::categoryList()
{
  return mCategoryList;
}
