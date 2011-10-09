/*
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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
#ifdef KDEPIM_MOBILE_UI
#include "ui_eventortodomobile.h"
#else
#include "ui_eventortododesktop.h"
#endif
//#ifdef KDEPIM_MOBILE_UI
//#include "ui_iedatetimemobile.h"
//#else
//#include "../editoralarms.h"
//#include "ui_incidencedatetime.h"
//#endif

#include <KCalCore/ICalTimeZones>

#include <KDebug>
#include <KSystemTimeZone>

using namespace IncidenceEditorNG;

IncidenceDateTime::IncidenceDateTime( Ui::EventOrTodoDesktop *ui )
  : IncidenceEditor( 0 ), mTimeZones( new KCalCore::ICalTimeZones ), mUi( ui ),
    mTimezoneCombosWhereVisibile( false )
{
  setTimeZonesVisibility( false );
  setObjectName( "IncidenceDateTime" );

#ifdef KDEPIM_MOBILE_UI
  mUi->mTimeZoneComboStart->setVisible( false );
  mUi->mTimeZoneComboEnd->setVisible( false );

  // We don't want to see the combobox list / calendar in the mobile version
  mUi->mStartDateEdit->setOptions( mUi->mStartDateEdit->options() & ~KDateComboBox::EditDate );
  mUi->mEndDateEdit->setOptions( mUi->mEndDateEdit->options() & ~KDateComboBox::EditDate );
  mUi->mStartTimeEdit->clear();
  mUi->mEndTimeEdit->clear();

  // This event filter is not needed in the desktop version
  mUi->mStartDateEdit->installEventFilter( this );
  mUi->mEndDateEdit->installEventFilter( this );
  mUi->mStartTimeEdit->installEventFilter( this );
  mUi->mEndTimeEdit->installEventFilter( this );
#else
  mUi->mTimeZoneLabel->setVisible( !mUi->mWholeDayCheck->isChecked() );
  connect( mUi->mTimeZoneLabel, SIGNAL(linkActivated(QString)),
           SLOT(toggleTimeZoneVisibility()) );
#endif

  connect( mUi->mFreeBusyCheck, SIGNAL(toggled(bool)), SLOT(checkDirtyStatus()) );
  connect( mUi->mWholeDayCheck, SIGNAL(toggled(bool)), SLOT(enableTimeEdits()) );
  connect( mUi->mWholeDayCheck, SIGNAL(toggled(bool)), SLOT(checkDirtyStatus()) );
}

IncidenceDateTime::~IncidenceDateTime()
{
  delete mTimeZones;
}

bool IncidenceDateTime::eventFilter( QObject *obj, QEvent *event )
{
  if ( event->type() == QEvent::FocusIn ) {
    if ( obj == mUi->mStartDateEdit ) {
      kDebug() << "emiting startDateTime: " << mUi->mStartDateEdit;
      emit startDateFocus( obj );
    } else if ( obj == mUi->mEndDateEdit ) {
      kDebug() << "emiting endDateTime: " << mUi->mEndDateEdit;
      emit endDateFocus( obj );
    } else if ( obj == mUi->mStartTimeEdit ) {
      kDebug() << "emiting startTimeTime: " << mUi->mStartTimeEdit;
      emit startTimeFocus( obj );
    } else if ( obj == mUi->mEndTimeEdit ) {
      kDebug() << "emiting endTimeTime: " << mUi->mEndTimeEdit;
      emit endTimeFocus( obj );
    }

    return true;
  } else {
    // standard event processing
    return QObject::eventFilter( obj, event );
  }
}

void IncidenceDateTime::load( const KCalCore::Incidence::Ptr &incidence )
{
  mLoadedIncidence = incidence;
  mLoadingIncidence = true;

  // We can only handle events or todos.
  if ( KCalCore::Todo::Ptr todo = IncidenceDateTime::incidence<KCalCore::Todo>() ) {
    load( todo );
  } else if ( KCalCore::Event::Ptr event = IncidenceDateTime::incidence<KCalCore::Event>() ) {
    load( event );
  } else if ( KCalCore::Journal::Ptr journal = IncidenceDateTime::incidence<KCalCore::Journal>() ) {
    load( journal );
  } else {
    kDebug() << "Not an Incidence.";
  }

  // Set the initial times before calling enableTimeEdits, as enableTimeEdits
  // assumes that the initial times are initialized.
  mInitialStartDT = currentStartDateTime();
  mInitialEndDT = currentEndDateTime();

  enableTimeEdits();

  if ( mUi->mTimeZoneComboStart->currentIndex() == 0 ) { // Floating
    mInitialStartDT.setTimeSpec( mInitialStartDT.toLocalZone().timeSpec() );
  }

  if ( mUi->mTimeZoneComboEnd->currentIndex() == 0 ) { // Floating
    mInitialEndDT.setTimeSpec( mInitialEndDT.toLocalZone().timeSpec() );
  }

  mWasDirty = false;
  mLoadingIncidence = false;
}

void IncidenceDateTime::save( const KCalCore::Incidence::Ptr &incidence )
{
  if ( KCalCore::Todo::Ptr todo =
       IncidenceDateTime::incidence<KCalCore::Todo>( incidence ) ) {
    save( todo );
  } else if ( KCalCore::Event::Ptr event =
              IncidenceDateTime::incidence<KCalCore::Event>( incidence ) ) {
    save( event );
  } else if ( KCalCore::Journal::Ptr journal =
              IncidenceDateTime::incidence<KCalCore::Journal>( incidence ) ) {
    save( journal );
  } else {
    Q_ASSERT_X( false, "IncidenceDateTimeEditor::save",
                "Only implemented for todos, events and journals" );
  }
}

bool IncidenceDateTime::isDirty() const
{
  if ( KCalCore::Todo::Ptr todo = IncidenceDateTime::incidence<KCalCore::Todo>() ) {
    return isDirty( todo );
  } else if ( KCalCore::Event::Ptr event = IncidenceDateTime::incidence<KCalCore::Event>() ) {
    return isDirty( event );
  } else if ( KCalCore::Journal::Ptr journal = IncidenceDateTime::incidence<KCalCore::Journal>() ) {
    return isDirty( journal );
  } else {
    Q_ASSERT_X( false, "IncidenceDateTimeEditor::isDirty",
                "Only implemented for todos and events" );
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

QDate IncidenceDateTime::endDate() const
{
  return currentEndDateTime().date();
}

QTime IncidenceDateTime::startTime() const
{
  return currentStartDateTime().time();
}

QTime IncidenceDateTime::endTime() const
{
  return currentEndDateTime().time();
}

/// private slots for General

void IncidenceDateTime::setTimeZonesVisibility( bool visible )
{
#ifndef KDEPIM_MOBILE_UI
  static const QString tz( i18nc( "@action show or hide the time zone widgets", "Time zones" ) );
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
  mUi->mTimeZoneComboEnd->setVisible( visible && type() != KCalCore::Incidence::TypeJournal );
}

void IncidenceDateTime::toggleTimeZoneVisibility()
{
  setTimeZonesVisibility( !mUi->mTimeZoneComboStart->isVisible() );
}

void IncidenceDateTime::updateStartTime( const QTime &newTime )
{
  if ( !newTime.isValid() ) {
    return;
  }

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

  emit startTimeChanged( mCurrentStartDateTime.time() );
  checkDirtyStatus();
}

void IncidenceDateTime::updateStartDate( const QDate &newDate )
{
  if ( !newDate.isValid() ) {
    return;
  }

  const bool dateChanged = mCurrentStartDateTime.date().day() != newDate.day() ||
                           mCurrentStartDateTime.date().month() != newDate.month();

  KDateTime endDateTime = currentEndDateTime();
  int daysep = mCurrentStartDateTime.daysTo( endDateTime );
  mCurrentStartDateTime.setDate( newDate );
  if ( mUi->mEndCheck->isChecked() ) {
    // Only update the end time when it is actually enabled, adjust end time so
    // that the event/todo has the same duration as before.
    endDateTime.setDate( mCurrentStartDateTime.date().addDays( daysep ) );
    mUi->mEndDateEdit->setDate( endDateTime.date() );
  }

  checkDirtyStatus();

  if ( dateChanged ) {
    emit startDateChanged( mCurrentStartDateTime.date() );
  }
}

void IncidenceDateTime::updateStartSpec()
{
  const QDate prevDate = mCurrentStartDateTime.date();

  if ( mUi->mEndCheck->isChecked() &&
       currentEndDateTime().timeSpec() == mCurrentStartDateTime.timeSpec() ) {
    mUi->mTimeZoneComboEnd->selectTimeSpec( mUi->mTimeZoneComboStart->selectedTimeSpec() );
  }

  mCurrentStartDateTime.setTimeSpec( mUi->mTimeZoneComboStart->selectedTimeSpec() );

  const bool dateChanged = mCurrentStartDateTime.date().day() != prevDate.day() ||
                           mCurrentStartDateTime.date().month() != prevDate.month();

  if ( dateChanged ) {
    emit startDateChanged( mCurrentStartDateTime.date() );
  }

  if ( type() == KCalCore::Incidence::TypeJournal ) {
    checkDirtyStatus();
  }
}

/// private slots for Todo

void IncidenceDateTime::enableStartEdit( bool enable )
{
  mUi->mStartDateEdit->setEnabled( enable );

  if ( mUi->mEndCheck->isChecked() || mUi->mStartCheck->isChecked() ) {
    mUi->mWholeDayCheck->setEnabled( true );
    setTimeZoneLabelEnabled( !mUi->mWholeDayCheck->isChecked() );
  } else {
    mUi->mWholeDayCheck->setEnabled( false );
    mUi->mWholeDayCheck->setChecked( false );
    setTimeZoneLabelEnabled( false );
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
    setTimeZoneLabelEnabled( !mUi->mWholeDayCheck->isChecked() );
  } else {
    mUi->mWholeDayCheck->setEnabled( false );
    mUi->mWholeDayCheck->setChecked( false );
    setTimeZoneLabelEnabled( false );
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

  setTimeZoneLabelEnabled( !wholeDayChecked );

  if ( mUi->mStartCheck->isChecked() ) {
    mUi->mStartTimeEdit->setEnabled( !wholeDayChecked );
    mUi->mTimeZoneComboStart->setEnabled( !wholeDayChecked );
    mUi->mTimeZoneComboStart->setFloating( wholeDayChecked, mInitialStartDT.timeSpec() );
  }
  if ( mUi->mEndCheck->isChecked() ) {
    mUi->mEndTimeEdit->setEnabled( !wholeDayChecked );
    mUi->mTimeZoneComboEnd->setEnabled( !wholeDayChecked );
    mUi->mTimeZoneComboEnd->setFloating( wholeDayChecked, mInitialEndDT.timeSpec() );
  }

  /**
     When editing a whole-day event, unchecking mWholeDayCheck shouldn't set both
     times to 00:00. DTSTART must always be smaller than DTEND
   */
  if ( sender() == mUi->mWholeDayCheck && !wholeDayChecked && // Somebody unchecked it, the incidence will now have time.
       mUi->mStartCheck->isChecked() && mUi->mEndCheck->isChecked() && // The incidence has both start and end/due dates
       currentStartDateTime() == currentEndDateTime() ) { // DTSTART == DTEND. This is illegal, lets correct it.
    // Not sure about the best time here... doesn't really matter, when someone unchecks mWholeDayCheck, she will
    // always want to set a time.
    mUi->mStartTimeEdit->setTime( QTime( 0, 0 ) );
    mUi->mEndTimeEdit->setTime( QTime( 1, 0 ) );
  }

