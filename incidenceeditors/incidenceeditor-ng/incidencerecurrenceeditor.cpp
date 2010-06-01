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

#include "incidencerecurrenceeditor.h"

#include <KCalendarSystem>
#include <KSystemTimeZones>

#include <KCal/Todo>

#include "ui_incidencerecurrenceeditor.h"

using namespace IncidenceEditorsNG;
using namespace KCal;

IncidenceRecurrenceEditor::IncidenceRecurrenceEditor( QWidget *parent )
  : IncidenceEditor( parent )
  , mUi( new Ui::IncidenceRecurrenceEditor )
{
  mUi->setupUi( this );

  mDayBoxes[0] = mUi->mDay0Check;
  mDayBoxes[1] = mUi->mDay1Check;
  mDayBoxes[2] = mUi->mDay2Check;
  mDayBoxes[3] = mUi->mDay3Check;
  mDayBoxes[4] = mUi->mDay4Check;
  mDayBoxes[5] = mUi->mDay5Check;
  mDayBoxes[6] = mUi->mDay6Check;

  int weekStart=KGlobal::locale()->weekStartDay();
  for ( int i = 0; i < 7; ++i ) {
    // i is the nr of the combobox, not the day of week!
    // label=(i+weekStart+6)%7 + 1;
    // index in CheckBox array(=day): label-1
    const KCalendarSystem *calSys = KGlobal::locale()->calendar();
    QString weekDayName = calSys->weekDayName( ( i + weekStart + 6 ) % 7 + 1,
                                               KCalendarSystem::ShortDayName );
    QString longDayName = calSys->weekDayName( ( i + weekStart + 6 ) % 7 + 1,
                                               KCalendarSystem::LongDayName );
    mDayBoxes[ ( i + weekStart + 6 ) % 7 ]->setText( weekDayName );
    mDayBoxes[ ( i + weekStart + 6 ) % 7 ]->setWhatsThis(
      i18nc( "@info:whatsthis",
             "Set %1 as the day when this event or to-do should recur.", longDayName ) );
    mDayBoxes[ ( i + weekStart + 6 ) % 7 ]->setToolTip(
      i18nc( "@info:tooltip", "Recur on %1", longDayName ) );
  }

  connect( mUi->mTypeButtonGroup, SIGNAL(buttonClicked(int)),
           SLOT(updateRecerrenceLabel(int)) );
}

