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

#ifdef KDEPIM_MOBILE_UI
#include "ui_iedatetimemobile.h"
#else
#include "ui_incidencedatetime.h"
#endif

using namespace IncidenceEditorsNG;
using namespace KCal;

IncidenceDateTimeEditor::IncidenceDateTimeEditor( QWidget *parent )
  : IncidenceEditor( parent )
  , mTimeZones( new ICalTimeZones )
  , mUi( new Ui::IncidenceDateTimeEditor )
{
  mUi->setupUi( this );
  showOrHideTimeZones( "hide" );

#ifdef KDEPIM_MOBILE_UI
  QButtonGroup *freeBusyGroup = new QButtonGroup( this );
  freeBusyGroup->addButton( mUi->mFreeRadio );
  freeBusyGroup->addButton( mUi->mBusyRadio );

  mUi->mTimeZoneComboStart->setVisible( false );
  mUi->mTimeZoneComboEnd->setVisible( false );
#else
  mUi->mAlarmBell->setPixmap( SmallIcon( "task-reminder" ) );
  mUi->mRecurrenceEditButton->setIcon(
    KIconLoader::global()->loadIcon(
      "task-recurring", KIconLoader::Desktop, KIconLoader::SizeSmall ) );
  connect( mUi->mTimeZoneLabel, SIGNAL(linkActivated(QString)),
           SLOT(showOrHideTimeZones(QString)) );
  connect( mUi->mRecurrenceEditButton, SIGNAL(clicked()), SLOT(editRecurrence()) );
#endif

  connect( mUi->mHasTimeCheck, SIGNAL(toggled(bool)), SLOT(setDuration()) );
  connect( mUi->mHasTimeCheck, SIGNAL(toggled(bool)), SLOT(checkDirtyStatus()) );
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
  } else if ( KCal::Event::ConstPtr event = IncidenceDateTimeEditor::incidence<Event>() ) {
    load( event );
  } else {
    kDebug() << "Not an event or an todo.";
  }

  enableTimeEdits( mUi->mHasTimeCheck->isChecked() );

  mInitialStartDT = currentStartDateTime();
  mInitialEndDT = currentEndDateTime();

  if ( mUi->mTimeZoneComboStart->currentIndex() == 0 ) // Floating
    mInitialStartDT.setTimeSpec( mInitialStartDT.toLocalZone().timeSpec() );

  if ( mUi->mTimeZoneComboEnd->currentIndex() == 0 ) // Floating
    mInitialEndDT.setTimeSpec( mInitialEndDT.toLocalZone().timeSpec() );

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
}

bool IncidenceDateTimeEditor::isDirty() const
{
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
  QPointer<IncidenceRecurrenceDialog> dialog( new IncidenceRecurrenceDialog( this ) );
  dialog->exec();
  delete dialog;
}

void IncidenceDateTimeEditor::enableAlarm( bool enable )
{
  mUi->mAlarmStack->setEnabled( enable );
  mUi->mAlarmEditButton->setEnabled( enable );
}

