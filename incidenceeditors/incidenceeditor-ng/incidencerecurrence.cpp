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

#include "incidencerecurrence.h"

#include <QtCore/QDebug>
#include <QtGui/QListWidgetItem>

#include <KCalendarSystem>

#include "incidencedatetime.h"

#ifdef KDEPIM_MOBILE_UI
#include "ui_eventortodomoremobile.h"
#else
#include "ui_eventortododesktop.h"
#endif

using namespace IncidenceEditorsNG;

/// Keep this in sync with the values in mUi->mRecurrenceTypeCombo
static const int sRecurrenceNeverIndex = 0;
static const int sRecurrenceDailyIndex = 1;
static const int sRecurrenceWeeklyIndex = 2;
static const int sRecurrenceMonthlyIndex = 3;
static const int sRecurrenceYearlyIndex = 3;

#ifdef KDEPIM_MOBILE_UI
IncidenceRecurrence::IncidenceRecurrence( IncidenceDateTime *dateTime, Ui::EventOrTodoMore *ui )
#else
IncidenceRecurrence::IncidenceRecurrence( IncidenceDateTime *dateTime, Ui::EventOrTodoDesktop *ui )
#endif
  : mUi( ui )
  , mDateTime( dateTime )
{
  toggleRecurrenceWidgets( false );
  fillCombos();

  connect( mDateTime, SIGNAL(startDateChanged(QDate)),
           SLOT(fillCombos()) );
  connect( mDateTime, SIGNAL(startDateChanged(QDate)), mUi->mExceptionDateEdit,
           SLOT(setDate(QDate)) );
  connect( mUi->mExceptionAddButton, SIGNAL(clicked()),
           SLOT(addException()));
  connect( mUi->mExceptionRemoveButton, SIGNAL(clicked()),
           SLOT(removeExceptions()) );
  connect( mUi->mExceptionDateEdit, SIGNAL(dateChanged(QDate)),
           SLOT(handleExceptionDateChange(QDate)) );
  connect( mUi->mExceptionList, SIGNAL(itemSelectionChanged()),
           SLOT(updateRemoveExceptionButton()) );
  connect( mUi->mRecurrenceTypeCombo, SIGNAL(currentIndexChanged(int)),
           SLOT(handleRecurrenceTypeChange(int)));
  connect( mUi->mFrequencyEdit, SIGNAL(valueChanged(int)),
           SLOT(handleFrequencyChange()) );
}

void IncidenceRecurrence::load( KCal::Incidence::ConstPtr incidence )
{
  Q_ASSERT( incidence );

  mLoadedIncidence = incidence;
  // We must be sure that the date/time in mDateTime is the correct date time.
  // So don't depend on CombinedIncidenceEditor or whatever external factor to
  // load the date/time before loading the recurrence
  mDateTime->load( incidence );
  fillCombos();
  setDefaults();

  int f = 0;
  KCal::Recurrence *r = 0;
  if ( mLoadedIncidence->recurrenceType() != KCal::Recurrence::rNone ) {
    r = mLoadedIncidence->recurrence();
    f = r->frequency();
  }

  switch ( mLoadedIncidence->recurrenceType() ) {
  case KCal::Recurrence::rNone:
    handleRecurrenceTypeChange( sRecurrenceNeverIndex );
    break;
  case KCal::Recurrence::rDaily:
    handleRecurrenceTypeChange( sRecurrenceDailyIndex );
    setFrequency( f );
    break;
  case KCal::Recurrence::rWeekly:
    handleRecurrenceTypeChange( sRecurrenceWeeklyIndex );
    setDays( r->days() );
    setFrequency( f );
    break;
  case KCal::Recurrence::rMonthlyPos: // Fall through
  case KCal::Recurrence::rMonthlyDay:
  {
    handleRecurrenceTypeChange( sRecurrenceMonthlyIndex );
    selectMonthlyItem( r, mLoadedIncidence->recurrenceType() );
    setFrequency( f );
    break;
  }
  case KCal::Recurrence::rYearlyMonth: // Fall through
  case KCal::Recurrence::rYearlyPos:   // Fall through
  case KCal::Recurrence::rYearlyDay:
  {
    handleRecurrenceTypeChange( sRecurrenceYearlyIndex );
    selectYearlyItem( r, mLoadedIncidence->recurrenceType() );
    setFrequency( f );
    break;
  }
  default:
    break;
  }

  if ( mLoadedIncidence->recurs() && r ) {
    setDuration( r->duration() );
    if ( r->duration() == 0 )
      mUi->mRecurrenceEndDate->setDate( r->endDate() );
  }

  setExceptionDates( mLoadedIncidence->recurrence()->exDates() );
}

