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

#include "incidencealarm.h"
#include "alarmdialog.h"
#include "alarmpresets.h"
#include "incidencedatetime.h"
#ifdef KDEPIM_MOBILE_UI
#include "ui_dialogmoremobile.h"
#else
#include "ui_dialogdesktop.h"
#endif

#include <calendarsupport/kcalprefs.h>

#include <QDebug>

using namespace IncidenceEditorNG;
using namespace CalendarSupport;

#ifdef KDEPIM_MOBILE_UI
IncidenceAlarm::IncidenceAlarm( IncidenceDateTime *dateTime, Ui::EventOrTodoMore *ui )
#else
IncidenceAlarm::IncidenceAlarm( IncidenceDateTime *dateTime, Ui::EventOrTodoDesktop *ui )
#endif
  : mUi( ui ),
    mDateTime( dateTime ),
    mEnabledAlarmCount( 0 ),
    mIsTodo( false )
{
  setObjectName( "IncidenceAlarm" );

  mUi->mAlarmPresetCombo->insertItems( 0, AlarmPresets::availablePresets() );
  mUi->mAlarmPresetCombo->setCurrentIndex( AlarmPresets::defaultPresetIndex() );
  updateButtons();

  connect( mDateTime, SIGNAL(startDateTimeToggled(bool)),
           SLOT(handleDateTimeToggle()) );
  connect( mDateTime, SIGNAL(endDateTimeToggled(bool)),
           SLOT(handleDateTimeToggle()) );
  connect( mUi->mAlarmAddPresetButton, SIGNAL(clicked()),
           SLOT(newAlarmFromPreset()) );
  connect( mUi->mAlarmList, SIGNAL(itemSelectionChanged()),
           SLOT(updateButtons()) );
  connect( mUi->mAlarmNewButton, SIGNAL(clicked()),
           SLOT(newAlarm()));
  connect( mUi->mAlarmConfigureButton, SIGNAL(clicked()),
           SLOT(editCurrentAlarm()) );
  connect( mUi->mAlarmToggleButton, SIGNAL(clicked()),
           SLOT(toggleCurrentAlarm()) );
  connect( mUi->mAlarmRemoveButton, SIGNAL(clicked()),
           SLOT(removeCurrentAlarm()) );
}

void IncidenceAlarm::load( const KCalCore::Incidence::Ptr &incidence )
{
  mLoadedIncidence = incidence;
  // We must be sure that the date/time in mDateTime is the correct date time.
  // So don't depend on CombinedIncidenceEditor or whatever external factor to
  // load the date/time before loading the recurrence
  mDateTime->load( incidence );

  mAlarms.clear();
  foreach ( const KCalCore::Alarm::Ptr &alarm, incidence->alarms() ) {
    mAlarms.append( KCalCore::Alarm::Ptr( new KCalCore::Alarm( *alarm.data() ) ) );
  }

  mIsTodo = incidence->type() == KCalCore::Incidence::TypeTodo;
  if ( mIsTodo ) {
    mUi->mAlarmPresetCombo->clear();
    mUi->mAlarmPresetCombo->addItems( AlarmPresets::availablePresets( AlarmPresets::BeforeEnd ) );
  } else {
    mUi->mAlarmPresetCombo->clear();
    mUi->mAlarmPresetCombo->addItems( AlarmPresets::availablePresets( AlarmPresets::BeforeStart ) );
  }
  mUi->mAlarmPresetCombo->setCurrentIndex( AlarmPresets::defaultPresetIndex() );

  handleDateTimeToggle();
  mWasDirty = false;

  updateAlarmList();
}

void IncidenceAlarm::save( const KCalCore::Incidence::Ptr &incidence )
{
  incidence->clearAlarms();
  KCalCore::Alarm::List::ConstIterator it;
  for ( it = mAlarms.constBegin(); it != mAlarms.constEnd(); ++it ) {
    KCalCore::Alarm::Ptr al( new KCalCore::Alarm( *(*it) ) );
    al->setParent( incidence.data() );
    // We need to make sure that both lists are the same in the end for isDirty.
    Q_ASSERT( *al == *(*it) );
    incidence->addAlarm( al );
  }
}