void IncidenceDateTimeEditor::showOrHideTimeZones( const QString &showOrHide )
{
  if ( showOrHide == QLatin1String( "hide" ) ) {
#ifndef KDEPIM_MOBILE_UI
    mUi->mTimeZoneLabel->setText( "<a href=\"show\"><font color='blue'>Time zones &gt;&gt;</font></a>" );
#endif
    mUi->mTimeZoneComboStart->setVisible( false );
    mUi->mTimeZoneComboEnd->setVisible( false );
  } else {
#ifndef KDEPIM_MOBILE_UI
    mUi->mTimeZoneLabel->setText( "<a href=\"hide\"><font color='blue'>&lt;&lt;Time zones</font></a>" );
#endif
    mUi->mTimeZoneComboStart->setVisible( true );
    mUi->mTimeZoneComboEnd->setVisible( true );
  }
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

void IncidenceDateTimeEditor::updateRecurrenceSummary( KCal::Incidence::ConstPtr incidence )
{
#ifdef KDEPIM_MOBILE_UI
  Q_UNUSED( incidence );
#else
  if ( incidence->recurs() ) {
    mUi->mRecurrenceLabel->setText( IncidenceFormatter::recurrenceString( const_cast<Incidence *>( incidence.get() ) ) );
  } else {
    mUi->mRecurrenceLabel->setText( QString() );
  }
#endif
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
    mUi->mTimeZoneComboStart->setFloating( !enable, mInitialStartDT.timeSpec() );
  }
  if( mUi->mEndCheck->isChecked() ) {
    mUi->mEndTimeEdit->setEnabled( enable );
    mUi->mTimeZoneComboEnd->setEnabled( enable );
    mUi->mTimeZoneComboEnd->setFloating( !enable, mInitialEndDT.timeSpec() );
  }
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

  // TODO Recurrence
  // TODO Alarms

  return false;
}

/// Event specific methods

bool IncidenceDateTimeEditor::isDirty( KCal::Event::ConstPtr event ) const
{
  // When the check box is checked, it has time associated and thus is not an all
  // day event. So the editor is dirty when the event is allDay and the checkbox
  // is checked.
  if ( event->allDay() == mUi->mHasTimeCheck->isChecked() )
    return true;

  if ( !event->allDay() ) {
    if ( currentStartDateTime() != mInitialStartDT )
      return true;

    if ( currentEndDateTime() != mInitialEndDT )
      return true;
  }

  // TODO Recurrence
  // TODO Alarms
  
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
  connect( mUi->mHasTimeCheck, SIGNAL(toggled(bool)),
           SLOT(enableTimeEdits(bool)) );
  connect( mUi->mHasTimeCheck, SIGNAL(toggled(bool)),
           SLOT(enableAlarm(bool)) );
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

  mUi->mHasTimeCheck->setChecked( !event->allDay() );
  enableTimeEdits( !event->allDay() );

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
    if ( startDT.isUtc() ) {
      startDT = startDT.toLocalZone();
    }
    if ( endDT.isUtc() ) {
      endDT = endDT.toLocalZone();
    }
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
    mUi->mFreeBusyCombo->setCurrentIndex( 1 );
#endif
    break;
  case Event::Opaque:
#ifdef KDEPIM_MOBILE_UI
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
#ifdef KDEPIM_MOBILE_UI
  mUi->mFreeRadio->setVisible( false );
  mUi->mBusyRadio->setVisible( false );
#else
  mUi->mDurationLabel->setVisible( false );
  mUi->mFreeBusyCombo->setVisible( false );
#endif
  mUi->mFreeBusyLabel->setVisible( false );

  mUi->mHasTimeCheck->setChecked( !todo->allDay() );

  // Connect to the right logic
  connect( mUi->mStartCheck, SIGNAL(toggled(bool)), SLOT(enableStartEdit(bool)) );
  connect( mUi->mStartDateEdit, SIGNAL(dateChanged(QDate)), SLOT(checkDirtyStatus()) );
  connect( mUi->mStartTimeEdit, SIGNAL(timeChanged(QTime)), SLOT(startTimeChanged(QTime)) );
  connect( mUi->mTimeZoneComboStart, SIGNAL(currentIndexChanged(int)), SLOT(checkDirtyStatus()) );

  connect( mUi->mEndCheck, SIGNAL(toggled(bool)), SLOT(enableEndEdit(bool)) );
  connect( mUi->mEndCheck, SIGNAL(toggled(bool)), SLOT(enableAlarm(bool)) );
  //   connect( mDueCheck, SIGNAL(toggled(bool)), SIGNAL(dueDateEditToggle(bool)) );
  connect( mUi->mEndDateEdit, SIGNAL(dateChanged(QDate)), SLOT(checkDirtyStatus()) );
  connect( mUi->mEndTimeEdit, SIGNAL(timeChanged(const QTime&)), SLOT(checkDirtyStatus()) );
  connect( mUi->mTimeZoneComboEnd, SIGNAL(currentIndexChanged(int)), SLOT(checkDirtyStatus()) );

  connect( mUi->mHasTimeCheck, SIGNAL(toggled(bool)), SLOT(enableTimeEdits(bool)));

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
  enableAlarm( todo->hasDueDate() );
  updateRecurrenceSummary( todo );
}

