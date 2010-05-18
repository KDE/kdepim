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

#include "incidencedatetimeeditor.h"

#include <KCal/ICalTimeZones>
#include <KCal/IncidenceFormatter>

#include "incidencerecurrencedialog.h"
#include "ui_incidencedatetime.h"

using namespace IncidenceEditorsNG;
using namespace KCal;

IncidenceDateTimeEditor::IncidenceDateTimeEditor( QWidget *parent )
  : IncidenceEditor( parent )
  , mTimeZones( new ICalTimeZones )
  , mUi( new Ui::IncidenceDateTimeEditor )
  , mStartDateModified( false )
{
  mUi->setupUi( this );
  mUi->mAlarmBell->setPixmap( SmallIcon( "task-reminder" ) );
  mUi->mRecurrenceEditButton->setIcon(
    KIconLoader::global()->loadIcon(
      "task-recurring", KIconLoader::Desktop, KIconLoader::SizeSmall ) );

  connect( mUi->mRecurrenceEditButton, SIGNAL(clicked()), SLOT(editRecurrence()) );
}

IncidenceDateTimeEditor::~IncidenceDateTimeEditor()
{
  delete mTimeZones;
}


void IncidenceDateTimeEditor::load( KCal::Incidence::ConstPtr incidence )
{
  mLoadedIncidence = incidence;

  // We can only handle events or todos.
  if ( KCal::Todo::ConstPtr todo = IncidenceDateTimeEditor::incidence<Todo>() ) {
    load( todo );
  } else {
    KCal::Event::ConstPtr event = IncidenceDateTimeEditor::incidence<Event>();
    Q_ASSERT( event );
  }

  mWasDirty = false;
}

void IncidenceDateTimeEditor::save( KCal::Incidence::Ptr incidence )
{ }

bool IncidenceDateTimeEditor::isDirty() const
{
  if ( KCal::Todo::ConstPtr todo = IncidenceDateTimeEditor::incidence<Todo>() ) {
    return isDirty( todo );
  } else {
//     KCal::Event::ConstPtr event = IncidenceDateTimeEditor::incidence<Event>();
//     eventIsDirty( event );
    return false;
  }
}

/// private slots for General

void IncidenceDateTimeEditor::editRecurrence()
{
  QPointer<IncidenceRecurrenceDialog> dialog( new IncidenceRecurrenceDialog( this ) );
  dialog->exec();
  delete dialog;
}

void IncidenceDateTimeEditor::enableAlarm( bool enable )
{
  mUi->mAlarmStack->setEnabled( enable );
  mUi->mAlarmEditButton->setEnabled( enable );
}

void IncidenceDateTimeEditor::updateRecurrenceSummary( KCal::Incidence::ConstPtr incidence )
{
  if ( incidence->recurs() ) {
    mUi->mRecurrenceLabel->setText( IncidenceFormatter::recurrenceString( const_cast<Incidence *>( incidence.get() ) ) );
  } else {
    mUi->mRecurrenceLabel->setText( QString() );
  }
}

/// private slots for Todo

void IncidenceDateTimeEditor::enableStartEdit( bool enable )
{
  mUi->mStartDateEdit->setEnabled( enable );

  if( mUi->mEndCheck->isChecked() || mUi->mStartCheck->isChecked() ) {
    mUi->mHasTimeCheck->setEnabled( true );
  } else {
    mUi->mHasTimeCheck->setEnabled( false );
    mUi->mHasTimeCheck->setChecked( false );
  }

  if ( enable ) {
    mUi->mStartTimeEdit->setEnabled( mUi->mHasTimeCheck->isChecked() );
    mUi->mTimeZoneComboStart->setEnabled( mUi->mHasTimeCheck->isChecked() );
  } else {
    mUi->mStartTimeEdit->setEnabled( false );
    mUi->mTimeZoneComboStart->setEnabled( false );
  }

  mUi->mTimeZoneComboStart->setFloating( !mUi->mTimeZoneComboStart->isEnabled() );
  checkDirtyStatus();
}

void IncidenceDateTimeEditor::enableEndEdit( bool enable )
{
  mUi->mEndDateEdit->setEnabled( enable );

  if( mUi->mEndCheck->isChecked() || mUi->mStartCheck->isChecked() ) {
    mUi->mHasTimeCheck->setEnabled( true );
  } else {
    mUi->mHasTimeCheck->setEnabled( false );
  }

  if ( enable ) {
    mUi->mEndTimeEdit->setEnabled( mUi->mHasTimeCheck->isChecked() );
    mUi->mTimeZoneComboEnd->setEnabled( mUi->mHasTimeCheck->isChecked() );
  } else {
    mUi->mEndTimeEdit->setEnabled( false );
    mUi->mTimeZoneComboEnd->setEnabled( false );
  }

  mUi->mTimeZoneComboEnd->setFloating( !mUi->mTimeZoneComboEnd->isEnabled() );
  checkDirtyStatus();
}