void IncidenceRecurrenceEditor::load( KCal::Incidence::ConstPtr inc )
{
  Q_ASSERT( inc );

  mLoadedIncidence = inc;

  QBitArray rDays( 7 );
  int day = 0;
  int count = 0;
  int month = 0;

  KDateTime::Spec timeSpec = KSystemTimeZones::local();
  if ( inc->type() == "Todo" ) {
    KCal::Todo::ConstPtr todo = incidence<KCal::Todo>();
    setDefaults( todo->dtStart( true ).toTimeSpec( timeSpec ).dateTime(),
                 todo->dtDue().toTimeSpec( timeSpec ).dateTime() /*, todo->allDay() */ );
  } else {
    setDefaults( inc->dtStart().toTimeSpec( timeSpec ).dateTime(),
                 inc->dtEnd().toTimeSpec( timeSpec ).dateTime() /*, incidence->allDay() */ );
  }

  uint recurs = mLoadedIncidence->recurrenceType();
  int f = 0;
  Recurrence *r = 0;

  if ( recurs ) {
    r = mLoadedIncidence->recurrence();
    f = r->frequency();
  }

  setRecurrenceEnabled( recurs );

  RecurrenceType recurrenceType = IncidenceRecurrenceEditor::Weekly;

  switch ( recurs ) {
  case Recurrence::rNone:
    break;
  case Recurrence::rDaily:
    recurrenceType = IncidenceRecurrenceEditor::Daily;
    setFrequency( f );
    break;
  case Recurrence::rWeekly:
    recurrenceType = IncidenceRecurrenceEditor::Weekly;
    setFrequency( f );
    setDays( r->days() );
    break;
  case Recurrence::rMonthlyPos:
  {
    // TODO: we only handle one possibility in the list right now,
    // so I have hardcoded calls with first().  If we make the GUI
    // more extended, this can be changed.
    recurrenceType = IncidenceRecurrenceEditor::Monthly;

    QList<RecurrenceRule::WDayPos> rmp = r->monthPositions();
    if ( !rmp.isEmpty() ) {
      setByPos( rmp.first().pos(), rmp.first().day() );
    }

    setFrequency( f );
    break;
  }
  case Recurrence::rMonthlyDay:
  {
    recurrenceType = IncidenceRecurrenceEditor::Monthly;

    QList<int> rmd = r->monthDays();
    // check if we have any setting for which day (vcs import is broken and
    // does not set any day, thus we need to check)
    if ( rmd.isEmpty() ) {
      day = mLoadedIncidence->dtStart().date().day();
    } else {
      day = rmd.first();
    }
    setByDay( Monthly, day );
    setFrequency( f );
    break;
  }
  case Recurrence::rYearlyMonth:
  {
    recurrenceType = IncidenceRecurrenceEditor::Yearly;
    QList<int> rmd = r->yearDates();
    if ( rmd.isEmpty() ) {
      day = mLoadedIncidence->dtStart().date().day();
    } else {
      day = rmd.first();
    }
    int month = mLoadedIncidence->dtStart().date().month();
    rmd = r->yearMonths();
    if ( !rmd.isEmpty() ) {
      month = rmd.first();
    }
    setByMonth( day, month );
    setFrequency( f );
    break;
  }
  case Recurrence::rYearlyPos:
  {
    recurrenceType = IncidenceRecurrenceEditor::Yearly;

    QList<int> months = r->yearMonths();
    if ( months.isEmpty() ) {
      month = mLoadedIncidence->dtStart().date().month();
    } else {
      month = months.first();
    }

    QList<RecurrenceRule::WDayPos> pos = r->yearPositions();

    if ( pos.isEmpty() ) {
      // Use dtStart if nothing is given (shouldn't happen!)
      count = ( mLoadedIncidence->dtStart().date().day() - 1 ) / 7;
      day = mLoadedIncidence->dtStart().date().dayOfWeek();
    } else {
      count = pos.first().pos();
      day = pos.first().day();
    }
    setByPos( count, day, month );
    setFrequency( f );
    break;
  }
  case Recurrence::rYearlyDay:
  {
    recurrenceType = IncidenceRecurrenceEditor::Yearly;
    QList<int> days = r->yearDays();
    if ( days.isEmpty() ) {
      day = mLoadedIncidence->dtStart().date().dayOfYear();
    } else {
      day = days.first();
    }
    setByDay( Yearly, day );
    setFrequency( f );
    break;
  }
  default:
    break;
  }

  setType( recurrenceType );
  mUi->mRuleStack->setCurrentIndex( recurrenceType );

  setDateTimes(
    mLoadedIncidence->recurrence()->startDateTime().toTimeSpec( timeSpec ).dateTime() );

  if ( mLoadedIncidence->recurs() && r ) {
    setDuration( r->duration() );
    if ( r->duration() == 0 )
      mUi->mEndDateEdit->setDate( r->endDate() );
  }

  setExceptionDates( mLoadedIncidence->recurrence()->exDates() );
}

void IncidenceRecurrenceEditor::save( KCal::Incidence::Ptr incidence )
{
  // clear out any old settings;
  Recurrence *r = incidence->recurrence();
  r->unsetRecurs();

  if ( !mUi->mEnabledCheck->isChecked() || !isEnabled() )
    return;

  int duration = mUi->mEndDurationEdit->value();
  QDate endDate;
  if ( duration == 0 ) {
    endDate = mUi->mEndDateEdit->date();
  }

  int recurrenceType = mUi->mTypeButtonGroup->checkedId();
  if ( recurrenceType == Daily ) {
    r->setDaily( mUi->mFrequencyEdit->value() );
  } else if ( recurrenceType == Weekly ) {
    r->setWeekly( mUi->mFrequencyEdit->value(), days() );
  } else if ( recurrenceType == Monthly ) {
    r->setMonthly( mUi->mFrequencyEdit->value() );

    if ( mUi->mMonthlyByPosRadio->isChecked() ) {
      int pos = monthlyPos();

      QBitArray days( 7 );
      days.fill( false );
      days.setBit( mUi->mMonthlyByPosWeekdayCombo->currentIndex() );
      r->addMonthlyPos( pos, days );
    } else {
      // it's by day
      r->addMonthlyDate( monthlyDay() );
    }
  } else if ( recurrenceType == Yearly ) {
    r->setYearly( mUi->mFrequencyEdit->value() );

    switch ( mUi->mYearlyButtonGroup->checkedId() ) {
    case 0: // by month
      r->addYearlyDate( mUi->mYearlyByDaySpin->value() );
      r->addYearlyMonth( mUi->mYearlyByMonthCombo->currentIndex() + 1 );
      break;
    case 1: // by pos:
    {
      r->addYearlyMonth( mUi->mYearlyByPosMonthCombo->currentIndex() + 1 );
      QBitArray days( 7 );
      days.fill( false );
      days.setBit( mUi->mYearlyByPosWeekdayCombo->currentIndex() );
      r->addYearlyPos( yearlyPosCount(), days );
      break;
    }
    case 2: // by day:
      r->addYearlyDay( mUi->mYearlyByDaySpin->value() );
      break;
    }
  } // end "Yearly"

  if ( duration > 0 ) {
    r->setDuration( duration );
  } else if ( duration == 0 ) {
    r->setEndDate( endDate );
  }
  incidence->recurrence()->setExDates( mExceptionDates );
}

