/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef DATENAVIGATOR_H
#define DATENAVIGATOR_H

#include <libkcal/incidencebase.h>

#include <tqobject.h>

/**
  This class controls date navigation. All requests to move the views to another
  date are sent to the DateNavigator. The DateNavigator processes the new
  selection of dates and emits the required signals for the views.
*/
class DateNavigator : public QObject
{
    Q_OBJECT
  public:
    DateNavigator( TQObject *parent = 0, const char *name = 0 );
    ~DateNavigator();

    KCal::DateList selectedDates();

    int datesCount() const;

  public slots:
    void selectDates( const KCal::DateList & );
    void selectDate( const TQDate & );

    void selectDates( int count );
    void selectDates( const TQDate &, int count, const TQDate &preferredMonth = TQDate() );

    void selectWeek();
    void selectWeek( const TQDate &, const TQDate &preferredMonth = TQDate() );

    void selectWorkWeek();
    void selectWorkWeek( const TQDate & );

    void selectWeekByDay( int weekDay, const TQDate &, const TQDate &preferredMonth = TQDate() );

    void selectToday();

    void selectPreviousYear();
    void selectPreviousMonth( const TQDate &currentMonth = TQDate(),
                              const TQDate &selectionLowerLimit = TQDate(),
                              const TQDate &selectionUpperLimit = TQDate() );
    void selectPreviousWeek();
    void selectNextWeek();
    void selectNextMonth( const TQDate &currentMonth = TQDate(),
                          const TQDate &selectionLowerLimit = TQDate(),
                          const TQDate &selectionUpperLimit = TQDate() );
    void selectNextYear();

    void selectPrevious();
    void selectNext();

    void selectMonth( int month );
    void selectYear( int year );

  signals:
    /* preferredMonth is useful when the datelist crosses months,
       if valid, any month-like component should honour it
    */
    void datesSelected( const KCal::DateList &, const TQDate &preferredMonth );

  protected:
    void emitSelected( const TQDate &preferredMonth = TQDate() );

  private:

    /*
      Selects next month if offset equals 1, or previous month
      if offset equals -1.
      Bigger offsets are accepted.
    */
    void shiftMonth( const TQDate &date,
                     const TQDate &selectionLowerLimit,
                     const TQDate &selectionUpperLimit,
                     int offset );

    KCal::DateList mSelectedDates;

    enum {
      MAX_SELECTABLE_DAYS = 50
    };
};

#endif
