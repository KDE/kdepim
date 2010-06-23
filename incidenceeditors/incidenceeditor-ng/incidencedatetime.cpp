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

#include "incidencedatetime.h"

#include <KCal/ICalTimeZones>
#include <KCal/IncidenceFormatter>
#include <KDebug>
#include <KSystemTimeZones>

//#ifdef KDEPIM_MOBILE_UI
//#include "ui_iedatetimemobile.h"
//#else
//#include "../editoralarms.h"
//#include "ui_incidencedatetime.h"
//#endif

// TODO different UI file in case of mobile
#ifdef KDEPIM_MOBILE_UI
#include "ui_eventortodomobile.h"
#else
#include "ui_eventortododesktop.h"
#endif
#include "../editoralarms.h"

using namespace IncidenceEditorsNG;
using namespace KCal;

IncidenceDateTime::IncidenceDateTime( Ui::EventOrTodoDesktop *ui )
  : IncidenceEditor( 0 )
  , mTimeZones( new ICalTimeZones )
  , mUi( ui )
{
  mUi->setupUi( this );
  
  setTimeZonesVisibility( false );

#ifdef KDEPIM_MOBILE_UI
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

  connect( mUi->mWholeDayCheck, SIGNAL(toggled(bool)),
           SLOT(checkDirtyStatus()) );
}

IncidenceDateTime::~IncidenceDateTime()
{
  delete mTimeZones;
}

void IncidenceDateTime::load( KCal::Incidence::ConstPtr incidence )
{
  mLoadedIncidence = incidence;

  // We can only handle events or todos.
  if ( KCal::Todo::ConstPtr todo = IncidenceDateTime::incidence<Todo>() ) {
    load( todo );
  } else if ( KCal::Event::ConstPtr event = IncidenceDateTime::incidence<Event>() ) {
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

  mWasDirty = false;
}

void IncidenceDateTime::save( KCal::Incidence::Ptr incidence )
{
  if ( KCal::Todo::Ptr todo = IncidenceDateTime::incidence<Todo>( incidence ) )
    save( todo );
  else if ( KCal::Event::Ptr event = IncidenceDateTime::incidence<Event>( incidence ) )
    save( event );
  else
    Q_ASSERT_X( false, "IncidenceDateTimeEditor::save", "Only implemented for todos and events" );
}

bool IncidenceDateTime::isDirty() const
{
  if ( KCal::Todo::ConstPtr todo = IncidenceDateTime::incidence<Todo>() ) {
    return isDirty( todo );
  } else if ( KCal::Event::ConstPtr event = IncidenceDateTime::incidence<Event>() ) {
    return isDirty( event );
  } else {
    Q_ASSERT_X( false, "IncidenceDateTimeEditor::isDirty", "Only implemented for todos and events" );
    return false;
  }
}

void IncidenceDateTime::setActiveDate( const QDate &activeDate )
{
  mActiveDate = activeDate;
}

QDate IncidenceDateTime::startDate() const
{
  return currentStartDateTime().date();
}

/// private slots for General

void IncidenceDateTime::setTimeZonesVisibility( bool visible )
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

void IncidenceDateTime::toggleTimeZoneVisibility()
{
  setTimeZonesVisibility( !mUi->mTimeZoneComboStart->isVisible() );
}

void IncidenceDateTime::updateStartTime( const QTime &newTime )
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

void IncidenceDateTime::updateStartDate( const QDate &newDate )
{
  if ( !newDate.isValid() )
    return;

  const bool dateChanged = mCurrentStartDateTime.date().day() != newDate.day()
                           || mCurrentStartDateTime.date().month() != newDate.month();

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

  if ( dateChanged )
    emit startDateChanged( mCurrentStartDateTime.date() );
}

void IncidenceDateTime::updateStartSpec()
{
  QDate prevDate = mCurrentStartDateTime.date();

  if ( mUi->mEndCheck->isChecked()
    && currentEndDateTime().timeSpec() == mCurrentStartDateTime.timeSpec() )
    mUi->mTimeZoneComboEnd->selectTimeSpec( mUi->mTimeZoneComboStart->selectedTimeSpec() );

  mCurrentStartDateTime.setTimeSpec( mUi->mTimeZoneComboStart->selectedTimeSpec() );

  const bool dateChanged = mCurrentStartDateTime.date().day() != prevDate.day()
                           || mCurrentStartDateTime.date().month() != prevDate.month();
  if ( dateChanged )
    emit startDateChanged( mCurrentStartDateTime.date() );
}

/// private slots for Todo

void IncidenceDateTime::enableStartEdit( bool enable )
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

void IncidenceDateTime::enableEndEdit( bool enable )
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

void IncidenceDateTime::enableTimeEdits()
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

bool IncidenceDateTime::isDirty( KCal::Todo::ConstPtr todo ) const
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

bool IncidenceDateTime::isDirty( KCal::Event::ConstPtr event ) const
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

KDateTime IncidenceDateTime::currentStartDateTime() const
{
  return KDateTime(
    mUi->mStartDateEdit->date(),
    mUi->mStartTimeEdit->time(),
    mUi->mTimeZoneComboStart->selectedTimeSpec() );
}

KDateTime IncidenceDateTime::currentEndDateTime() const
{
  return KDateTime(
    mUi->mEndDateEdit->date(),
    mUi->mEndTimeEdit->time(),
    mUi->mTimeZoneComboEnd->selectedTimeSpec() );
}

void IncidenceDateTime::load( KCal::Event::ConstPtr event )
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
           SLOT(updateStartTime(QTime)) );
  connect( mUi->mStartDateEdit, SIGNAL(dateChanged(QDate)),
           SLOT(updateStartDate(QDate)) );
  connect( mUi->mTimeZoneComboStart, SIGNAL(currentIndexChanged(int)),
           SLOT(updateStartSpec()) );
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
#ifdef KDEPIM_MOBILE_UI
    mUi->mFreeRadio->setChecked( true );