bool IncidenceAlarm::isDirty() const
{
  if ( mLoadedIncidence->alarms().count() != mAlarms.count() ) {
    return true;
  }

  if ( !mLoadedIncidence->alarms().isEmpty() ) {
    const KCalCore::Alarm::List initialAlarms = mLoadedIncidence->alarms();

    if ( initialAlarms.count() != mAlarms.count() ) {
      return true; // The number of alarms has changed
    }

    // Note: Not the most efficient algorithm but I'm assuming that we're only
    //       dealing with a couple, at most tens of alarms. The idea is we check
    //       if all currently enabled alarms are also in the incidence. The
    //       disabled alarms are not changed by our code at all, so we assume that
    //       they're still there.
    foreach ( const KCalCore::Alarm::Ptr &alarm, mAlarms ) {
      bool found = false;
      foreach ( const KCalCore::Alarm::Ptr &initialAlarm, initialAlarms ) {
        if ( *alarm == *initialAlarm ) {
          found  = true;
          break;
        }
      }

      if ( !found ) {
        // There was an alarm in the mLoadedIncidence->alarms() that wasn't found
        // in mLastAlarms. This means that one of the alarms was modified.
        return true;
      }
    }
  }

 return false;
}

void IncidenceAlarm::editCurrentAlarm()
{
  KCalCore::Alarm::Ptr currentAlarm = mAlarms.at( mUi->mAlarmList->currentRow() );

#ifdef KDEPIM_MOBILE_UI
  QPointer<AlarmDialog> dialog( new AlarmDialog( mLoadedIncidence->type() ) );
#else
  QPointer<AlarmDialog> dialog( new AlarmDialog( mLoadedIncidence->type(), mUi->mTabWidget ) );
#endif
  dialog->load( currentAlarm );

  dialog->setAllowBeginReminders( mDateTime->startDateTimeEnabled() );
  dialog->setAllowEndReminders( mDateTime->endDateTimeEnabled() );

  if ( dialog->exec() == KDialog::Accepted ) {
    dialog->save( currentAlarm );
    updateAlarmList();
    checkDirtyStatus();
  }
  delete dialog;
}

void IncidenceAlarm::handleDateTimeToggle()
{
  QWidget *parent = mUi->mAlarmPresetCombo->parentWidget();  // the parent of a toplevel widget
  if ( parent ) {
    parent->setEnabled( mDateTime->startDateTimeEnabled() || mDateTime->endDateTimeEnabled() );
  }

  mUi->mAlarmPresetCombo->setEnabled( mDateTime->endDateTimeEnabled() );
  mUi->mAlarmAddPresetButton->setEnabled( mDateTime->endDateTimeEnabled() );

#ifndef KDEPIM_MOBILE_UI
  mUi->mQuickAddReminderLabel->setEnabled( mDateTime->endDateTimeEnabled() );
#endif
}

void IncidenceAlarm::newAlarm()
{
#ifdef KDEPIM_MOBILE_UI
  QPointer<AlarmDialog> dialog( new AlarmDialog( mLoadedIncidence->type() ) );
#else
  QPointer<AlarmDialog> dialog( new AlarmDialog( mLoadedIncidence->type(), mUi->mTabWidget ) );
#endif
  const int reminderOffset = KCalPrefs::instance()->reminderTime();

  if ( reminderOffset >= 0 ) {
    dialog->setOffset( reminderOffset );
  } else {
    dialog->setOffset( DEFAULT_REMINDER_OFFSET );
  }
  dialog->setUnit( AlarmDialog::Minutes );
  if ( mIsTodo && mDateTime->endDateTimeEnabled() ) {
    dialog->setWhen( AlarmDialog::BeforeEnd );
  } else {
    dialog->setWhen( AlarmDialog::BeforeStart );
  }

  dialog->setAllowBeginReminders( mDateTime->startDateTimeEnabled() );
  dialog->setAllowEndReminders( mDateTime->endDateTimeEnabled() );

  if ( dialog->exec() == KDialog::Accepted ) {
    KCalCore::Alarm::Ptr newAlarm( new KCalCore::Alarm( 0 ) );
    dialog->save( newAlarm );
    newAlarm->setEnabled( true );
    mAlarms.append( newAlarm );
    updateAlarmList();
    checkDirtyStatus();
  }
  delete dialog;
}

void IncidenceAlarm::newAlarmFromPreset()
{
  if ( mIsTodo ) {
    mAlarms.append(
      AlarmPresets::preset( AlarmPresets::BeforeEnd, mUi->mAlarmPresetCombo->currentText() ) );
  } else {
    mAlarms.append(
      AlarmPresets::preset( AlarmPresets::BeforeStart, mUi->mAlarmPresetCombo->currentText() ) );
  }

  updateAlarmList();
  checkDirtyStatus();
}

