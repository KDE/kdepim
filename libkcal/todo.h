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
      Returns an exact copy of this todo. The returned object is owned by the
      caller.
    */
    Todo *clone();

    /**
      Sets due date and time.

      @param dtDue The due date/time.
      @param first Set the date of the first occurence (if the todo is recurrent).
    */
    void setDtDue(const QDateTime &dtDue, bool first = false);
    /**
      Returns due date and time.

      @param first If true and the todo recurs, the due date of the first
      occurence will be returned.If false and recurrent, the date of the
      current occurence will be returned. If non-recurrent, the normal due date
      will be returned.
    */
    QDateTime dtDue( bool first = false ) const;
    /**
      Returns due time as string formatted according to the users locale
      settings.
    */
    QString dtDueTimeStr() const;
    /**
      Returns due date as string formatted according to the users locale
      settings.

      @param shortfmt If set to true, use short date format, if set to false use
                      long format.
    */
    QString dtDueDateStr( bool shortfmt = true ) const;
    /**
      Returns due date and time as string formatted according to the users locale
      settings.
    */
    QString dtDueStr() const;

    /**
      Returns true if the todo has a due date, otherwise return false.
    */
    bool hasDueDate() const;
    /**
      Set if the todo has a due date.

      @param hasDueDate true if todo has a due date, otherwise false
    */
    void setHasDueDate( bool hasDueDate );

    /**
      Returns true if the todo has a start date, otherwise return false.
    */
    bool hasStartDate() const;
    /**
      Set if the todo has a start date.

      @param hasStartDate true if todo has a start date, otherwise false
    */
    void setHasStartDate( bool hasStartDate );

    /**
      Returns the startdate of the todo.
      @param first If true, the startdate of the todo will be returned. If the
      todo recurs, the startdate of the first occurence will be returned.
      If false and the todo recurs, the relative startdate will be returned,
      based on the date returned by dtRecurrence().
    */
    QDateTime dtStart( bool first = false ) const;

    /**
      Sets the startdate of the todo.
    */
    void setDtStart( const QDateTime &dtStart );

    /** Returns an todo's starting time as a string formatted according to the
     users locale settings.
     @param first If true, the startdate of the todo will be returned. If the
     todo recurs, the startdate of the first occurence will be returned.
     If false and the todo recurs, the relative startdate will be returned,
     based on the date returned by dtRecurrence().
    */
    QString dtStartTimeStr( bool first = false ) const;
    /** Returns an todo's starting date as a string formatted according to the
     users locale settings.
     @param shortfmt If true, use short date format, if set to false use
     long format.
     @param first If true, the startdate of the todo will be returned. If the
     todo recurs, the startdate of the first occurence will be returned.
     If false and the todo recurs, the relative startdate will be returned,
     based on the date returned by dtRecurrence().
    */
    QString dtStartDateStr( bool shortfmt = true, bool first = false ) const;
    /** Returns an todo's starting date and time as a string formatted according
     to the users locale settings.
     @param first If true, the startdate of the todo will be returned. If the
     todo recurs, the startdate of the first occurence will be returned.
     If false and the todo recurs, the relative startdate will be returned,
     based on the date returned by dtRecurrence().
    */
    QString dtStartStr( bool first = false ) const;

    /**
      Returns true if the todo is 100% completed, otherwise return false.
    */
    bool isCompleted() const;
    /**
      Set completed state.

      @param completed If true set completed state to 100%, if false set
                       completed state to 0%.
    */
    void setCompleted( bool completed );

    /**
      Returns how many percent of the task are completed. Returns a value
      between 0 and 100.
    */
    int percentComplete() const;
    /**
      Set how many percent of the task are completed. Valid values are in the
      range from 0 to 100.
    */
    void setPercentComplete( int );

    /**
      Returns date and time when todo was completed.
    */
    QDateTime completed() const;
    /**
      Returns string contaiting date and time when the todo was completed
      formatted according to the users locale settings.
    */
    QString completedStr() const;
    /**
      Set date and time of completion.
    */
    void setCompleted( const QDateTime &completed );

    /**
      Returns true, if todo has a date associated with completion, otherwise
      return false.
    */
    bool hasCompletedDate() const;

    /**
      Sets the due date/time of the current occurence if recurrent.
    */
    void setDtRecurrence( const QDateTime &dt );

    /**
      Returns the due date/time of the current occurence if recurrent.
    */
    QDateTime dtRecurrence() const;

  private:
    bool accept(Visitor &v) { return v.visit( this ); }

    QDateTime mDtDue;                    // due date of todo
                                         // (first occurence if recurrent)
    QDateTime mDtRecurrence;             // due date of recurrence

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