#else
    mUi->mFreeBusyCheck->setChecked( false );
#endif
    break;
  case Event::Opaque:
#ifdef KDEPIM_MOBILE_UI
    mUi->mBusyRadio->setChecked( true );
#else
    mUi->mFreeBusyCheck->setChecked( true );
#endif
    break;
  }
}

void IncidenceDateTime::load( KCal::Todo::ConstPtr todo )
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
#ifdef KDEPIM_MOBILE_UI
  mUi->mFreeRadio->setVisible( false );
  mUi->mBusyRadio->setVisible( false );
#else
  mUi->mFreeBusyCheck->setVisible( false );
#endif
//  mUi->mFreeBusyLabel->setVisible( false );

  mUi->mWholeDayCheck->setChecked( todo->allDay() );

  // Connect to the right logic
  connect( mUi->mStartCheck, SIGNAL(toggled(bool)), SLOT(enableStartEdit(bool)) );
  connect( mUi->mStartDateEdit, SIGNAL(dateChanged(QDate)), SLOT(checkDirtyStatus()) );
  connect( mUi->mStartTimeEdit, SIGNAL(timeChanged(QTime)), SLOT(updateStartTime(QTime)) );
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
}

void IncidenceDateTime::save( KCal::Event::Ptr event )
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
#ifdef KDEPIM_MOBILE_UI
  event->setTransparency( mUi->mFreeRadio->isChecked() ?
                          KCal::Event::Transparent : KCal::Event::Opaque );
#else
  event->setTransparency( mUi->mFreeBusyCheck->isChecked() ? Event::Opaque : Event::Transparent );
#endif
}

void IncidenceDateTime::save( KCal::Todo::Ptr todo )
{
  Q_ASSERT_X( false, "IncidenceDateTimeEditor::save", "not implemented" );
}

void IncidenceDateTime::setDateTimes( const KDateTime &start, const KDateTime &end )
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

void IncidenceDateTime::setTimes( const KDateTime &start, const KDateTime &end )
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

#include "moc_incidencedatetime.cpp"
