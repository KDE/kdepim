/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#ifndef INCIDENCERECURRENCEEDITOR_H
#define INCIDENCERECURRENCEEDITOR_H

#include "incidenceeditor-ng.h"

class QButtonGroup;
class QCheckBox;

namespace Ui {
class IncidenceRecurrenceEditor;
}

namespace IncidenceEditorsNG {

class IncidenceRecurrenceEditor : public IncidenceEditor
{
  Q_OBJECT
  public:
    enum RecurrenceType {
      Daily,
      Weekly,
      Monthly,
      Yearly
    };
  
  public:
    IncidenceRecurrenceEditor( QWidget *parent = 0 );

    virtual void load( KCal::Incidence::ConstPtr incidence );
    virtual void save( KCal::Incidence::Ptr incidence );
    virtual bool isDirty() const;

    void loadPreset( const KCal::Recurrence &preset );
    void removeRecurrence();

  private slots:
    void addException();
    void changeException();
    void deleteException();
    void updateExceptionButtons( const QDate &currentDate );
    void updateExceptionButtons( const QString &selectedDate );
    void updateRecurrenceLabel( int recurrenceRadioIndex );

  private:
    QBitArray days() const;
    int monthlyDay() const;
    int monthlyPos() const;
    int yearlyPosCount() const;
    void setByDay( RecurrenceType type, int day );
    void setByMonth( int day, int month );
    void setByPos( int count, int weekday );
    void setByPos( int count, int weekday, int month );
    void setDateTimes( const QDateTime &from, const QDateTime &to = QDateTime() );
    void setDays( const QBitArray &days );
    void setDefaults( const QDateTime &from, const QDateTime &to );
    void setDuration( int duration );
    void setExceptionDates( const KCal::DateList &dates );
    void setFrequency( int f );
    void setRecurrenceEnabled( bool enabled );
    void setType( RecurrenceType type );

  private:
    KCal::DateList mExceptionDates;

    Ui::IncidenceRecurrenceEditor *mUi;
    QButtonGroup *mTypeButtonGroup;
    QCheckBox *mDayBoxes[7];
};

} // IncidenceEditorsNG

#endif // INCIDENCERECURRENCEEDITOR_H