void IncidenceDateTimeEditor::save( KCal::Event::Ptr event )
{
  if ( mUi->mHasTimeCheck->isChecked() ) { // Timed event
    event->setAllDay( false );

    // set date/time end
    event->setDtStart( currentStartDateTime() );
    event->setDtEnd( currentEndDateTime() );
  } else { // All day event
    event->setAllDay( true );

    // need to change this.
    KDateTime eventDT = currentStartDateTime();
    eventDT.setDateOnly( true );
    event->setDtStart( eventDT );
    event->setDtEnd( eventDT );
  }

  // Free == Event::Transparant
  // Busy == Event::Opaque
#ifdef KDEPIM_MOBILE_UI
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
    KDateTime dt( QDate::currentDate(), QTime::currentTime() );
    mUi->mEndDateEdit->setDate( dt.date() );
    mUi->mEndTimeEdit->setTime( dt.time() );
    mUi->mTimeZoneComboEnd->selectTimeSpec( dt.timeSpec() );
  }

  mCurrentStartDateTime = currentStartDateTime();
  setDuration();
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

  setDuration();
//   emitDateTimeStr();
}

void IncidenceDateTimeEditor::setDuration()
{
#ifndef KDEPIM_MOBILE_UI
  // Those checks are always checked for events, but not for todos. If one of them
  // isn't checked we don't show the duration.
  if ( !mUi->mStartCheck->isChecked() || !mUi->mEndCheck->isChecked() ) {
    mUi->mDurationLabel->setVisible( false );
    return;
  }

  mUi->mDurationLabel->setVisible( true );
  
  QString tmpStr, catStr;
  int hourdiff, minutediff;
  // end date is an accepted temporary state while typing, but don't show
  // any duration if this happens
  KDateTime startDateTime = currentStartDateTime();
  KDateTime endDateTime = currentEndDateTime();
    
  if ( startDateTime < endDateTime ) {

    if ( !mUi->mHasTimeCheck->isChecked() ) {
      int daydiff = startDateTime.date().daysTo( endDateTime.date() ) + 1;
      tmpStr = i18nc( "@label", "Duration: " );
      tmpStr.append( i18ncp( "@label", "1 Day", "%1 Days", daydiff ) );
    } else {
      hourdiff = startDateTime.date().daysTo( endDateTime.date() ) * 24;
      hourdiff += endDateTime.time().hour() - startDateTime.time().hour();
      minutediff = endDateTime.time().minute() - startDateTime.time().minute();
      // If minutediff is negative, "borrow" 60 minutes from hourdiff
      if ( minutediff < 0 && hourdiff > 0 ) {
        hourdiff -= 1;
        minutediff += 60;
      }
      if ( hourdiff || minutediff ) {
        tmpStr = i18nc( "@label", "Duration: " );
        if ( hourdiff ){
          catStr = i18ncp( "@label", "1 hour", "%1 hours", hourdiff );
          tmpStr.append( catStr );
        }
        if ( hourdiff && minutediff ) {
          tmpStr += i18nc( "@label", ", " );
        }
        if ( minutediff ){
          catStr = i18ncp( "@label", "1 minute", "%1 minutes", minutediff );
          tmpStr += catStr;
        }
      } else {
        tmpStr = "";
      }
    }
  }
  mUi->mDurationLabel->setText( tmpStr );
  mUi->mDurationLabel->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Shows the duration of the event or to-do with the "
           "current start and end dates and times." ) );
#endif
}