bool IncidenceRecurrenceEditor::isDirty() const
{
  return false;
}

/// Private slots

void IncidenceRecurrenceEditor::updateRecerrenceLabel( int recurrenceRadioIndex )
{
  switch ( recurrenceRadioIndex ) {
    case 0:
      mUi->mRecurrenceRuleLabel->setText( i18nc( "@label recurrence expressed in days", "day(s)" ) );
      break;
    case 1:
      mUi->mRecurrenceRuleLabel->setText( i18nc( "@label", "week(s) on:" ) );
      break;
    case 2:
      mUi->mRecurrenceRuleLabel->setText( i18nc( "@label", "month(s)" ) );
      break;
    case 3:
      mUi->mRecurrenceRuleLabel->setText( i18nc( "@label", "year(s)" ) );
      break;
    default:
      Q_ASSERT( false );
  }
}

/// Private functions

QBitArray IncidenceRecurrenceEditor::days() const
{
  QBitArray days( 7 );

  for ( int i = 0; i < 7; ++i )
    days.setBit( i, mDayBoxes[ i ]->isChecked() );

  return days;
}

int IncidenceRecurrenceEditor::monthlyDay() const
{
  int day = mUi->mMonthlyByDayCombo->currentIndex();
  if ( day >= 31 )
    day = 31-day-1;
  else
    ++day;

  return day;
}

int IncidenceRecurrenceEditor::monthlyPos() const
{
  int pos = mUi->mMonthlyByPosCountCombo->currentIndex();
  if ( pos <= 4 ) // positive count
    return pos + 1;
  else
    return 4 - pos;
}

int IncidenceRecurrenceEditor::yearlyPosCount() const
{
  int pos = mUi->mYearlyByPosDayCombo->currentIndex();
  if ( pos <= 4 ) { // positive count
    return pos + 1;
  } else {
    return 4 - pos;
  }
}

void IncidenceRecurrenceEditor::setByPos( int count, int weekday )
{
  mUi->mMonthlyByPosRadio->setChecked( true );
  if ( count > 0 ) {
    mUi->mMonthlyByPosCountCombo->setCurrentIndex( count - 1 );
  } else {
    // negative weeks means counted from the end of month
    mUi->mMonthlyByPosCountCombo->setCurrentIndex( -count + 4 );
  }
  mUi->mMonthlyByPosWeekdayCombo->setCurrentIndex( weekday - 1 );
}

void IncidenceRecurrenceEditor::setByPos( int count, int weekday, int month )
{
  mUi->mYearlyByPosRadio->setChecked( true );
  if ( count > 0 ) {
    mUi->mYearlyByPosDayCombo->setCurrentIndex( count - 1 );
  } else {
    mUi->mYearlyByPosDayCombo->setCurrentIndex( -count + 4 );
  }
  mUi->mYearlyByPosWeekdayCombo->setCurrentIndex( weekday - 1 );
  mUi->mYearlyByPosMonthCombo->setCurrentIndex( month-1 );
}

void IncidenceRecurrenceEditor::setByDay( RecurrenceType type, int day )
{
  Q_ASSERT( type == Monthly || type == Yearly );

  if ( type == Monthly ) {
    mUi->mMonthlyByDayRadio->setChecked( true );
    // Days from the end are after the ones from the begin, so correct for the
    // negative sign and add 30 (index starting at 0)
    if ( day > 0 && day <= 31 ) {
      mUi->mMonthlyByDayCombo->setCurrentIndex( day - 1 );
    } else if ( day < 0 ) {
      mUi->mMonthlyByDayCombo->setCurrentIndex( 31 - 1 - day );
    }
  } else if ( type == Yearly ) {
    mUi->mYearlyByDayRadio->setChecked( true );
    mUi->mYearlyByDaySpin->setValue( day );
  }
}

void IncidenceRecurrenceEditor::setByMonth( int day, int month )
{
  mUi->mYearlyByMonthRadio->setChecked( true );
  mUi->mYearlyByMonthSpin->setValue( day );
  mUi->mYearlyByMonthCombo->setCurrentIndex( month - 1 );
}