void IncidenceRecurrence::save( KCal::Incidence::Ptr incidence )
{
  // clear out any old settings;
  KCal::Recurrence *r = incidence->recurrence();
  r->unsetRecurs(); // Why not clear() ?

  if ( mUi->mRecurrenceTypeCombo->currentIndex() == sRecurrenceNeverIndex )
    return;

  const int lDuration = duration();
  QDate endDate;
  if ( lDuration == 0 )
    endDate = mUi->mRecurrenceEndDate->date();

  int recurrenceType = mUi->mRecurrenceTypeCombo->currentIndex();
  if ( recurrenceType == sRecurrenceDailyIndex ) {
    r->setDaily( mUi->mFrequencyEdit->value() );
  } else if ( recurrenceType == sRecurrenceWeeklyIndex ) {
    r->setWeekly( mUi->mFrequencyEdit->value(), days() );
  } else if ( recurrenceType == sRecurrenceMonthlyIndex ) {
    r->setMonthly( mUi->mFrequencyEdit->value() );

    if ( mUi->mMonthlyCombo->currentIndex() == 0 )      // Every nth
      r->addMonthlyDate( dayOfMonthFromStart() );
    else if ( mUi->mMonthlyCombo->currentIndex() == 1 ) // Every (last - n)th last day
      r->addMonthlyDate( -dayOfMonthFromEnd() );
    else if ( mUi->mMonthlyCombo->currentIndex() == 2 ) // Every ith weekday
      r->addMonthlyPos( monthWeekFromStart(), weekday() );
    else // Every (last - i)th last weekday
      r->addMonthlyPos( -monthWeekFromEnd(), weekday() );

  } else if ( recurrenceType == sRecurrenceYearlyIndex ) {
    r->setYearly( mUi->mFrequencyEdit->value() );

    if ( mUi->mYearlyCombo->currentIndex() == 0 ) {        // Every nth of month
      r->addYearlyDate( dayOfMonthFromStart() );
      r->addYearlyMonth( mDateTime->startDate().month() );
    } else if ( mUi->mYearlyCombo->currentIndex() == 1 ) { // Every (last - n)th last day of month
      r->addYearlyDate( dayOfMonthFromEnd() );
      r->addYearlyMonth( mDateTime->startDate().month() );
    } else if ( mUi->mYearlyCombo->currentIndex() == 2 ) { // Every ith weekday of month
      r->addYearlyMonth( mDateTime->startDate().month() );
      r->addYearlyPos( monthWeekFromStart(), weekday() );
    } else if ( mUi->mYearlyCombo->currentIndex() == 3 ) { // Every (last - i)th last weekday of month
      r->addYearlyMonth( mDateTime->startDate().month() );
      r->addYearlyPos( monthWeekFromEnd(), weekday() );
    } else // The lth day of the year (l : 1 - 356)
      r->addYearlyDay( dayOfYearFromStart() );
  }

  if ( lDuration > 0 ) {
    r->setDuration( lDuration );
  } else if ( lDuration == 0 ) {
    r->setEndDate( endDate );
  }

  r->setExDates( mExceptionDates );

}

bool IncidenceRecurrence::isDirty() const
{
  return false;
}

bool IncidenceRecurrence::isValid()
{
  return true;
}

void IncidenceRecurrence::addException()
{
  const QDate date = mUi->mExceptionDateEdit->date();
  const QString dateStr = KGlobal::locale()->formatDate( date );
  if( mUi->mExceptionList->findItems( dateStr, Qt::MatchExactly ).isEmpty() ) {
    mExceptionDates.append( date );
    mUi->mExceptionList->addItem( dateStr );
  }

  mUi->mExceptionAddButton->setEnabled( false );
}