void IncidenceAlarm::removeCurrentAlarm()
{
  Q_ASSERT( mUi->mAlarmList->selectedItems().size() == 1 );
  const int curAlarmIndex = mUi->mAlarmList->currentRow();
  delete mUi->mAlarmList->takeItem( curAlarmIndex );
  mAlarms.remove( curAlarmIndex );

  updateAlarmList();
  updateButtons();
  checkDirtyStatus();
}

void IncidenceAlarm::toggleCurrentAlarm()
{
  Q_ASSERT( mUi->mAlarmList->selectedItems().size() == 1 );
  const int curAlarmIndex = mUi->mAlarmList->currentRow();
  KCalCore::Alarm::Ptr alarm = mAlarms.at( curAlarmIndex );
  alarm->setEnabled( !alarm->enabled() );

  updateButtons();
  updateAlarmList();
  checkDirtyStatus();
}

void IncidenceAlarm::updateAlarmList()
{
  const int prevEnabledAlarmCount = mEnabledAlarmCount;
  mEnabledAlarmCount = 0;

  const QModelIndex currentIndex = mUi->mAlarmList->currentIndex();
  mUi->mAlarmList->clear();
  foreach ( const KCalCore::Alarm::Ptr &alarm, mAlarms ) {
    mUi->mAlarmList->addItem( stringForAlarm( alarm ) );
    if ( alarm->enabled() ) {
      ++mEnabledAlarmCount;
    }
  }

  mUi->mAlarmList->setCurrentIndex( currentIndex );
  if ( prevEnabledAlarmCount != mEnabledAlarmCount ) {
    emit alarmCountChanged( mEnabledAlarmCount );
  }
}

void IncidenceAlarm::updateButtons()
{
  if ( mUi->mAlarmList->count() > 0 && mUi->mAlarmList->selectedItems().count() > 0 ) {
    mUi->mAlarmConfigureButton->setEnabled( true );
    mUi->mAlarmRemoveButton->setEnabled( true );
    mUi->mAlarmToggleButton->setEnabled( true );
    KCalCore::Alarm::Ptr selAlarm;
    if ( mUi->mAlarmList->currentIndex().isValid() ) {
      selAlarm = mAlarms.at( mUi->mAlarmList->currentIndex().row() );
    }
    if ( selAlarm && selAlarm->enabled() ) {
      mUi->mAlarmToggleButton->setText( i18nc( "Disable currently selected reminder", "Disable" ) );
    } else {
      mUi->mAlarmToggleButton->setText( i18nc( "Enable currently selected reminder", "Enable" ) );
    }
  } else {
    mUi->mAlarmConfigureButton->setEnabled( false );
    mUi->mAlarmRemoveButton->setEnabled( false );
    mUi->mAlarmToggleButton->setEnabled( false );
  }
}