void IncidenceRecurrenceEditor::setDateTimes( const QDateTime &start,
                                              const QDateTime & )
{
  mUi->mStartDateLabel->setText(
    i18nc( "@label", "Begins on: %1", KGlobal::locale()->formatDate( start.date() ) ) );

  // These methods had an empty impl in RecurBase in the old code, with the following
  // comment attached:
  //
  // FIXME: If we want to adjust the recurrence when the start/due date change,
  // we need to reimplement this method in the derived classes!

//   mDaily->setDateTimes( start, end );
//   mWeekly->setDateTimes( start, end );
//   mMonthly->setDateTimes( start, end );
//   mYearly->setDateTimes( start, end );

  // Now set the defaults for all unused types, use the start time for it
  bool enabled = mUi->mEnabledCheck->isChecked();
  int type = mUi->mTypeButtonGroup->checkedId();

  if ( !enabled || type != Weekly ) {
    QBitArray days( 7 );
    days.fill( 0 );
    days.setBit( ( start.date().dayOfWeek() + 6 ) % 7 );
    setDays( days );
  }
  if ( !enabled || type != Monthly ) {
    setByPos( ( start.date().day() - 1 ) / 7 + 1, start.date().dayOfWeek() - 1 );
    setByDay( Monthly, start.date().day() );
  }
  if ( !enabled || type != Yearly ) {
    setByDay( Yearly, start.date().dayOfYear() );
    setByPos( ( start.date().day() - 1 ) / 7 + 1,
              start.date().dayOfWeek(), start.date().month() );
    setByMonth( start.date().day(), start.date().month() );
  }
}

void IncidenceRecurrenceEditor::setDays( const QBitArray &days )
{
  for ( int i = 0; i < 7; ++i ) {
    mDayBoxes[ i ]->setChecked( days.testBit( i ) );
  }
}

void IncidenceRecurrenceEditor::setDefaults( const QDateTime &from, const QDateTime &to )
{
  setDateTimes( from, to );
  setRecurrenceEnabled( false );

  mUi->mNoEndDateButton->setChecked( true );
  mUi->mEndDateEdit->setDate( from.date() );

  setType( IncidenceRecurrenceEditor::Weekly );
  mUi->mRuleStack->setCurrentIndex( IncidenceRecurrenceEditor::Weekly );

  setFrequency( 1 );

  QBitArray days( 7 );
  days.fill( 0 );
  days.setBit( ( from.date().dayOfWeek() + 6 ) % 7 );
  setDays( days );

  setByPos( ( from.date().day() - 1 ) / 7 + 1, from.date().dayOfWeek() );
  setByDay( Monthly, from.date().day() );

  setByDay( Yearly, from.date().dayOfYear() );
  setByPos( ( from.date().day() - 1 ) / 7 + 1,
            from.date().dayOfWeek() - 1, from.date().month() );
  setByMonth( from.date().day(), from.date().month() );
}

void IncidenceRecurrenceEditor::setDuration( int duration )
{
  if ( duration == -1 ) {
    mUi->mNoEndDateButton->setChecked( true );
  } else if ( duration == 0 ) {
    mUi->mEndDateButton->setChecked( true );
  } else {
    mUi->mEndDurationButton->setChecked( true );
    mUi->mEndDurationEdit->setValue( duration );
  }
}

void IncidenceRecurrenceEditor::setExceptionDates( const DateList &dates )
{
  mUi->mExceptionList->clear();
  mExceptionDates.clear();
  DateList::ConstIterator dit;
  for ( dit = dates.begin(); dit != dates.end(); ++dit ) {
    mUi->mExceptionList->addItem( KGlobal::locale()->formatDate(* dit ) );
    mExceptionDates.append( *dit );
  }
}


void IncidenceRecurrenceEditor::setFrequency( int f )
{
  if ( f < 1 )
    f = 1;

  mUi->mFrequencyEdit->setValue( f );
}

void IncidenceRecurrenceEditor::setRecurrenceEnabled( bool enabled )
{
  mUi->mEnabledCheck->setChecked( enabled );
  mUi->mTimeGroupBox->setEnabled( enabled );
  mUi->mRuleBox->setEnabled( enabled );
  mUi->mRecurrenceRangeGroupBox->setEnabled( enabled );
  mUi->mExceptionsGroupBox->setEnabled( enabled );
}

void IncidenceRecurrenceEditor::setType( RecurrenceType type )
{
  switch ( type ) {
  case Daily:
    mUi->mDailyButton->setChecked( true );
    break;
  case Weekly:
    mUi->mWeeklyButton->setChecked( true );
    break;
  case Monthly:
    mUi->mMonthlyButton->setChecked( true );
    break;
  case Yearly:
  default:
    mUi->mYearlyButton->setChecked( true );
    break;
  }
}

