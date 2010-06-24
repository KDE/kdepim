/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#ifndef INCIDENCERECURRENCE_H
#define INCIDENCERECURRENCE_H

#include "incidenceeditor-ng.h"

namespace Ui {
class EventOrTodoDesktop;
}

namespace IncidenceEditorsNG {

class IncidenceDateTime;

class IncidenceRecurrence : public IncidenceEditor
{
    Q_OBJECT
public:
    IncidenceRecurrence( IncidenceDateTime *dateTime = 0, Ui::EventOrTodoDesktop *ui = 0 );

    virtual void load( KCal::Incidence::ConstPtr incidence );
    virtual void save( KCal::Incidence::Ptr incidence );
    virtual bool isDirty() const;
    virtual bool isValid();

private Q_SLOTS:
    void addException();
    void fillCombos();
    void handleExceptionDateChange( const QDate &currentDate );
    void handleFrequencyChange();
    void handleRecurrenceTypeChange( int currentIndex );
    void removeExceptions();
    void updateRemoveExceptionButton();

private:
    /**
     * Return the day in the month/year on which the event recurs, starting at the
     * beginning/end. Both return a positive number.
     */
    short dayOfMonthFromStart() const;
    short dayOfMonthFromEnd() const;
    short dayOfYearFromStart() const; // We don't need from end for year
    /** Returns the days selected in the day combo */
    QBitArray days() const;
    int duration() const;

    /** Returns the week number (1-5) of the month in which the start date occurs. */
    short monthWeekFromStart() const;
    short monthWeekFromEnd() const;

    QString numberToString( int number ) const;
    void selectMonthlyItem( KCal::Recurrence *recurrence, ushort recurenceType );
    void selectYearlyItem( KCal::Recurrence *recurrence, ushort recurenceType );
    void setDays( const QBitArray &days );
    void setDefaults();
    void setDuration( int duration );
    void setExceptionDates( const KCal::DateList &dates );
    void setFrequency( int freq );
    void toggleRecurrenceWidgets( bool enable );
    /** Returns an array with the weekday on which the event occurs set to 1 */
    QBitArray weekday() const;

    /**
     * Return how many times the weekday represented by @param date occurs in
     * the month of @param date.
     */
    int weekdayCountForMonth( const QDate &date ) const;

private:
    Ui::EventOrTodoDesktop *mUi;
    IncidenceDateTime *mDateTime;
    KCal::DateList mExceptionDates;
};

}

#endif // INCIDENCERECURRENCE_H
