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

#ifndef INCIDENCEEDITOR_INCIDENCERECURRENCE_H
#define INCIDENCEEDITOR_INCIDENCERECURRENCE_H

#include "incidenceeditor-ng.h"

#include <KLocalizedString>
#include <QDate>
namespace Ui
{
class EventOrTodoDesktop;
class EventOrTodoMore;
}

namespace IncidenceEditorNG
{

class IncidenceDateTime;

/// Keep this in sync with the values in mUi->mRecurrenceTypeCombo
enum RecurrenceType {
    RecurrenceTypeNone = 0,
    RecurrenceTypeDaily,
    RecurrenceTypeWeekly,
    RecurrenceTypeMonthly,
    RecurrenceTypeYearly,
    RecurrenceTypeUnknown, // keep this one at the end of the ones which are also in the combobox
    RecurrenceTypeException
};

class INCIDENCEEDITORS_NG_EXPORT IncidenceRecurrence : public IncidenceEditor
{
    Q_OBJECT
public:
#ifdef KDEPIM_MOBILE_UI
    IncidenceRecurrence(IncidenceDateTime *dateTime, Ui::EventOrTodoMore *ui);
#else
    IncidenceRecurrence(IncidenceDateTime *dateTime, Ui::EventOrTodoDesktop *ui);
#endif

    virtual void load(const KCalCore::Incidence::Ptr &incidence);
    virtual void save(const KCalCore::Incidence::Ptr &incidence);
    virtual bool isDirty() const;
    virtual bool isValid() const;

    RecurrenceType currentRecurrenceType() const;

Q_SIGNALS:
    void recurrenceChanged(IncidenceEditorNG::RecurrenceType type);

private Q_SLOTS:
    void addException();
    void fillCombos();
    void handleDateTimeToggle();
    void handleEndAfterOccurrencesChange(int currentValue);
    void handleExceptionDateChange(const QDate &currentDate);
    void handleFrequencyChange();
    void handleRecurrenceTypeChange(int currentIndex);
    void removeExceptions();
    void updateRemoveExceptionButton();
    void updateWeekDays(const QDate &newStartDate);
    void handleStartDateChange(const QDate &);

private:

    /**
       I needed save() to be const, so created this func.
       save() calls this now, and changes members outside.
    */
    void writeToIncidence(const KCalCore::Incidence::Ptr &incidence) const;

    KLocalizedString subsOrdinal(const KLocalizedString &text, int number) const;
    /**
     * Return the day in the month/year on which the event recurs, starting at the
     * beginning/end. Both return a positive number.
     */
    short dayOfMonthFromStart() const;
    short dayOfMonthFromEnd() const;
    short dayOfYearFromStart() const; // We don't need from end for year
    int duration() const;

    /** Returns the week number (1-5) of the month in which the start date occurs. */
    short monthWeekFromStart() const;
    short monthWeekFromEnd() const;

    /** DO NOT USE THIS METHOD DIRECTLY
        use subsOrdinal() instead for i18n * */
    QString numberToString(int number) const;
    void selectMonthlyItem(KCalCore::Recurrence *recurrence, ushort recurenceType);
    void selectYearlyItem(KCalCore::Recurrence *recurrence, ushort recurenceType);
    void setDefaults();
    void setDuration(int duration);
    void setExceptionDates(const KCalCore::DateList &dates);
    void setFrequency(int freq);
    void toggleRecurrenceWidgets(int enable);
    /** Returns an array with the weekday on which the event occurs set to 1 */
    QBitArray weekday() const;

    /**
     * Return how many times the weekday represented by @param date occurs in
     * the month of @param date.
     */
    int weekdayCountForMonth(const QDate &date) const;

    QDate currentDate() const;

private:
#ifdef KDEPIM_MOBILE_UI
    Ui::EventOrTodoMore *mUi;
#else
    Ui::EventOrTodoDesktop *mUi;
#endif
    QDate mCurrentDate;
    IncidenceDateTime *mDateTime;
    KCalCore::DateList mExceptionDates;

    // So we can easily detect if the user changed the type,
    // without going through complicated recurrence logic:
    int mMonthlyInitialType;
    int mYearlyInitialType;
};

}

#endif
