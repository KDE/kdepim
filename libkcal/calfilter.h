/*
    This file is part of libkcal.

    Copyright (c) 2001,2003,2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_CALFILTER_H
#define KCAL_CALFILTER_H

#include <qstring.h>
#include <qptrlist.h>

#include "event.h"
#include "todo.h"
#include "journal.h"

namespace KCal {

/**
  Filter for calendar objects.
*/
class CalFilter
{
  public:
    /** Construct filter. */
    CalFilter();
    /** Construct filter with name */
    CalFilter( const QString &name );
    /** Destruct filter. */
    ~CalFilter();

    /**
      Set name of filter.
    */
    void setName( const QString &name ) { mName = name; }
    /**
      Return name of filter.
    */
    QString name() const { return mName; }

    /**
      Apply filter to eventlist, all events not matching filter criterias are
      removed from the list.
    */
    void apply( Event::List *eventlist );

    /**
      Apply filter to todolist, all todos not matching filter criterias are
      removed from the list.
    */
    void apply( Todo::List *todolist );

    /**
      Apply filter to todolist, all todos not matching filter criterias are
      removed from the list.
    */
    void apply( Journal::List *journallist);

    /**
      Apply filter criteria on the specified incidence. Return true, if event passes
      criteria, otherwise return false.
    */
    bool filterIncidence( Incidence * );

    /**
      Enable or disable filter.
    */
    void setEnabled( bool );
    /**
      Return wheter the filter is enabled or not.
    */
    bool isEnabled();


    /**
      Set list of categories, which is used for showing/hiding categories of
      events.
      See related functions.
    */
    void setCategoryList( const QStringList & );
    /**
      Return category list, used for showing/hiding categories of events.
      See related functions.
    */
    QStringList categoryList();

    enum { HideRecurring = 1, HideCompleted = 2, ShowCategories = 4,
           HideInactiveTodos = 8 };

    /**
      Set criteria, which have to be fulfilled by events passing the filter.
    */
    void setCriteria( int );
    /**
      Get inclusive filter criteria.
    */
    int criteria();

  private:
    QString mName;

    int mCriteria;

    bool mEnabled;

    QStringList mCategoryList;

    class Private;
    Private *d;
};

}

#endif