void IncidenceRecurrence::fillCombos()
{
  // First fill the weekly combo, but only when it is not empy because it is not
  // dependend on the start day of the event
  const KCalendarSystem *calSys = KGlobal::locale()->calendar();
  const int weekStart = KGlobal::locale()->weekStartDay();
  if (mUi->mWeekDayCombo->count() == 0 ) {
    for ( int i = 0; i < 7; ++i ) {
      // i is the nr of the combobox, not the day of week!
      // label=(i+weekStart+6)%7 + 1;
      // index in CheckBox array(=day): label-1
      const int index = ( i + weekStart + 6 ) % 7;

      QString weekDayName = calSys->weekDayName( index + 1, KCalendarSystem::ShortDayName );
      mUi->mWeekDayCombo->addItem( weekDayName );
    }
  }

  // Next the monthly combo. This contains the following elements:
  // - nth day of the month
  // - (month.lastDay() - n)th day of the month
  // - the ith ${weekday} of the month
  // - the (month.weekCount() - i)th day of the month
  const int currentMonthlyIndex = mUi->mMonthlyCombo->currentIndex();
  mUi->mMonthlyCombo->clear();
  const QDate startDate = mDateTime->startDate();
  QString item = "the " + numberToString( dayOfMonthFromStart() );
  mUi->mMonthlyCombo->addItem( item );

  item = "the " + numberToString( dayOfMonthFromEnd() ) + " last day";
  mUi->mMonthlyCombo->addItem( item );

  item = "the " + numberToString( monthWeekFromStart() ) + ' ' + calSys->weekDayName( startDate.dayOfWeek(), KCalendarSystem::LongDayName );
  mUi->mMonthlyCombo->addItem( item );

  if ( monthWeekFromEnd() == 1 )
    item = "the last " + calSys->weekDayName( startDate.dayOfWeek(), KCalendarSystem::LongDayName );
  else
    item = "the " + numberToString( monthWeekFromEnd() ) + " last " + calSys->weekDayName( startDate.dayOfWeek(), KCalendarSystem::LongDayName );
  mUi->mMonthlyCombo->addItem( item );
  mUi->mMonthlyCombo->setCurrentIndex( currentMonthlyIndex == -1 ? 0 : currentMonthlyIndex );

  // Finally the yearly combo. This contains the following options:
  // - ${n}th of ${long-month-name}
  // - ${month.lastDay() - n}th last day of ${long-month-name}
  // - the ${i}th ${weekday} of ${long-month-name}
  // - the ${month.weekCount() - i}th day of ${long-month-name}
  // - the ${m}th day of the year
  const int currentYearlyIndex = mUi->mYearlyCombo->currentIndex();
  mUi->mYearlyCombo->clear();
  const QString longMonthName = calSys->monthName( startDate );
  item = "the " + numberToString( startDate.day() ) + " of " + longMonthName;
  mUi->mYearlyCombo->addItem( item );

  item = "the " + numberToString( startDate.daysInMonth() - startDate.day() ) + " last day of " + longMonthName;
  mUi->mYearlyCombo->addItem( item );

  item = "the " + numberToString( monthWeekFromStart() ) + ' ' + calSys->weekDayName( startDate.dayOfWeek(), KCalendarSystem::LongDayName ) + " of " + longMonthName;
  mUi->mYearlyCombo->addItem( item );

  if ( monthWeekFromEnd() == 1 )
    item = "the last " + calSys->weekDayName( startDate.dayOfWeek(), KCalendarSystem::LongDayName );
  else
    item = "the " + numberToString( monthWeekFromEnd() ) + " last " + calSys->weekDayName( startDate.dayOfWeek(), KCalendarSystem::LongDayName ) + " of " + longMonthName;
  mUi->mYearlyCombo->addItem( item );

  item = "the " + numberToString( startDate.dayOfYear() ) + " day of the year";
  mUi->mYearlyCombo->addItem( item );
  mUi->mYearlyCombo->setCurrentIndex( currentYearlyIndex == -1 ? 0 : currentYearlyIndex );
}

void IncidenceRecurrence::handleExceptionDateChange( const QDate &currentDate )
{
  const QDate date = mUi->mExceptionDateEdit->date();
  const QString dateStr = KGlobal::locale()->formatDate( date );

  mUi->mExceptionAddButton->setEnabled( currentDate >= mDateTime->startDate()
                                        && mUi->mExceptionList->findItems( dateStr, Qt::MatchExactly ).isEmpty() );
}

void IncidenceRecurrence::handleFrequencyChange()
{
  handleRecurrenceTypeChange( mUi->mRecurrenceTypeCombo->currentIndex() );
}

