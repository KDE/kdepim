/*
    Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

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

#include "incidencedatetimeeditor.h"

#include <KCal/ICalTimeZones>
#include <KCal/IncidenceFormatter>
#include <KDebug>
#include <KSystemTimeZones>

#include "incidencerecurrencedialog.h"
#include "incidencerecurrenceeditor.h"
#include "recurrencepresets.h"

//#ifdef KDEPIM_MOBILE_UI
//#include "ui_iedatetimemobile.h"
//#else
//#include "../editoralarms.h"
//#include "ui_incidencedatetime.h"
//#endif

#include "ui_eventortododesktop.h"
#include "../editoralarms.h"

using namespace IncidenceEditorsNG;
using namespace KCal;

IncidenceDateTimeEditor::IncidenceDateTimeEditor( Ui::EventOrTodoDesktop *ui )
  : IncidenceEditor( 0 )
  , mTimeZones( new ICalTimeZones )
  , mUi( ui )
  , mLastRecurrence( 0 )
{
  setTimeZonesVisibility( false );

#if 0 //def KDEPIM_MOBILE_UI
  QButtonGroup *freeBusyGroup = new QButtonGroup( this );
  freeBusyGroup->addButton( mUi->mFreeRadio );
  freeBusyGroup->addButton( mUi->mBusyRadio );

  mUi->mTimeZoneComboStart->setVisible( false );
  mUi->mTimeZoneComboEnd->setVisible( false );
#else
  mUi->mTimeZoneLabel->setVisible( !mUi->mWholeDayCheck->isChecked() );
  connect( mUi->mTimeZoneLabel, SIGNAL(linkActivated(QString)),
           SLOT(toggleTimeZoneVisibility()) );
#endif

//  mUi->mRecurrenceEditButton->setIcon( SmallIcon( "task-recurring" ) );
//  mUi->mRecurrenceCombo->insertItems( 1, RecurrencePresets::availablePresets() );
//  mUi->mRecurrenceEditButton->setEnabled( false );

//  connect( mUi->mRecurrenceCombo, SIGNAL(currentIndexChanged(int)),
//           SLOT(updateRecurrencePreset(int)) );
//  connect( mUi->mRecurrenceEditButton, SIGNAL(clicked()),
//           SLOT(editRecurrence()) );
  connect( mUi->mWholeDayCheck, SIGNAL(toggled(bool)),
           SLOT(checkDirtyStatus()) );
}

IncidenceDateTimeEditor::~IncidenceDateTimeEditor()
{
  delete mTimeZones;
  delete mLastRecurrence;
}

void IncidenceDateTimeEditor::load( KCal::Incidence::ConstPtr incidence )
{
  mLoadedIncidence = incidence;

  // We can only handle events or todos.
  if ( KCal::Todo::ConstPtr todo = IncidenceDateTimeEditor::incidence<Todo>() ) {
    load( todo );
  } else if ( KCal::Event::ConstPtr event = IncidenceDateTimeEditor::incidence<Event>() ) {
    load( event );
  } else {
    kDebug() << "Not an event or an todo.";
  }

  // Set the initial times before calling enableTimeEdits, as enableTimeEdits
  // assumes that the initial times are initialized.
  mInitialStartDT = currentStartDateTime();
  mInitialEndDT = currentEndDateTime();

  enableTimeEdits();

  if ( mUi->mTimeZoneComboStart->currentIndex() == 0 ) // Floating
    mInitialStartDT.setTimeSpec( mInitialStartDT.toLocalZone().timeSpec() );

  if ( mUi->mTimeZoneComboEnd->currentIndex() == 0 ) // Floating
    mInitialEndDT.setTimeSpec( mInitialEndDT.toLocalZone().timeSpec() );

  setTimeZonesVisibility( false );
  if ( mUi->mTimeZoneComboStart->isEnabled()
       && ( mUi->mTimeZoneComboStart->currentIndex() == 0
            || ! mUi->mTimeZoneComboStart->selectedTimeSpec().isLocalZone() ) )
    setTimeZonesVisibility( true );
  if ( mUi->mTimeZoneComboEnd->isEnabled()
       && ( mUi->mTimeZoneComboEnd->currentIndex() == 0
            || ! mUi->mTimeZoneComboEnd->selectedTimeSpec().isLocalZone() ) )
    setTimeZonesVisibility( true );

//  if ( incidence->recurs() ) {
//    // Note: we use a copy, because  mLastRecurrence gets deleted when the recurrence
//    //       change.
//    mLastRecurrence = new Recurrence( *incidence->recurrence() );
//    Q_ASSERT( *mLastRecurrence == *incidence->recurrence() );
//    KDateTime startDt = currentStartDateTime();
//    startDt.setDateOnly( mUi->mWholeDayCheck->isChecked() && mUi->mStartCheck->isChecked() );

//    const int index = RecurrencePresets::presetIndex( *mLastRecurrence, startDt );
//    if ( index == -1 ) { // Custom recurrence
//      mUi->mRecurrenceCombo->blockSignals( true );
//      mUi->mRecurrenceCombo->setCurrentIndex( mUi->mRecurrenceCombo->count() - 1 );
//      mUi->mRecurrenceCombo->blockSignals( false );
//    } else {
//      mUi->mRecurrenceCombo->blockSignals( true );
//      // Add one to cope with the "no recurrence" option in the combo, which is
//      // not in the presets.
//      mUi->mRecurrenceCombo->setCurrentIndex( index + 1 );
//      mUi->mRecurrenceEditButton->setEnabled( true );
//      mUi->mRecurrenceCombo->blockSignals( false );
//    }

//  } else {
//    mUi->mRecurrenceCombo->setCurrentIndex( 0 );
//  }

//  if ( incidence->isAlarmEnabled() ) {
//    if ( incidence->alarms().size() > 1 ) { // Must be custom
//      mUi->mAlarmCombo->blockSignals( true );
//      mUi->mAlarmCombo->setCurrentIndex( mUi->mAlarmCombo->count() - 1 );
//      mUi->mAlarmEditButton->setEnabled( true );
//      mUi->mAlarmCombo->blockSignals( false );
//    } else {
//      // Find out if the alarm is a custom one or one of the presets.
//      const int index = AlarmPresets::presetIndex( *incidence->alarms().first() );
//      if ( index == -1 ) {
//        mUi->mAlarmCombo->blockSignals( true );
//        mUi->mAlarmCombo->setCurrentIndex( mUi->mAlarmCombo->count() - 1 );
//        mUi->mAlarmCombo->blockSignals( false );
//      } else {
//        // Add one to cope with the "no alarm" option in the combo, which is not
//        // in the presets.
//        mUi->mAlarmCombo->blockSignals( true );
//        mUi->mAlarmCombo->setCurrentIndex( index + 1 );
//        mUi->mAlarmCombo->blockSignals( false );
//        mUi->mAlarmEditButton->setEnabled( true );
//      }

//      mLastAlarms.setAutoDelete( true );
//      mLastAlarms.clearAll();
//      for ( int i = 0; i < incidence->alarms().size(); ++i )
//        mLastAlarms.append( new Alarm( *incidence->alarms().at( i ) ) );

//    }
//  } else {
//    mUi->mAlarmCombo->setCurrentIndex( 0 );
//  }

  mWasDirty = false;
}

void IncidenceDateTimeEditor::save( KCal::Incidence::Ptr incidence )
{
  if ( KCal::Todo::Ptr todo = IncidenceDateTimeEditor::incidence<Todo>( incidence ) )
    save( todo );
  else if ( KCal::Event::Ptr event = IncidenceDateTimeEditor::incidence<Event>( incidence ) )
    save( event );
  else
    Q_ASSERT_X( false, "IncidenceDateTimeEditor::save", "Only implemented for todos and events" );

//  if ( mUi->mRecurrenceCombo->currentIndex() > 0 ) {
//    Q_ASSERT( mLastRecurrence );
//    *incidence->recurrence() = *mLastRecurrence;
//    Q_ASSERT( *mLastRecurrence == *incidence->recurrence() );
//  } else {
//    incidence->recurrence()->unsetRecurs();
//  }
}

bool IncidenceDateTimeEditor::isDirty() const
{
//  // When no alarm was set on the loaded incidence, the last alarms array should
//  // be empty, otherwise the alarms have changed.

//  // Check if a recurrence was set on a non recurring event
//  if ( !mLoadedIncidence->recurs() && mLastRecurrence )
//    return true;

//  // Check if the recurrence is removed
//  if ( mLoadedIncidence->recurs() && !mLastRecurrence )
//    return true;

//  // Check if the recurrence has changed.
//  if ( mLastRecurrence && *mLastRecurrence != *mLoadedIncidence->recurrence() )
//    return true;

  if ( KCal::Todo::ConstPtr todo = IncidenceDateTimeEditor::incidence<Todo>() ) {
    return isDirty( todo );
  } else if ( KCal::Event::ConstPtr event = IncidenceDateTimeEditor::incidence<Event>() ) {
    return isDirty( event );
  } else {
    Q_ASSERT_X( false, "IncidenceDateTimeEditor::isDirty", "Only implemented for todos and events" );
    return false;
  }
}

void IncidenceDateTimeEditor::setActiveDate( const QDate &activeDate )
{
  mActiveDate = activeDate;
}

/// private slots for General

void IncidenceDateTimeEditor::editRecurrence()
{
#if 0 //ifndef KDEPIM_MOBILE_UI
  Q_ASSERT( mLastRecurrence );
  Q_ASSERT( mUi->mRecurrenceCombo->currentIndex() ); // a preset or custom should be selected

  QScopedPointer<IncidenceRecurrenceDialog> dialog( new IncidenceRecurrenceDialog );
  dialog->load( *mLastRecurrence, currentStartDateTime().dateTime(), currentEndDateTime().dateTime() );
  if ( dialog->exec() == QDialog::Accepted ) {
    dialog->save( mLastRecurrence );

    if (  mUi->mRecurrenceCombo->currentIndex() < mUi->mRecurrenceCombo->count() - 1 ) {
      KDateTime start = currentStartDateTime();
      QScopedPointer<Recurrence>
        preset( RecurrencePresets::preset( mUi->mRecurrenceCombo->currentText(), start ) );

      if ( *preset != *mLastRecurrence ) {
        mUi->mRecurrenceCombo->blockSignals( true );
        mUi->mRecurrenceCombo->setCurrentIndex( mUi->mRecurrenceCombo->count() - 1 );
        mUi->mRecurrenceCombo->blockSignals( false );
      }
    }
  }
#endif
}

void IncidenceDateTimeEditor::setTimeZonesVisibility( bool visible )
{
  static const QString tz( i18nc( "@info:label", "Time zones" ) );

#ifndef KDEPIM_MOBILE_UI
  QString placeholder( "<a href=\"hide\"><font color='blue'>&lt;&lt; %1</font></a>" );
  if ( visible ) {
    placeholder = placeholder.arg( tz );
  } else {
    placeholder = QString( "<a href=\"show\"><font color='blue'>%1 &gt;&gt;</font></a>" );
    placeholder = placeholder.arg( tz );
  }
  mUi->mTimeZoneLabel->setText( placeholder );
#endif

  mUi->mTimeZoneComboStart->setVisible( visible );
  mUi->mTimeZoneComboEnd->setVisible( visible );
}

void IncidenceDateTimeEditor::toggleTimeZoneVisibility()
{
  setTimeZonesVisibility( !mUi->mTimeZoneComboStart->isVisible() );
}

void IncidenceDateTimeEditor::startTimeChanged( const QTime &newTime )
{
  if ( !newTime.isValid() )
    return;

  KDateTime endDateTime = currentEndDateTime();
  const int secsep = mCurrentStartDateTime.secsTo( endDateTime );
  mCurrentStartDateTime.setTime( newTime );
  if ( mUi->mEndCheck->isChecked() ) {
    // Only update the end time when it is actually enabled, adjust end time so
    // that the event/todo has the same duration as before.
    endDateTime = mCurrentStartDateTime.addSecs( secsep );
    mUi->mEndTimeEdit->setTime( endDateTime.time() );
    mUi->mEndDateEdit->setDate( endDateTime.date() );
  }

//   emit dateTimesChanged( mCurrStartDateTime, mCurrEndDateTime );
  checkDirtyStatus();
}

void IncidenceDateTimeEditor::startDateChanged( const QDate &newDate )
{
  if ( !newDate.isValid() )
    return;

  KDateTime endDateTime = currentEndDateTime();
  int daysep = mCurrentStartDateTime.daysTo( endDateTime );
  mCurrentStartDateTime.setDate( newDate );
  if ( mUi->mEndCheck->isChecked() ) {
    // Only update the end time when it is actually enabled, adjust end time so
    // that the event/todo has the same duration as before.
    endDateTime.setDate( mCurrentStartDateTime.date().addDays( daysep ) );
    mUi->mEndDateEdit->setDate( endDateTime.date() );
//     emit dateTimesChanged( mCurrStartDateTime, mCurrEndDateTime );
  }

  checkDirtyStatus();
}

void IncidenceDateTimeEditor::startSpecChanged()
{
  if ( mUi->mEndCheck->isChecked()
    && currentEndDateTime().timeSpec() == mCurrentStartDateTime.timeSpec() )
    mUi->mTimeZoneComboEnd->selectTimeSpec( mUi->mTimeZoneComboStart->selectedTimeSpec() );

  mCurrentStartDateTime.setTimeSpec( mUi->mTimeZoneComboStart->selectedTimeSpec() );
//   emit dateTimesChanged( mCurrStartDateTime, mCurrEndDateTime );
}

void IncidenceDateTimeEditor::updateRecurrencePreset( int index )
{
//  mUi->mRecurrenceEditButton->setEnabled( mUi->mRecurrenceCombo->currentIndex() > 0 );

//  if ( index == 0 ) { // No recurrence
//    delete mLastRecurrence;
//    mLastRecurrence = 0;
//    checkDirtyStatus();
//    return;
//  }

//  QScopedPointer<Recurrence> rec;
//  KDateTime start = currentStartDateTime();
//  start.setDateOnly( mUi->mWholeDayCheck->isChecked() && mUi->mStartCheck->isChecked() );

//  if ( index == (mUi->mRecurrenceCombo->count() - 1) ) {
//    // Configure a custom recurrence, use by default the Weekly recurrence preset
//    rec.reset( RecurrencePresets::preset( i18nc( "@item:inlistbox", "Weekly" ), start ) );
//    QScopedPointer<IncidenceRecurrenceDialog> dialog( new IncidenceRecurrenceDialog );
//    dialog->load( *rec, currentStartDateTime().dateTime(), currentEndDateTime().dateTime() );
//    if ( dialog->exec() == QDialog::Accepted )
//      dialog->save( mLastRecurrence );
//    else {
//      delete mLastRecurrence;
//      mLastRecurrence = 0;
//      mUi->mRecurrenceCombo->setCurrentIndex( 0 );
//    }
//  } else {
//    // Load a preset
//    delete mLastRecurrence;
//    mLastRecurrence = RecurrencePresets::preset( mUi->mRecurrenceCombo->currentText(), start );
//  }

  checkDirtyStatus();
}

void IncidenceDateTimeEditor::updateRecurrenceSummary( KCal::Incidence::ConstPtr incidence )
{
#ifdef KDEPIM_MOBILE_UI
  Q_UNUSED( incidence );
#else
//  if ( incidence->recurs() ) {
//    mUi->mRecurrenceLabel->setText( IncidenceFormatter::recurrenceString( const_cast<Incidence *>( incidence.get() ) ) );
//  } else {
//    mUi->mRecurrenceLabel->setText( QString() );
//  }
#endif
}

/// private slots for Todo

void IncidenceDateTimeEditor::enableStartEdit( bool enable )
{
  mUi->mStartDateEdit->setEnabled( enable );

  if( mUi->mEndCheck->isChecked() || mUi->mStartCheck->isChecked() ) {
    mUi->mWholeDayCheck->setEnabled( true );
  } else {
    mUi->mWholeDayCheck->setEnabled( false );
    mUi->mWholeDayCheck->setChecked( true );
  }

  if ( enable ) {
    mUi->mStartTimeEdit->setEnabled( !mUi->mWholeDayCheck->isChecked() );
    mUi->mTimeZoneComboStart->setEnabled( !mUi->mWholeDayCheck->isChecked() );
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
    mUi->mWholeDayCheck->setEnabled( true );
  } else {
    mUi->mWholeDayCheck->setEnabled( false );
  }

  if ( enable ) {
    mUi->mEndTimeEdit->setEnabled( !mUi->mWholeDayCheck->isChecked() );
    mUi->mTimeZoneComboEnd->setEnabled( !mUi->mWholeDayCheck->isChecked() );
  } else {
    mUi->mEndTimeEdit->setEnabled( false );
    mUi->mTimeZoneComboEnd->setEnabled( false );
  }

  mUi->mTimeZoneComboEnd->setFloating( !mUi->mTimeZoneComboEnd->isEnabled() );
  checkDirtyStatus();
}

void IncidenceDateTimeEditor::enableTimeEdits()
{
  // NOTE: assumes that the initial times are initialized.
  const bool wholeDayChecked = mUi->mWholeDayCheck->isChecked();
#ifndef KDEPIM_MOBILE_UI
  mUi->mTimeZoneLabel->setVisible( !wholeDayChecked );
#endif

  if( mUi->mStartCheck->isChecked() ) {
    mUi->mStartTimeEdit->setEnabled( !wholeDayChecked );
    mUi->mTimeZoneComboStart->setEnabled( !wholeDayChecked );
    mUi->mTimeZoneComboStart->setFloating( wholeDayChecked, mInitialStartDT.timeSpec() );
  }
  if( mUi->mEndCheck->isChecked() ) {
    mUi->mEndTimeEdit->setEnabled( !wholeDayChecked );
    mUi->mTimeZoneComboEnd->setEnabled( !wholeDayChecked );
    mUi->mTimeZoneComboEnd->setFloating( wholeDayChecked, mInitialEndDT.timeSpec() );
  }

#ifndef KDEPIM_MOBILE_UI
  setTimeZonesVisibility( !wholeDayChecked && mUi->mTimeZoneLabel->text().startsWith( "<<" ) );
#endif
}

bool IncidenceDateTimeEditor::isDirty( KCal::Todo::ConstPtr todo ) const
{
  Q_ASSERT( todo );

  // First check the start time/date of the todo
  if ( todo->hasStartDate() != mUi->mStartCheck->isChecked() )
    return true;
  
  if ( mUi->mStartCheck->isChecked() ) {
    // Use mActiveStartTime. This is the KDateTime::Spec selected on load comming from
    // the combobox. We use this one as it can slightly differ (e.g. missing
    // country code in the incidence time spec) from the incidence.
    if ( currentStartDateTime() != mInitialStartDT )
      return true;
  }

  if ( todo->hasDueDate() != mUi->mEndCheck->isChecked() )
    return true;

  if ( mUi->mEndCheck->isChecked() ) {
    if ( currentEndDateTime() != mInitialEndDT )
      return true;
  }

  return false;
}

/// Event specific methods

bool IncidenceDateTimeEditor::isDirty( KCal::Event::ConstPtr event ) const
{
  // When the check box is checked, it has time associated and thus is not an all
  // day event. So the editor is dirty when the event is allDay and the checkbox
  // is checked.
  if ( event->allDay() != mUi->mWholeDayCheck->isChecked() )
    return true;

  if ( !event->allDay() ) {
    if ( currentStartDateTime() != mInitialStartDT )
      return true;

    if ( currentEndDateTime() != mInitialEndDT )
      return true;
  }
  
  return false;
}

/// Private methods

KDateTime IncidenceDateTimeEditor::currentStartDateTime() const
{
  return KDateTime(
    mUi->mStartDateEdit->date(),
    mUi->mStartTimeEdit->time(),
    mUi->mTimeZoneComboStart->selectedTimeSpec() );
}

KDateTime IncidenceDateTimeEditor::currentEndDateTime() const
{
  return KDateTime(
    mUi->mEndDateEdit->date(),
    mUi->mEndTimeEdit->time(),
    mUi->mTimeZoneComboEnd->selectedTimeSpec() );
}

void IncidenceDateTimeEditor::load( KCal::Event::ConstPtr event )
{
  // First en/disable the necessary ui bits and pieces
  mUi->mStartLabel->setVisible( true );
  mUi->mEndLabel->setVisible( true );
  mUi->mStartCheck->setVisible( false );
  mUi->mStartCheck->setChecked( true ); // Set to checked so we can reuse enableTimeEdits.
  mUi->mEndCheck->setVisible( false );
  mUi->mEndCheck->setChecked( true ); // Set to checked so we can reuse enableTimeEdits.

  // All day
  connect( mUi->mWholeDayCheck, SIGNAL(toggled(bool)),
           SLOT(enableTimeEdits()) );
  // Start time
  connect( mUi->mStartTimeEdit, SIGNAL(timeChanged(QTime)),
           SLOT(startTimeChanged(QTime)) );
  connect( mUi->mStartDateEdit, SIGNAL(dateChanged(QDate)),
           SLOT(startDateChanged(QDate)) );
  connect( mUi->mTimeZoneComboStart, SIGNAL(currentIndexChanged(int)),
           SLOT(startSpecChanged()) );
  // End time
  connect( mUi->mEndTimeEdit, SIGNAL(timeChanged(QTime)),
           SLOT(checkDirtyStatus()) );
  connect( mUi->mEndDateEdit, SIGNAL(dateChanged(QDate)),
           SLOT(checkDirtyStatus()) );
  connect( mUi->mTimeZoneComboEnd, SIGNAL(currentIndexChanged(int)),
           SLOT(checkDirtyStatus()) );

  mUi->mWholeDayCheck->setChecked( event->allDay() );
  enableTimeEdits();

  bool isTemplate = false; // TODO
  if ( !isTemplate ) {
    KDateTime startDT = event->dtStart();
    KDateTime endDT = event->dtEnd();
    if ( event->recurs() && mActiveDate.isValid() ) {
      // Consider the active date when editing recurring Events.
      KDateTime kdt( mActiveDate, QTime( 0, 0, 0 ), KSystemTimeZones::local() );
      const int eventLength = startDT.daysTo( endDT );
      kdt = kdt.addSecs( -1 );
      startDT.setDate( event->recurrence()->getNextDateTime( kdt ).date() );
      if ( event->hasEndDate() ) {
        endDT.setDate( startDT.addDays( eventLength ).date() );
      } else {
        if ( event->hasDuration() ) {
          endDT = startDT.addSecs( event->duration().asSeconds() );
        } else {
          endDT = startDT;
        }
      }
    }
    // Convert UTC to local timezone, if needed (i.e. for kolab #204059)
    if ( startDT.isUtc() )
      startDT = startDT.toLocalZone();

    if ( endDT.isUtc() )
      endDT = endDT.toLocalZone();

    setDateTimes( startDT, endDT );
  } else {
    // set the start/end time from the template, only as a last resort #190545
    if ( !event->dtStart().isValid() || !event->dtEnd().isValid() ) {
      setTimes( event->dtStart(), event->dtEnd() );
    }
  }

  switch( event->transparency() ) {
  case Event::Transparent:
#if 0 //def KDEPIM_MOBILE_UI
    mUi->mFreeRadio->setChecked( true );
#else
    mUi->mFreeBusyCombo->setCurrentIndex( 1 );
#endif
    break;
  case Event::Opaque:
#if 0 //def KDEPIM_MOBILE_UI
    mUi->mBusyRadio->setChecked( true );
#else
    mUi->mFreeBusyCombo->setCurrentIndex( 0 );
#endif
    break;
  }

  updateRecurrenceSummary( event );
}

void IncidenceDateTimeEditor::load( KCal::Todo::ConstPtr todo )
{
  // First en/disable the necessary ui bits and pieces
  mUi->mStartLabel->setVisible( false );
  mUi->mEndLabel->setVisible( false );

  mUi->mStartCheck->setVisible( true );
  mUi->mStartCheck->setChecked( todo->hasStartDate() );
  mUi->mStartDateEdit->setEnabled( todo->hasStartDate() );
  mUi->mStartTimeEdit->setEnabled( todo->hasStartDate() );
  mUi->mTimeZoneComboStart->setEnabled( todo->hasStartDate() );
  
  mUi->mEndCheck->setVisible( true );
  mUi->mEndCheck->setChecked( todo->hasDueDate() );
  mUi->mEndDateEdit->setEnabled( todo->hasDueDate() );
  mUi->mEndTimeEdit->setEnabled( todo->hasDueDate() );
  mUi->mTimeZoneComboEnd->setEnabled( todo->hasDueDate() );

  // These fields where not enabled in the old code either:
#if 0 //def KDEPIM_MOBILE_UI
  mUi->mFreeRadio->setVisible( false );
  mUi->mBusyRadio->setVisible( false );
#else
  mUi->mFreeBusyCombo->setVisible( false );
#endif
  mUi->mFreeBusyLabel->setVisible( false );

  mUi->mWholeDayCheck->setChecked( todo->allDay() );

  // Connect to the right logic
  connect( mUi->mStartCheck, SIGNAL(toggled(bool)), SLOT(enableStartEdit(bool)) );
  connect( mUi->mStartDateEdit, SIGNAL(dateChanged(QDate)), SLOT(checkDirtyStatus()) );
  connect( mUi->mStartTimeEdit, SIGNAL(timeChanged(QTime)), SLOT(startTimeChanged(QTime)) );
  connect( mUi->mTimeZoneComboStart, SIGNAL(currentIndexChanged(int)), SLOT(checkDirtyStatus()) );

  connect( mUi->mEndCheck, SIGNAL(toggled(bool)), SLOT(enableEndEdit(bool)) );
  //   connect( mDueCheck, SIGNAL(toggled(bool)), SIGNAL(dueDateEditToggle(bool)) );
  connect( mUi->mEndDateEdit, SIGNAL(dateChanged(QDate)), SLOT(checkDirtyStatus()) );
  connect( mUi->mEndTimeEdit, SIGNAL(timeChanged(const QTime&)), SLOT(checkDirtyStatus()) );
  connect( mUi->mTimeZoneComboEnd, SIGNAL(currentIndexChanged(int)), SLOT(checkDirtyStatus()) );

  connect( mUi->mWholeDayCheck, SIGNAL(toggled(bool)), SLOT(enableTimeEdits()));

  //TODO: do something with tmpl, note: this wasn't used in the old code either.
//   Q_UNUSED( tmpl );

  KDateTime endDT = KDateTime( QDate::currentDate(), QTime::currentTime() ).toLocalZone();
  if ( todo->hasDueDate() ) {
    endDT = todo->dtDue();
    if ( todo->recurs() && mActiveDate.isValid() ) {
      KDateTime dt( mActiveDate, QTime( 0, 0, 0 ) );
      dt = dt.addSecs( -1 );
      endDT.setDate( todo->recurrence()->getNextDateTime( dt ).date() );
    }
    if ( endDT.isUtc() )
      endDT = endDT.toLocalZone();
  }

  KDateTime startDT = KDateTime( QDate::currentDate(), QTime::currentTime() ).toLocalZone();
  if ( todo->hasStartDate() ) {
    startDT = todo->dtStart();
    if ( todo->recurs() && mActiveDate.isValid() && todo->hasDueDate() ) {
      int days = todo->dtStart( true ).daysTo( todo->dtDue( true ) );
      startDT.setDate( startDT.date().addDays( -days ) );
    }
    if ( startDT.isUtc() )
      startDT = startDT.toLocalZone();
  }

  setDateTimes( startDT, endDT );
  updateRecurrenceSummary( todo );
}

void IncidenceDateTimeEditor::save( KCal::Event::Ptr event )
{
  if ( mUi->mWholeDayCheck->isChecked() ) { // Timed event
    event->setAllDay( true );

    // need to change this.
    KDateTime eventDT = currentStartDateTime();
    eventDT.setDateOnly( true );
    event->setDtStart( eventDT );
    event->setDtEnd( eventDT );
  } else { // All day event
    event->setAllDay( false );

    // set date/time end
    event->setDtStart( currentStartDateTime() );
    event->setDtEnd( currentEndDateTime() );
  }

  // Free == Event::Transparant
  // Busy == Event::Opaque
#if 0 //def KDEPIM_MOBILE_UI
  event->setTransparency( mUi->mFreeRadio->isChecked() ?
                          KCal::Event::Transparent : KCal::Event::Opaque );
#else
  event->setTransparency( mUi->mFreeBusyCombo->currentIndex() > 0 ?
                          KCal::Event::Transparent : KCal::Event::Opaque );
#endif
}

void IncidenceDateTimeEditor::save( KCal::Todo::Ptr todo )
{
  Q_ASSERT_X( false, "IncidenceDateTimeEditor::save", "not implemented" );
}

void IncidenceDateTimeEditor::setDateTimes( const KDateTime &start, const KDateTime &end )
{
  if ( start.isValid() ) {
    mUi->mStartDateEdit->setDate( start.date() );
    mUi->mStartTimeEdit->setTime( start.time() );
    mUi->mTimeZoneComboStart->selectTimeSpec( start.timeSpec() );
  } else {
    KDateTime dt( QDate::currentDate(), QTime::currentTime() );
    mUi->mStartDateEdit->setDate( dt.date() );
    mUi->mStartTimeEdit->setTime( dt.time() );
    mUi->mTimeZoneComboStart->selectTimeSpec( dt.timeSpec() );
  }

  if ( end.isValid() ) {
    mUi->mEndDateEdit->setDate( end.date() );
    mUi->mEndTimeEdit->setTime( end.time() );
    mUi->mTimeZoneComboEnd->selectTimeSpec( end.timeSpec() );
  } else {
    KDateTime dt( QDate::currentDate(), QTime::currentTime().addSecs( 60 * 60 ) );
    mUi->mEndDateEdit->setDate( dt.date() );
    mUi->mEndTimeEdit->setTime( dt.time() );
    mUi->mTimeZoneComboEnd->selectTimeSpec( dt.timeSpec() );
  }

  mCurrentStartDateTime = currentStartDateTime();
}

void IncidenceDateTimeEditor::setTimes( const KDateTime &start, const KDateTime &end )
{
  // like setDateTimes(), but it set only the start/end time, not the date
  // it is used while applying a template to an event.
  mUi->mStartTimeEdit->blockSignals( true );
  mUi->mStartTimeEdit->setTime( start.time() );
  mUi->mStartTimeEdit->blockSignals( false );

  mUi->mEndTimeEdit->setTime( end.time() );

  mUi->mTimeZoneComboStart->selectTimeSpec( start.timeSpec() );
  mUi->mTimeZoneComboEnd->selectTimeSpec( end.timeSpec() );

//   emitDateTimeStr();
}

#include "moc_incidencedatetimeeditor.cpp"
