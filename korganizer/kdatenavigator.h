/*
    This file is part of KOrganizer.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
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
#ifndef KDATENAVIGATOR_H
#define KDATENAVIGATOR_H

#include <tqframe.h>
#include <tqdatetime.h>

#include <libkcal/incidencebase.h>

class TQPushButton;
class TQLabel;

namespace KCal {
class Calendar;
class Incidence;
}
class NavigatorBar;
using namespace KCal;

class KODayMatrix;

class KDateNavigator: public QFrame
{
    Q_OBJECT
  public:
    KDateNavigator( TQWidget *parent = 0, const char *name = 0 );
    ~KDateNavigator();

    /**
      Associate date navigator with a calendar. It is used by KODayMatrix.
    */
    void setCalendar( Calendar * );

    void setBaseDate( const TQDate & );

    KCal::DateList selectedDates() const { return mSelectedDates; }

    TQSizePolicy sizePolicy () const;

    NavigatorBar *navigatorBar() const { return mNavigatorBar; }
    TQDate startDate() const;
    TQDate endDate() const;
    void setUpdateNeeded();

    /**
       Returns the current displayed month.
       It's a TQDate instead of uint so it can be easily feed to KCalendarSystem's
       functions.
    */
    TQDate month() const;

  public slots:
    void selectDates( const KCal::DateList & );
    void updateView();
    void updateConfig();
    void updateDayMatrix();
    void updateToday();

  signals:
    void datesSelected( const KCal::DateList & );
    void incidenceDropped( Incidence *, const TQDate & );
    void incidenceDroppedMove( Incidence *, const TQDate & );
    void weekClicked( const TQDate & );

    void goPrevious();
    void goNext();
    void nextMonthClicked();
    void prevMonthClicked();
    void nextYearClicked();
    void prevYearClicked();

    void monthSelected( int month );
    void yearSelected( int year );

  protected:
    void updateDates();

    void wheelEvent( TQWheelEvent * );

    bool eventFilter( TQObject *, TQEvent * );

    void setShowWeekNums( bool enabled );

  private:
    NavigatorBar *mNavigatorBar;

    TQLabel *mHeadings[ 7 ];
    TQLabel *mWeeknos[ 7 ];

    KODayMatrix *mDayMatrix;

    KCal::DateList mSelectedDates;
    TQDate mBaseDate;

    // Disabling copy constructor and assignment operator
    KDateNavigator( const KDateNavigator & );
    KDateNavigator &operator=( const KDateNavigator & );
};

#endif
