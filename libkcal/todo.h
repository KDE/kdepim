/*
    This file is part of libkcal.

    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_TODO_H
#define KCAL_TODO_H

#include "incidence.h"

namespace KCal {

/**
  This class provides a Todo in the sense of RFC2445.
*/
class Todo : public Incidence
{
  public:
    typedef ListBase<Todo> List;

    Todo();
    Todo( const Todo & );
    ~Todo();
    bool operator==( const Todo& ) const;

    QCString type() const { return "Todo"; }

    /**
      Return an exact copy of this todo. The returned object is owned by the
      caller.
    */
    Todo *clone();

    /**
      Set due date and time.
    */
    void setDtDue(const QDateTime &dtDue);
    /**
      Return due date and time.
    */
    QDateTime dtDue() const;
    /**
      Return due time as string formatted according to the users locale
      settings.
    */
    QString dtDueTimeStr() const;
    /**
      Return due date as string formatted according to the users locale
      settings.
      
      @param shortfmt If set to true, use short date format, if set to false use
                      long format.
    */
    QString dtDueDateStr( bool shortfmt = true ) const;
    /**
      Return due date and time as string formatted according to the users locale
      settings.
    */
    QString dtDueStr() const;

    /**
      Return true if the todo has a due date, otherwise return false.
    */
    bool hasDueDate() const;
    /**
      Set if the todo has a due date.
      
      @param hasDueDate true if todo has a due date, otherwise false
    */
    void setHasDueDate( bool hasDueDate );

    /**
      Return true if the todo has a start date, otherwise return false.
    */
    bool hasStartDate() const;
    /**
      Set if the todo has a start date.
      
      @param hasDueDate true if todo has a start date, otherwise false
    */
    void setHasStartDate( bool hasStartDate );

    /**
      Return true if the todo is 100% completed, otherwise return false.
    */
    bool isCompleted() const;
    /**
      Set completed state.
      
      @param completed If true set completed state to 100%, if false set
                       completed state to 0%.
    */
    void setCompleted( bool completed );
    
    /**
      Return how many percent of the task are completed. Returns a value
      between 0 and 100.
    */
    int percentComplete() const;
    /**
      Set how many percent of the task are completed. Valid values are in the
      range from 0 to 100.
    */
    void setPercentComplete( int );

    /**
      Return date and time when todo was completed.
    */
    QDateTime completed() const;
    /**
      Return string contaiting date and time when the todo was completed
      formatted according to the users locale settings.
    */
    QString completedStr() const;
    /**
      Set date and time of completion.
    */
    void setCompleted( const QDateTime &completed );

    /**
      Return true, if todo has a date associated with completion, otherwise
      return false.
    */
    bool hasCompletedDate() const;
    
  private:
    bool accept(Visitor &v) { return v.visit( this ); }

    QDateTime mDtDue;                    // due date of todo

    bool mHasDueDate;                    // if todo has associated due date
    bool mHasStartDate;                  // if todo has associated start date

    QDateTime mCompleted;
    bool mHasCompletedDate;

    int mPercentComplete;

    class Private;
    Private *d;
};

}

#endif