void IncidenceRecurrence::handleRecurrenceTypeChange( int currentIndex )
{
  toggleRecurrenceWidgets( currentIndex > 0 );
  QString text;
  switch ( currentIndex ) {
  case 2:
    text = i18ncp( "Event recurs every n week(s)", "week", "weeks", mUi->mFrequencyEdit->value() );
    break;
  case 3:
    text = i18ncp( "Event recurs every n month(s)", "month", "months", mUi->mFrequencyEdit->value() );
    break;
  case 4:
    text = i18ncp( "Event recurs every n year(s)", "year", "years", mUi->mFrequencyEdit->value() );
    break;
  default:
    text = i18ncp( "Event recurs every n day(s)", "day", "days", mUi->mFrequencyEdit->value() );
    break;
  }

  mUi->mRecurrenceRuleLabel->setText( text );
}

void IncidenceRecurrence::removeExceptions()
{
  QList<QListWidgetItem *> selectedExceptions = mUi->mExceptionList->selectedItems();
  foreach ( QListWidgetItem *selectedException, selectedExceptions ) {
    const int row = mUi->mExceptionList->row( selectedException );
    mExceptionDates.removeAt( row );
    delete mUi->mExceptionList->takeItem( row );
  }

  handleExceptionDateChange( mUi->mExceptionDateEdit->date() );
}

void IncidenceRecurrence::updateRemoveExceptionButton()
{
  mUi->mExceptionRemoveButton->setEnabled( mUi->mExceptionList->selectedItems().count() > 0 );
}

short IncidenceRecurrence::dayOfMonthFromStart() const
{
  return mDateTime->startDate().day();
}

short IncidenceRecurrence::dayOfMonthFromEnd() const
{
  const QDate start = mDateTime->startDate();
  return start.daysInMonth() - start.day();
}

short IncidenceRecurrence::dayOfYearFromStart() const
{
  return mDateTime->startDate().dayOfYear();
}

QBitArray IncidenceRecurrence::days() const
{
  QBitArray days( 7 );
  const int weekStart = KGlobal::locale()->weekStartDay();

  for ( int i = 0; i < 7; ++i ) {
    // i is the nr of the combobox, not the day of week!
    // label=(i+weekStart+6)%7 + 1;
    // index in CheckBox array(=day): label-1
    const int index = ( i + weekStart + 6 ) % 7;
    days.setBit( i, mUi->mWeekDayCombo->itemCheckState( index ) == Qt::Checked );
  }

  return days;
}

int IncidenceRecurrence::duration() const
{
  if ( mUi->mRecurrenceEndCombo->currentIndex() == 0 ) {
    return -1;
  } else if ( mUi->mRecurrenceEndCombo->currentIndex() == 1 ) {
    return mUi->mEndDurationEdit->value();
  } else {
    return 0;
  }
}

short IncidenceRecurrence::monthWeekFromStart() const
{
  QDate date = mDateTime->startDate();

  int count = 1;
  QDate tmp = date.addDays( -7 );
  while ( tmp.month() == date.month() ) {
    tmp = tmp.addDays( -7 ); // Count backward
    ++count;
  }

  // 1 is the first week, 4/5 is the last week of the month
  return count;
}

short IncidenceRecurrence::monthWeekFromEnd() const
{
  QDate date = mDateTime->startDate();

  int count = 1;
  QDate tmp = date.addDays( 7 );
  while ( tmp.month() == date.month() ) {
    tmp = tmp.addDays( 7 );  // Count forward
    ++count;
  }

  // 1 is the last week, 4/5 is the first week of the month
  return count;
}


QString IncidenceRecurrence::numberToString( int number ) const
{
  QString result = QString::number( number );
  if ( result.endsWith( '1' ) ) {
    if ( result.endsWith( "11" ) ) {
      return "th";
    } else {
      return result + "st";
    }
  }

  if ( result.endsWith( '2' ) )
    return result + "nd";

  if ( result.endsWith( '3' ) )
    return result + "rd";
  else
    return result + "th";
}