void IncidenceDateTimeEditor::enableTimeEdits( bool enable )
{
  if( mUi->mStartCheck->isChecked() ) {
    mUi->mStartTimeEdit->setEnabled( enable );
    mUi->mTimeZoneComboStart->setEnabled( enable );
    mUi->mTimeZoneComboStart->setFloating( !enable, mStartSpec );
  }
  if( mUi->mEndCheck->isChecked() ) {
    mUi->mEndTimeEdit->setEnabled( enable );
    mUi->mTimeZoneComboEnd->setEnabled( enable );
    mUi->mTimeZoneComboEnd->setFloating( !enable, mEndSpec );
  }
}

bool IncidenceDateTimeEditor::isDirty( KCal::Todo::ConstPtr todo ) const
{
  Q_ASSERT( todo );

  // First check the start time/date of the todo
  if ( todo->hasStartDate() != mUi->mStartCheck->isChecked() )
    return true;

  if ( mUi->mStartCheck->isChecked() ) {
    KDateTime startDT = todo->dtStart();
    // TODO: Integrate date, whereever it comes from.
//     if ( todo->recurs() && date.isValid() && todo->hasDueDate() ) {
//       int days = todo->dtStart( true ).daysTo( todo->dtDue( true ) );
//       startDT.setDate( dueDT.date().addDays( -days ) );
//     }
    if ( startDT.isUtc() )
      startDT = startDT.toLocalZone();

    if ( mUi->mStartDateEdit->date() != startDT.date() )
      return true;

    if ( mUi->mStartTimeEdit->time() != startDT.time() )
      return true;

    // Use mStartSpec. This is the KDateTime::Spec selected on load comming from
    // the combobox. We use this one as it can slightly differ (e.g. missing
    // country code in the incidence time spec) from the incidence.
    if ( mUi->mTimeZoneComboStart->selectedTimeSpec() != mStartSpec )
      return true;
  }

  if ( todo->hasDueDate() != mUi->mEndCheck->isChecked() )
    return true;

  if ( mUi->mEndCheck->isChecked() ) {
    KDateTime dueDT = todo->dtDue();
        // TODO: Integrate date, whereever it comes from.
//     if ( todo->recurs() && date.isValid() ) {
//       KDateTime dt( date, QTime( 0, 0, 0 ) );
//       dt = dt.addSecs( -1 );
//       dueDT.setDate( todo->recurrence()->getNextDateTime( dt ).date() );
//     }
    if ( dueDT.isUtc() )
        dueDT = dueDT.toLocalZone();

    if ( mUi->mEndDateEdit->date() != dueDT.date() )
      return true;

    if ( mUi->mEndTimeEdit->time() != dueDT.time() )
      return true;

    // Use mEndSpec. This is the KDateTime::Spec selected on load comming from
    // the combobox. We use this one as it can slightly differ (e.g. missing
    // country code in the incidence time spec) from the incidence.
    if ( mUi->mTimeZoneComboEnd->selectedTimeSpec() != mEndSpec )
      return true;
  }

  return false;
}

// TODO: Investigate if we need this at all.
// void IncidenceDateTimeEditor::slotTodoDateChanged()
// {
//   KLocale *l = KGlobal::locale();
//   QString dateTimeStr = "";
// 
//   if ( mUi->mStartCheck->isChecked() ) {
//     dateTimeStr += i18nc( "to-do start datetime",
//                           "Start: %1", l->formatDate( mUi->mStartDateEdit->date() ) );
//     if ( mUi->mHasTimeCheck->isChecked() ) {
//       dateTimeStr += QString( " %1" ).arg( l->formatTime( mUi->mStartTimeEdit->time() ) );
//       dateTimeStr += ' ';
//       dateTimeStr += mUi->mTimeZoneComboStart->selectedTimeSpec().timeZone().name();
//     }
//   }
// 
//   if ( mUi->mEndCheck->isChecked() ) {
//     dateTimeStr += i18nc( "to-do due datetime", "   Due: %1",
//                           l->formatDate( mUi->mEndDateEdit->date() ) );
//     if ( mUi->mHasTimeCheck->isChecked() ) {
//       dateTimeStr += QString( " %1" ).arg( l->formatTime( mUi->mEndTimeEdit->time() ) );
//       dateTimeStr += ' ';
//       dateTimeStr += mUi->mTimeZoneComboEnd->selectedTimeSpec().timeZone().name();
//     }
//   }
// 
//   mEndSpec = mUi->mTimeZoneComboEnd->selectedTimeSpec();
// 
//   // TODO: Investigate whether we really need all those signals.
// //   emit dateTimeStrChanged( dateTimeStr );
// //   QDateTime endDt( mDueDateEdit->date(), mDueTimeEdit->time() );
// //   emit signalDateTimeChanged( endDt, endDt );
// }

void IncidenceDateTimeEditor::slotTodoStartDateModified()
{
  mStartDateModified = true;
//   slotTodoDateChanged();
  checkDirtyStatus();
}


/// Private methods

void IncidenceDateTimeEditor::load( KCal::Event::ConstPtr event )
{
  // First en/disable the necessary ui bits and pieces
  mUi->mStartLabel->setVisible( true );
  mUi->mEndLabel->setVisible( true );
  mUi->mStartCheck->setVisible( false );
  mUi->mEndCheck->setVisible( false );
}

