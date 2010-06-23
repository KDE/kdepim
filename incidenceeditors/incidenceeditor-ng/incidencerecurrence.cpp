#include "incidencerecurrence.h"

#include <QtCore/QDebug>
#include <QtGui/QListWidgetItem>

#include <KCalendarSystem>

#include "incidencedatetime.h"
#include "ui_eventortododesktop.h"

using namespace IncidenceEditorsNG;

IncidenceRecurrence::IncidenceRecurrence( IncidenceDateTime *dateTime, Ui::EventOrTodoDesktop *ui )
  : mUi( ui )
  , mDateTime( dateTime )
{
  toggleRecurrenceWidgets( false );
  fillCombos();

  connect( mUi->mExceptionAddButton, SIGNAL(clicked()),
           SLOT(addException()));
  connect( mUi->mExceptionRemoveButton, SIGNAL(clicked()),
           SLOT(removeExceptions()) );
  connect( mUi->mExceptionDateEdit, SIGNAL(dateChanged(QDate)),
           SLOT(handleExceptionDateChange(QDate)) );
  connect( mUi->mExceptionList, SIGNAL(itemSelectionChanged()),
           SLOT(updateRemoveExceptionButton()) );
  connect( mUi->mTypeCombo, SIGNAL(currentIndexChanged(int)),
           SLOT(handleRecurrenceTypeChange(int)));
}

void IncidenceRecurrence::load( KCal::Incidence::ConstPtr incidence )
{

}

void IncidenceRecurrence::save( KCal::Incidence::Ptr incidence )
{

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

void IncidenceRecurrence::handleExceptionDateChange( const QDate &currentDate )
{
  const QDate date = mUi->mExceptionDateEdit->date();
  const QString dateStr = KGlobal::locale()->formatDate( date );

  mUi->mExceptionAddButton->setEnabled( currentDate >= mDateTime->startDate()
                                        && mUi->mExceptionList->findItems( dateStr, Qt::MatchExactly ).isEmpty() );
}

void IncidenceRecurrence::handleRecurrenceTypeChange( int currentIndex )
{
  toggleRecurrenceWidgets( currentIndex > 0 );
  switch ( currentIndex ) {
  case 1:
    mUi->mRecurrenceRuleLabel->setText( i18n("day(s)") );
    break;
  case 2:
    mUi->mRecurrenceRuleLabel->setText( i18n("week(s)") );
    break;
  case 3:
    mUi->mRecurrenceRuleLabel->setText( i18n("month(s)") );
    break;
  case 4:
    mUi->mRecurrenceRuleLabel->setText( i18n("year(s)") );
    break;
  default:
    mUi->mRecurrenceRuleLabel->setText( i18n("day(s)") );
  }
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
  mUi->mMonthlyCombo->clear();
  const QDate startDate = mDateTime->startDate();
  QString item = "the " + numberToString( startDate.day() );
  mUi->mMonthlyCombo->addItem( item );

  item = "the " + numberToString( startDate.daysInMonth() - startDate.day() ) + " last day";
  mUi->mMonthlyCombo->addItem( item );

  const int weekOfMonthNr = weekdayOfMonth( startDate );
  item = "the " + numberToString( weekOfMonthNr ) + ' ' + calSys->weekDayName( startDate.dayOfWeek(), KCalendarSystem::LongDayName );
  mUi->mMonthlyCombo->addItem( item );

  const int weekOfMonthNrFromEnd = weekdayCountForMonth( startDate ) + 1 - weekOfMonthNr;
  if ( weekOfMonthNrFromEnd == 1 )
    item = "the last " + calSys->weekDayName( startDate.dayOfWeek(), KCalendarSystem::LongDayName );
  else
    item = "the " + numberToString( weekOfMonthNrFromEnd ) + " last " + calSys->weekDayName( startDate.dayOfWeek(), KCalendarSystem::LongDayName );
  mUi->mMonthlyCombo->addItem( item );

  // Finally the yearly combo. This contains the following options:
  // - ${n}th of ${long-month-name}
  // - ${month.lastDay() - n}th last day of ${long-month-name}
  // - the ${i}th ${weekday} of ${long-month-name}
  // - the ${month.weekCount() - i}th day of ${long-month-name}
  // - the ${m}th day of the year
  mUi->mYearlyCombo->clear();
  const QString longMonthName = calSys->monthName( startDate );
  item = "the " + numberToString( startDate.day() ) + " of " + longMonthName;
  mUi->mYearlyCombo->addItem( item );

  item = "the " + numberToString( startDate.daysInMonth() - startDate.day() ) + " last day of " + longMonthName;
  mUi->mYearlyCombo->addItem( item );

  item = "the " + numberToString( weekOfMonthNr ) + ' ' + calSys->weekDayName( startDate.dayOfWeek(), KCalendarSystem::LongDayName ) + " of " + longMonthName;
  mUi->mYearlyCombo->addItem( item );

  if ( weekOfMonthNrFromEnd == 1 )
    item = "the last " + calSys->weekDayName( startDate.dayOfWeek(), KCalendarSystem::LongDayName );
  else
    item = "the " + numberToString( weekOfMonthNrFromEnd ) + " last " + calSys->weekDayName( startDate.dayOfWeek(), KCalendarSystem::LongDayName ) + " of " + longMonthName;
  mUi->mYearlyCombo->addItem( item );

  item = "the " + numberToString( startDate.dayOfYear() ) + " day of the year";
  mUi->mYearlyCombo->addItem( item );
}

QString IncidenceRecurrence::numberToString( int number ) const
{
  QString result = QString::number( number );
  if ( result.endsWith( '1' ) )
    if ( result.endsWith( "11" ) )
      return "th";
    else
      return result + "st";
  if ( result.endsWith( '2' ) )
    return result + "nd";
  if ( result.endsWith( '3' ) )
    return result + "rd";
  else
    return result + "th";
}

void IncidenceRecurrence::toggleRecurrenceWidgets( bool enable )
{
  mUi->mFrequencyLabel->setVisible( enable );
  mUi->mFrequencyEdit->setVisible( enable );
  mUi->mRecurrenceRuleLabel->setVisible( enable );
  mUi->mRepeatStack->setVisible( enable && mUi->mTypeCombo->currentIndex() > 1 );
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

int IncidenceRecurrence::weekdayCountForMonth( const QDate &date )
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

int IncidenceRecurrence::weekdayOfMonth( const QDate &date )
{
  Q_ASSERT( date.isValid() );
  // This methods returns the week number in the month containing @param date.
  // The numbering starts at 1.

  QDate prev = date.addDays( -7 );
  int weeknr = 1;
  while ( prev.month() == date.month() ) {
    prev = prev.addDays( -7 );
    ++weeknr;
  }

  return weeknr;
}