void IncidenceRecurrence::selectMonthlyItem( KCal::Recurrence *recurrence, ushort recurenceType )
{
  Q_ASSERT( recurenceType == KCal::Recurrence::rMonthlyPos || recurenceType == KCal::Recurrence::rMonthlyDay );

  if ( recurenceType == KCal::Recurrence::rMonthlyPos ) {
    QList<KCal::RecurrenceRule::WDayPos> rmp = recurrence->monthPositions();
    if ( !rmp.isEmpty() )
      return; // Use the default values. Probably marks the editor as dirty

    if ( rmp.first().pos() > 0 ) { // nth day
      // TODO if ( rmp.first().pos() != mDateTime->startDate().day() ) { warn user }
      // NOTE: This silencly changes the recurrence when:
      //       rmp.first().pos() != mDateTime->startDate().day()
      mUi->mMonthlyCombo->setCurrentIndex( 0 );
    } else { // (month.last() - n)th day
      // TODO: Handle recurrences we cannot represent
      // QDate startDate = mDateTime->startDate();
      // const int dayFromEnd = startDate.daysInMonth() - startDate.day();
      // if ( qAbs( rmp.first().pos() ) != dayFromEnd ) { /* warn user */ }
      mUi->mMonthlyCombo->setCurrentIndex( 1 );
    }
  } else { // Monthly by day

    // check if we have any setting for which day (vcs import is broken and
    // does not set any day, thus we need to check)
    int day = mDateTime->startDate().day();
    if ( !recurrence->monthDays().isEmpty() )
      day = recurrence->monthDays().first();

    // Days from the end are after the ones from the begin, so correct for the
    // negative sign and add 30 (index starting at 0)
    // TODO: Do similar checks as in the monthlyPos case
    if ( day > 0 && day <= 31 ) {
      mUi->mMonthlyCombo->setCurrentIndex( 2 );
    } else if ( day < 0 ) {
      mUi->mMonthlyCombo->setCurrentIndex( 3 );
    }
  }
}

void IncidenceRecurrence::selectYearlyItem( KCal::Recurrence *recurrence, ushort recurenceType )
{
  Q_ASSERT( recurenceType == KCal::Recurrence::rYearlyDay
            || recurenceType == KCal::Recurrence::rYearlyMonth
            || recurenceType == KCal::Recurrence::rYearlyPos );

  if ( recurenceType == KCal::Recurrence::rYearlyDay ) {

    int day = mDateTime->startDate().dayOfYear();
    if ( !recurrence->yearDays().isEmpty() )
      day = recurrence->yearDays().first();

    // TODO Check if day has actually the same value as in the combo.
    mUi->mYearlyCombo->setCurrentIndex( 4 );

  } else if ( recurenceType == KCal::Recurrence::rYearlyMonth ) {

    int day = mDateTime->startDate().day();
    if ( !recurrence->yearDates().isEmpty() )
      day = recurrence->yearDates().first();

    int month = mDateTime->startDate().month();
    if ( !recurrence->yearMonths().isEmpty() )
      month = recurrence->yearMonths().first();

    // TODO check month and day to be correct values with respect to what is
    //      presented in the combo box.

    if ( day < 0 )
      mUi->mYearlyCombo->setCurrentIndex( 0 );
    else
      mUi->mYearlyCombo->setCurrentIndex( 1 );

  } else { //KCal::Recurrence::rYearlyPos

    int month = mDateTime->startDate().month();
    if ( !recurrence->yearMonths().isEmpty() )
      month = recurrence->yearMonths().first();

    // count is the nth weekday of the month or the ith last weekday of the month.
    int count = ( mDateTime->startDate().day() - 1 ) / 7;
    int day = mDateTime->startDate().dayOfWeek();
    if ( !recurrence->yearPositions().isEmpty() ) {
      count = recurrence->yearPositions().first().pos();
      day = recurrence->yearPositions().first().day();
    }

    // TODO check month,count and day to be correct values with respect to what is
    //      presented in the combo box.

    if ( count > 0 )
      mUi->mYearlyCombo->setCurrentIndex( 2 );
    else
      mUi->mYearlyCombo->setCurrentIndex( 3 );
  }

}

void IncidenceRecurrence::setDays( const QBitArray &days )
{
  Q_ASSERT( mUi->mWeekDayCombo->count() == 7 ); // The combobox must be filled.

  QStringList checkedDays;
  const int weekStart = KGlobal::locale()->weekStartDay();
  for ( int i = 0; i < 7; ++i ) {
    // i is the nr of the combobox, not the day of week!
    // label=(i+weekStart+6)%7 + 1;
    // index in CheckBox array(=day): label-1
    if ( days.testBit( i ) ) {
      const int index = ( i + weekStart + 6 ) % 7;
      checkedDays << mUi->mWeekDayCombo->itemText( index );
    }
  }

  mUi->mWeekDayCombo->setCheckedItems( checkedDays );
}