void IncidenceDateTimeEditor::load( KCal::Todo::ConstPtr todo )
{
  // First en/disable the necessary ui bits and pieces
  mUi->mStartLabel->setVisible( false );
  mUi->mEndLabel->setVisible( false );
  mUi->mStartCheck->setVisible( true );
  mUi->mEndCheck->setVisible( true );

  // These fields where not enabled in the old code either:
  mUi->mDurationExplLabel->setVisible( false );
  mUi->mDurationLabel->setVisible( false );
  mUi->mFreeBusyLabel->setVisible( false );
  mUi->mFreeBusyCombo->setVisible( false );

  mUi->mHasTimeCheck->setChecked( !todo->allDay() );

  // Connect to the right logic
  connect( mUi->mStartCheck, SIGNAL(toggled(bool)), SLOT(enableStartEdit(bool)) );
  connect( mUi->mStartDateEdit, SIGNAL(dateChanged(QDate)), SLOT(slotTodoStartDateModified()) );
  connect( mUi->mStartTimeEdit, SIGNAL(timeChanged(const QTime&)), SLOT(slotTodoStartDateModified()) );
  connect( mUi->mTimeZoneComboStart, SIGNAL(currentIndexChanged(int)), SLOT(slotTodoStartDateModified()) );

  connect( mUi->mEndCheck, SIGNAL(toggled(bool)), SLOT(enableEndEdit(bool)) );
  connect( mUi->mEndCheck, SIGNAL(toggled(bool)), SLOT(enableAlarm(bool)) );
  //   connect( mDueCheck, SIGNAL(toggled(bool)), SIGNAL(dueDateEditToggle(bool)) );
  connect( mUi->mEndDateEdit, SIGNAL(dateChanged(QDate)), SLOT(checkDirtyStatus()) );
  connect( mUi->mEndTimeEdit, SIGNAL(timeChanged(const QTime&)), SLOT(checkDirtyStatus()) );
  connect( mUi->mTimeZoneComboEnd, SIGNAL(currentIndexChanged(int)), SLOT(checkDirtyStatus()) );

  connect( mUi->mHasTimeCheck, SIGNAL(toggled(bool)), SLOT(enableTimeEdits(bool)));

  //TODO: do something with tmpl, note: this wasn't used in the old code either.
//   Q_UNUSED( tmpl );

  if ( todo->hasStartDate() ) {
    KDateTime startDT = todo->dtStart();
    // TODO: Integrate date, whereever it comes from.
//     if ( todo->recurs() && date.isValid() && todo->hasDueDate() ) {
//       int days = todo->dtStart( true ).daysTo( todo->dtDue( true ) );
//       startDT.setDate( dueDT.date().addDays( -days ) );
//     }
    if ( startDT.isUtc() )
      startDT = startDT.toLocalZone();

    mUi->mStartDateEdit->setDate( startDT.date() );
    mUi->mStartTimeEdit->setTime( startDT.time() );
    mUi->mStartCheck->setChecked( true );
    mUi->mTimeZoneComboStart->selectTimeSpec( todo->dtStart().timeSpec() );
    mStartSpec = mUi->mTimeZoneComboStart->selectedTimeSpec();
  } else {
    mUi->mStartDateEdit->setEnabled( false );
    mUi->mStartTimeEdit->setEnabled( false );
    mUi->mStartDateEdit->setDate( QDate::currentDate() );
    mUi->mStartTimeEdit->setTime( QTime::currentTime() );
    mUi->mStartCheck->setChecked( false );
    mUi->mTimeZoneComboStart->setEnabled( false );
  }

  if ( todo->hasDueDate() ) {
    enableAlarm( true );
    // TODO: Integrate date, whereever it comes from.
//     if ( todo->recurs() && date.isValid() ) {
//       KDateTime dt( date, QTime( 0, 0, 0 ) );
//       dt = dt.addSecs( -1 );
//       dueDT.setDate( todo->recurrence()->getNextDateTime( dt ).date() );
//     }
    KDateTime dueDT = todo->dtDue();
    if ( dueDT.isUtc() )
      dueDT = dueDT.toLocalZone();

    mUi->mEndDateEdit->setDate( dueDT.date() );
    mUi->mEndTimeEdit->setTime( dueDT.time() );
    mUi->mEndCheck->setChecked( true );
    mUi->mTimeZoneComboEnd->selectTimeSpec( todo->dtDue().timeSpec() );
    mEndSpec = mUi->mTimeZoneComboEnd->selectedTimeSpec();
  } else {
    enableAlarm( false );
    mUi->mEndDateEdit->setEnabled( false );
    mUi->mEndTimeEdit->setEnabled( false );
    mUi->mEndDateEdit->setDate( QDate::currentDate() );
    mUi->mEndTimeEdit->setTime( QTime::currentTime() );
    mUi->mEndCheck->setChecked( false );
    mUi->mTimeZoneComboEnd->setEnabled( false );
  }

  updateRecurrenceSummary( todo );
}