QString IncidenceAlarm::stringForAlarm( const KCalCore::Alarm::Ptr &alarm )
{
  Q_ASSERT( alarm );

  QString action;
  switch( alarm->type() ) {
  case KCalCore::Alarm::Display:
    action = i18n( "Display a dialog" );
    break;
  case KCalCore::Alarm::Procedure:
    action = i18n( "Execute a script" );
    break;
  case KCalCore::Alarm::Email:
    action = i18n( "Send an email" );
    break;
  case KCalCore::Alarm::Audio:
    action = i18n( "Play an audio file" );
    break;
  default:
    action = i18n( "Invalid Reminder." );
    return action;
  }

  QString offsetUnit =
    i18nc( "The reminder is set to X minutes before/after the event", "minutes" );

  const int offset = alarm->hasStartOffset() ? alarm->startOffset().asSeconds() / 60 :
                     alarm->endOffset().asSeconds() / 60; // make minutes

  int useoffset = offset;
  if ( offset % ( 24 * 60 ) == 0 && offset != 0 ) { // divides evenly into days?
    useoffset =  offset / 60 / 24;
    offsetUnit = i18nc( "The reminder is set to X days before/after the event", "days" );
  } else if ( offset % 60 == 0 && offset != 0 ) { // divides evenly into hours?
    offsetUnit = i18nc( "The reminder is set to X hours before/after the event", "hours" );
    useoffset = offset / 60;
  }

  QString repeatStr;
  if ( alarm->repeatCount() > 0 ) {
    repeatStr = i18nc( "The reminder is configured to repeat after snooze", "(Repeats)" );
  }

  if ( alarm->enabled() ) {
    if ( useoffset > 0 && alarm->hasStartOffset() ) {
      if ( mIsTodo ) {
        return i18n( "%1 %2 %3 after the to-do started %4",
                     action, useoffset, offsetUnit, repeatStr );
      } else {
        return i18n( "%1 %2 %3 after the event started %4",
                     action, useoffset, offsetUnit, repeatStr );
      }
    } else if ( useoffset < 0 && alarm->hasStartOffset() ) {
      if ( mIsTodo ) {
        return i18n( "%1 %2 %3 before the to-do starts %4",
                     action, qAbs( useoffset ), offsetUnit, repeatStr );
      } else {
        return i18n( "%1 %2 %3 before the event starts %4",
                     action, qAbs( useoffset ), offsetUnit, repeatStr );
      }
    } else if ( useoffset > 0 && alarm->hasEndOffset() ) {
      if ( mIsTodo ) {
        return i18n( "%1 %2 %3 after the to-do is due %4",
                     action, useoffset, offsetUnit, repeatStr );
      } else {
        return i18n( "%1 %2 %3 after the event ends %4",
                     action, useoffset, offsetUnit, repeatStr );
      }
    } else if ( useoffset < 0 && alarm->hasEndOffset() ) {
      if ( mIsTodo ) {
        return i18n( "%1 %2 %3 before the to-do is due %4",
                     action, qAbs( useoffset ), offsetUnit, repeatStr );
      } else {
        return i18n( "%1 %2 %3 before the event ends %4",
                     action, qAbs( useoffset ), offsetUnit, repeatStr );
      }
    }
  } else {
    if ( useoffset > 0 && alarm->hasStartOffset() ) {
      if ( mIsTodo ) {
        return i18n( "%1 %2 %3 after the to-do started %4 (Disabled)",
                     action, useoffset, offsetUnit, repeatStr );
      } else {
        return i18n( "%1 %2 %3 after the event started %4 (Disabled)",
                     action, useoffset, offsetUnit, repeatStr );
      }

    } else if ( useoffset < 0 && alarm->hasStartOffset() ) {
      if ( mIsTodo ) {
        return i18n( "%1 %2 %3 before the to-do starts %4 (Disabled)",
                     action, qAbs( useoffset ), offsetUnit, repeatStr );
      } else {
        return i18n( "%1 %2 %3 before the event starts %4 (Disabled)",
                     action, qAbs( useoffset ), offsetUnit, repeatStr );
      }
    } else if ( useoffset > 0 && alarm->hasEndOffset() ) {
      if ( mIsTodo ) {
        return i18n( "%1 %2 %3 after the to-do is due %4 (Disabled)",
                     action, useoffset, offsetUnit, repeatStr );
      } else {
        return i18n( "%1 %2 %3 after the event ends %4 (Disabled)",
                     action, useoffset, offsetUnit, repeatStr );
      }
    } else if ( useoffset < 0 && alarm->hasEndOffset() ) {
      if ( mIsTodo ) {
        return i18n( "%1 %2 %3 before the to-do is due %4 (Disabled)",
                     action, qAbs( useoffset ), offsetUnit, repeatStr );
      } else {
        return i18n( "%1 %2 %3 before the event ends %4 (Disabled)",
                     action, qAbs( useoffset ), offsetUnit, repeatStr );
      }
    }
  }

  // useoffset == 0
  if ( alarm->enabled() ) {
    if ( mIsTodo && alarm->hasStartOffset() ) {
      return i18n( "%1 when the to-do starts", action );
    } else if ( alarm->hasStartOffset() ) {
      return i18n( "%1 when the event starts", action );
    } else if ( mIsTodo && alarm->hasEndOffset() ) {
      return i18n( "%1 when the to-do is due", action );
    } else {
      return i18n( "%1 when the event ends", action );
    }
  } else {
    if ( mIsTodo && alarm->hasStartOffset() ) {
      return i18n( "%1 when the to-do starts (Disabled)", action );
    } else if ( alarm->hasStartOffset() ) {
      return i18n( "%1 when the event starts (Disabled)", action );
    } else if ( mIsTodo && alarm->hasEndOffset() ) {
      return i18n( "%1 when the to-do is due (Disabled)", action );
    } else {
      return i18n( "%1 when the event ends (Disabled)", action );
    }
  }
}