void IncidenceRecurrence::setDefaults()
{
  mUi->mRecurrenceEndCombo->setCurrentIndex( 0 ); // Ends never
  mUi->mRecurrenceEndDate->setDate( mDateTime->startDate() );
  mUi->mRecurrenceTypeCombo->setCurrentIndex( sRecurrenceNeverIndex );

  setFrequency( 1 );

  QBitArray days( 7 );
  days.fill( 0 );
  days.setBit( ( mDateTime->startDate().dayOfWeek() + 6 ) % 7 );
  setDays( days );

  mUi->mMonthlyCombo->setCurrentIndex( 0 ); // Recur on the nth of the month
  mUi->mYearlyCombo->setCurrentIndex( 0 );  // Recur on the nth of the month
}

void IncidenceRecurrence::setDuration( int duration )
{
  if ( duration == -1 ) { // No end date
    mUi->mRecurrenceEndCombo->setCurrentIndex( 0 );
    mUi->mRecurrenceEndStack->setCurrentIndex( 0 );
  } else if ( duration == 0 ) {
    mUi->mRecurrenceEndCombo->setCurrentIndex( 1 );
    mUi->mRecurrenceEndStack->setCurrentIndex( 1 );
  } else {
    mUi->mRecurrenceEndCombo->setCurrentIndex( 2 );
    mUi->mRecurrenceEndStack->setCurrentIndex( 2 );
    mUi->mEndDurationEdit->setValue( duration );
  }
}

void IncidenceRecurrence::setExceptionDates( const KCal::DateList &dates )
{
  mUi->mExceptionList->clear();
  mExceptionDates.clear();
  KCal::DateList::ConstIterator dit;
  for ( dit = dates.begin(); dit != dates.end(); ++dit ) {
    mUi->mExceptionList->addItem( KGlobal::locale()->formatDate(* dit ) );
    mExceptionDates.append( *dit );
  }
}

void IncidenceRecurrence::setFrequency( int frequency )
{
  if ( frequency < 1 )
    frequency = 1;

  mUi->mFrequencyEdit->setValue( frequency );
}

void IncidenceRecurrence::toggleRecurrenceWidgets( bool enable )
{
  mUi->mFrequencyLabel->setVisible( enable );
  mUi->mFrequencyEdit->setVisible( enable );
  mUi->mRecurrenceRuleLabel->setVisible( enable );
  mUi->mRepeatStack->setVisible( enable && mUi->mRecurrenceTypeCombo->currentIndex() > 1 );
  mUi->mRecurrenceEndLabel->setVisible( enable );
  mUi->mRecurrenceEndCombo->setVisible( enable );
  mUi->mEndDurationEdit->setVisible( enable );
  mUi->mRecurrenceEndStack->setVisible( enable );

  // Exceptions widgets
  mUi->mExceptionsLabel->setVisible( enable );
  mUi->mExceptionDateEdit->setVisible( enable );
  mUi->mExceptionAddButton->setVisible( enable );
  mUi->mExceptionAddButton->setEnabled( mUi->mExceptionDateEdit->date() >= mDateTime->startDate() );
  mUi->mExceptionRemoveButton->setVisible( enable );
  mUi->mExceptionRemoveButton->setEnabled( mUi->mExceptionList->selectedItems().count() > 0 );
  mUi->mExceptionList->setVisible( enable );
}

QBitArray IncidenceRecurrence::weekday() const
{
  QBitArray days( 7 );
  // QDate::dayOfWeek() -> returns [1 - 7], 1 == monday
  days.setBit(mDateTime->startDate().dayOfWeek() - 1, true );
  return days;
}

int IncidenceRecurrence::weekdayCountForMonth( const QDate &date ) const
{
  Q_ASSERT( date.isValid() );
  // This methods returns how often the weekday specified by @param date occurs
  // in the month represented by @param date.

  int count = 1;
  QDate tmp = date.addDays( -7 );
  while ( tmp.month() == date.month() ) {
    tmp = tmp.addDays( -7 );
    ++count;
  }

  tmp = date.addDays( 7 );
  while ( tmp.month() == date.month() ) {
    tmp = tmp.addDays( 7 );
    ++count;
  }

  return count;
}