#ifndef KDEPIM_MOBILE_UI
  const bool currentlyVisible = mUi->mTimeZoneLabel->text().contains( "&lt;&lt;" );
  setTimeZonesVisibility( !wholeDayChecked && mTimezoneCombosWhereVisibile );
  mTimezoneCombosWhereVisibile = currentlyVisible;
#endif
}

bool IncidenceDateTime::isDirty( const KCalCore::Todo::Ptr &todo ) const
{
  Q_ASSERT( todo );

  const bool hasDateTimes = mUi->mStartCheck->isChecked() ||
                            mUi->mEndCheck->isChecked();

  // First check the start time/date of the todo
  if ( todo->hasStartDate() != mUi->mStartCheck->isChecked() ) {
    return true;
  }

  if ( ( hasDateTimes && todo->allDay() ) != mUi->mWholeDayCheck->isChecked() ) {
    return true;
  }

  if ( todo->hasDueDate() != mUi->mEndCheck->isChecked() ) {
    return true;
  }

  if ( mUi->mStartCheck->isChecked() ) {
    // Use mActiveStartTime. This is the KDateTime::Spec selected on load coming from
    // the combobox. We use this one as it can slightly differ (e.g. missing
    // country code in the incidence time spec) from the incidence.
    if ( currentStartDateTime() != mInitialStartDT ) {
      return true;
    }
  }

  if ( mUi->mEndCheck->isChecked() && currentEndDateTime() != mInitialEndDT ) {
    return true;
  }

  return false;
}

/// Event specific methods

bool IncidenceDateTime::isDirty( const KCalCore::Event::Ptr &event ) const
{
  if ( event->allDay() != mUi->mWholeDayCheck->isChecked() ) {
    return true;
  }

  if ( mUi->mFreeBusyCheck->isChecked() &&
       event->transparency() != KCalCore::Event::Opaque ) {
    return true;
  }

  if ( !mUi->mFreeBusyCheck->isChecked() &&
       event->transparency() != KCalCore::Event::Transparent ) {
    return true;
  }

  if ( event->allDay() ) {
    if ( mUi->mStartDateEdit->date() != mInitialStartDT.date() ||
         mUi->mEndDateEdit->date() != mInitialEndDT.date() ) {
      return true;
    }
  } else {
    if ( currentStartDateTime() != mInitialStartDT ||
         currentEndDateTime() != mInitialEndDT ||
         currentStartDateTime().timeSpec() != mInitialStartDT.timeSpec() ||
         currentEndDateTime().timeSpec() != mInitialEndDT.timeSpec() ) {
      return true;
    }
  }

  return false;
}

bool IncidenceDateTime::isDirty( const KCalCore::Journal::Ptr &journal ) const
{
  if ( journal->allDay() != mUi->mWholeDayCheck->isChecked() ) {
    return true;
  }

  if ( journal->allDay() ) {
    if ( mUi->mStartDateEdit->date() != mInitialStartDT.date() ) {
      return true;
    }
  } else {
    if ( currentStartDateTime() != mInitialStartDT ) {
      return true;
    }
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

void IncidenceDateTime::load( const KCalCore::Event::Ptr &event )
{
  // First en/disable the necessary ui bits and pieces
  mUi->mStartCheck->setVisible( false );
  mUi->mStartCheck->setChecked( true ); // Set to checked so we can reuse enableTimeEdits.
  mUi->mEndCheck->setVisible( false );
  mUi->mEndCheck->setChecked( true ); // Set to checked so we can reuse enableTimeEdits.

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
  connect( mUi->mEndTimeEdit, SIGNAL(timeChanged(QTime)),
           SIGNAL(endTimeChanged(QTime)) );
  connect( mUi->mEndDateEdit, SIGNAL(dateChanged(QDate)),
           SIGNAL(endDateChanged(QDate)) );
  connect( mUi->mTimeZoneComboEnd, SIGNAL(currentIndexChanged(int)),
           SLOT(checkDirtyStatus()) );

  mUi->mWholeDayCheck->setChecked( event->allDay() );
  enableTimeEdits();

  bool isTemplate = false; // TODO
  if ( !isTemplate ) {
    KDateTime startDT = event->dtStart();
    KDateTime endDT = event->dtEnd();
    /*
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
    */
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
  case KCalCore::Event::Transparent:
    mUi->mFreeBusyCheck->setChecked( false );
    break;
  case KCalCore::Event::Opaque:
    mUi->mFreeBusyCheck->setChecked( true );
    break;
  }
}

void IncidenceDateTime::load( const KCalCore::Journal::Ptr &journal )
{
  // First en/disable the necessary ui bits and pieces
  mUi->mStartCheck->setVisible( false );
  mUi->mStartCheck->setChecked( true ); // Set to checked so we can reuse enableTimeEdits.
  mUi->mEndCheck->setVisible( false );
  mUi->mEndCheck->setChecked( true ); // Set to checked so we can reuse enableTimeEdits.
  mUi->mEndDateEdit->setVisible( false );
  mUi->mEndTimeEdit->setVisible( false );
  mUi->mTimeZoneComboEnd->setVisible( false );
  mUi->mEndLabel->setVisible( false );
  mUi->mFreeBusyCheck->setVisible( false );

  // Start time
  connect( mUi->mStartTimeEdit, SIGNAL(timeChanged(QTime)),
           SLOT(updateStartTime(QTime)) );
  connect( mUi->mStartDateEdit, SIGNAL(dateChanged(QDate)),
           SLOT(updateStartDate(QDate)) );
  connect( mUi->mTimeZoneComboStart, SIGNAL(currentIndexChanged(int)),
           SLOT(updateStartSpec()) );

  mUi->mWholeDayCheck->setChecked( journal->allDay() );
  enableTimeEdits();

  bool isTemplate = false; // TODO
  if ( !isTemplate ) {
    KDateTime startDT = journal->dtStart();

    /*
    if ( journal->recurs() && mActiveDate.isValid() ) {
      // Consider the active date when editing recurring journals
      KDateTime kdt( mActiveDate, QTime( 0, 0, 0 ), KSystemTimeZones::local() );
      kdt = kdt.addSecs( -1 );
      startDT.setDate( journal->recurrence()->getNextDateTime( kdt ).date() );
    }
    */
    // Convert UTC to local timezone, if needed (i.e. for kolab #204059)
    if ( startDT.isUtc() ) {
      startDT = startDT.toLocalZone();
    }
    setDateTimes( startDT, KDateTime() );
  } else {
    // set the start/end time from the template, only as a last resort #190545
    if ( !journal->dtStart().isValid() ) {
      setTimes( journal->dtStart(), KDateTime() );
    }
  }
}

void IncidenceDateTime::load( const KCalCore::Todo::Ptr &todo )
{
  // First en/disable the necessary ui bits and pieces
  mUi->mStartCheck->setVisible( true );
  mUi->mStartCheck->setChecked( todo->hasStartDate() );
  mUi->mStartDateEdit->setEnabled( todo->hasStartDate() );
  mUi->mStartTimeEdit->setEnabled( todo->hasStartDate() );
  mUi->mTimeZoneComboStart->setEnabled( todo->hasStartDate() );

  mUi->mEndLabel->setText( i18nc( "@label The due date/time of a to-do", "Due:" ) );
  mUi->mEndCheck->setVisible( true );
  mUi->mEndCheck->setChecked( todo->hasDueDate() );
  mUi->mEndDateEdit->setEnabled( todo->hasDueDate() );
  mUi->mEndTimeEdit->setEnabled( todo->hasDueDate() );
  mUi->mTimeZoneComboEnd->setEnabled( todo->hasDueDate() );

  // These fields where not enabled in the old code either:
  mUi->mFreeBusyCheck->setVisible( false );

  const bool hasDateTimes = mUi->mEndCheck->isChecked() || mUi->mStartCheck->isChecked();
  mUi->mWholeDayCheck->setChecked( hasDateTimes && todo->allDay() );
  mUi->mWholeDayCheck->setEnabled( hasDateTimes );

  // Connect to the right logic
  connect( mUi->mStartCheck, SIGNAL(toggled(bool)), SLOT(enableStartEdit(bool)) );
  connect( mUi->mStartCheck, SIGNAL(toggled(bool)), SIGNAL(startDateTimeToggled(bool)) );
  connect( mUi->mStartDateEdit, SIGNAL(dateChanged(QDate)), SLOT(checkDirtyStatus()) );
  connect( mUi->mStartTimeEdit, SIGNAL(timeChanged(QTime)), SLOT(updateStartTime(QTime)) );
  connect( mUi->mTimeZoneComboStart, SIGNAL(currentIndexChanged(int)), SLOT(checkDirtyStatus()) );

  connect( mUi->mEndCheck, SIGNAL(toggled(bool)), SLOT(enableEndEdit(bool)) );
  connect( mUi->mEndCheck, SIGNAL(toggled(bool)), SIGNAL(endDateTimeToggled(bool)) );
  connect( mUi->mEndDateEdit, SIGNAL(dateChanged(QDate)), SLOT(checkDirtyStatus()) );
  connect( mUi->mEndTimeEdit, SIGNAL(timeChanged(QTime)), SLOT(checkDirtyStatus()) );
  connect( mUi->mEndDateEdit, SIGNAL(dateChanged(QDate)), SIGNAL(endDateChanged(QDate)) );
  connect( mUi->mEndTimeEdit, SIGNAL(timeChanged(QTime)), SIGNAL(endTimeChanged(QTime)) );
  connect( mUi->mTimeZoneComboEnd, SIGNAL(currentIndexChanged(int)), SLOT(checkDirtyStatus()) );

  const KDateTime rightNow = KDateTime( QDate::currentDate(), QTime::currentTime() ).toLocalZone();

  const KDateTime endDT   = todo->hasDueDate() ? todo->dtDue( true/** first */ ) : rightNow;
  const KDateTime startDT = todo->hasStartDate() ? todo->dtStart( true/** first */ ) : rightNow;
  setDateTimes( startDT, endDT );
}

void IncidenceDateTime::save( const KCalCore::Event::Ptr &event )
{
  if ( mUi->mWholeDayCheck->isChecked() ) { // All day event
    event->setAllDay( true );

    // TODO: need to change this.
    KDateTime eventDTStart = currentStartDateTime();
    eventDTStart.setDateOnly( true );
    event->setDtStart( eventDTStart );

    KDateTime eventDTEnd = currentEndDateTime();
    eventDTEnd.setDateOnly( true );

    event->setDtEnd( eventDTEnd );
  } else { // Timed Event
    event->setAllDay( false );

    // set date/time end
    event->setDtStart( currentStartDateTime() );
    event->setDtEnd( currentEndDateTime() );
  }

  // Free == Event::Transparent
  // Busy == Event::Opaque
  event->setTransparency( mUi->mFreeBusyCheck->isChecked() ?
                          KCalCore::Event::Opaque :
                          KCalCore::Event::Transparent );
}

void IncidenceDateTime::save( const KCalCore::Todo::Ptr &todo )
{
  if ( mUi->mStartCheck->isChecked() ) {
    todo->setDtStart( currentStartDateTime() );
    // Set allday must be executed after setDtStart
    todo->setAllDay( mUi->mWholeDayCheck->isChecked() );
  } else {
    todo->setHasStartDate( false );
  }

  if ( mUi->mEndCheck->isChecked() ) {
    todo->setDtDue( currentEndDateTime(), true/** first */ );
    // Set allday must be executed after setDtDue
    todo->setAllDay( mUi->mWholeDayCheck->isChecked() );

    if ( currentEndDateTime() != mInitialEndDT ) {
      // We don't offer any way to edit the current completed occurrence.
      // So, if the due date changes, reset the dtRecurrence
      todo->setDtRecurrence( currentEndDateTime() );
    }
  } else {
    todo->setHasDueDate( false );
  }
}

void IncidenceDateTime::save( const KCalCore::Journal::Ptr &journal )
{
  journal->setAllDay( mUi->mWholeDayCheck->isChecked() );

  if ( mUi->mWholeDayCheck->isChecked() ) { // All day journal
    KDateTime journalDTStart = currentStartDateTime();
    journalDTStart.setDateOnly( true );
    journal->setDtStart( journalDTStart );
  } else { // Timed Journal
    // set date/time end
    journal->setDtStart( currentStartDateTime() );
  }
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
  emit startDateChanged( start.date() );
  emit startTimeChanged( start.time() );
  emit endDateChanged( end.date() );
  emit endTimeChanged( end.time() );
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

void IncidenceDateTime::setStartDate( const QDate &newDate )
{
  mUi->mStartDateEdit->setDate( newDate );
  updateStartDate( newDate );
}

void IncidenceDateTime::setStartTime( const QTime &newTime )
{
  mUi->mStartTimeEdit->setTime( newTime );
  updateStartTime( newTime );
}

bool IncidenceDateTime::startDateTimeEnabled() const
{
  return mUi->mStartCheck->isChecked();
}

bool IncidenceDateTime::endDateTimeEnabled() const
{
  return mUi->mEndCheck->isChecked();
}

bool IncidenceDateTime::isValid() const
{
  if ( startDateTimeEnabled() && endDateTimeEnabled() &&
       currentStartDateTime() > currentEndDateTime() ) {
    if ( mLoadedIncidence->type() == KCalCore::Incidence::TypeEvent ) {
      mLastErrorString = i18nc( "@info",
                                "The event ends before it starts.\n"
                                "Please correct dates and times." );

    } else if ( mLoadedIncidence->type() == KCalCore::Incidence::TypeTodo ) {
      mLastErrorString = i18nc( "@info",
                                "The to-do is due before it starts.\n"
                                "Please correct dates and times." );

    }
    return false;
  } else {
    mLastErrorString.clear();
    return true;
  }
}

void IncidenceDateTime::setTimeZoneLabelEnabled( bool enable )
{
  #ifndef KDEPIM_MOBILE_UI
    mUi->mTimeZoneLabel->setVisible( enable );
  #else
    Q_UNUSED( enable );
  #endif
}

#include "moc_incidencedatetime.cpp"
