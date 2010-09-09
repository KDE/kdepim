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

#include "alarmdialog.h"
#include "editorconfig.h"
#include "ui_alarmdialog.h"

#include <KPIMUtils/Email>

using namespace IncidenceEditorNG;
using namespace KCalCore;

AlarmDialog::AlarmDialog( KCalCore::Incidence::IncidenceType incidenceType, QWidget *parent )
  : KDialog( parent )
  ,  mUi( new Ui::AlarmDialog )
  , mIncidenceType( incidenceType )
  , mAllowBeginReminders( true )
  , mAllowEndReminders( true )
{
  setWindowTitle( i18n( "Create a new alarm" ) );
  mUi->setupUi( mainWidget() );

  const int defaultReminderTime = IncidenceEditorNG::EditorConfig::instance()->reminderTime();
  mUi->mAlarmOffset->setValue( defaultReminderTime );

  int defaultReminderUnits = IncidenceEditorNG::EditorConfig::instance()->reminderTimeUnits();
  if ( defaultReminderUnits < 0 || defaultReminderUnits > 2 )
    defaultReminderUnits = 0; // minutes

  mUi->mOffsetUnit->setCurrentIndex( defaultReminderUnits );

  if ( IncidenceEditorNG::EditorConfig::instance()->defaultAudioFileReminders() )
    mUi->mSoundFile->setUrl( IncidenceEditorNG::EditorConfig::instance()->audioFilePath() );

  fillCombo();
}

void AlarmDialog::load( const Alarm::Ptr &alarm )
{
  if ( !alarm ) {
    return;
  }

  setWindowTitle( i18n( "Edit existing alarm" ) );

  // Offsets
  int offset;
  int beforeafterpos = 0;

  if ( alarm->hasEndOffset() ) {
    beforeafterpos = 2;
    offset = alarm->endOffset().asSeconds();
  } else {
    // TODO: Also allow alarms at fixed times, not relative to start/end
    offset = alarm->startOffset().asSeconds();
  }
  // Negative offset means before the start/end...
  if ( offset < 0 ) {
    offset = -offset;
  } else {
    ++beforeafterpos;
  }
  mUi->mBeforeAfter->setCurrentIndex( beforeafterpos );

  offset = offset / 60; // make minutes
  int useoffset = offset;

  if ( offset % ( 24 * 60 ) == 0 && offset > 0 ) { // divides evenly into days?
    useoffset = offset / ( 24 * 60 );
    mUi->mOffsetUnit->setCurrentIndex( 2 );
  } else if ( offset % 60 == 0 && offset > 0 ) { // divides evenly into hours?
    useoffset = offset / 60;
    mUi->mOffsetUnit->setCurrentIndex( 1 );
  } else {
    useoffset = offset;
    mUi->mOffsetUnit->setCurrentIndex( 0 );
  }
  mUi->mAlarmOffset->setValue( useoffset );

  // Repeating
  mUi->mRepeats->setChecked( alarm->repeatCount() > 0 );
  if ( alarm->repeatCount() > 0 ) {
    mUi->mRepeatCount->setValue( alarm->repeatCount() );
    mUi->mRepeatInterval->setValue( alarm->snoozeTime().asSeconds() / 60 ); // show as minutes
  }
  int id = 0;

  switch ( alarm->type() ) {
  case Alarm::Audio:
    mUi->mTypeCombo->setCurrentIndex( 1 );
    mUi->mSoundFile->setUrl( alarm->audioFile() );
    id = 1;
    break;
  case Alarm::Procedure:
    mUi->mTypeCombo->setCurrentIndex( 2 );
    mUi->mApplication->setUrl( alarm->programFile() );
    mUi->mAppArguments->setText( alarm->programArguments() );
    id = 2;
    break;
  case Alarm::Email:
  {
    mUi->mTypeCombo->setCurrentIndex( 3 );
    Person::List addresses = alarm->mailAddresses();
    QStringList add;
    for ( Person::List::ConstIterator it = addresses.constBegin();
          it != addresses.constEnd(); ++it ) {
      add << (*it)->fullName();
    }
    mUi->mEmailAddress->setText( add.join( ", " ) );
    mUi->mEmailText->setPlainText( alarm->mailText() );
    id = 3;
    break;
  }
  case Alarm::Display:
  case Alarm::Invalid:
  default:
    mUi->mTypeCombo->setCurrentIndex( 0 );
    mUi->mDisplayText->setPlainText( alarm->text() );
    break;
  }

  mUi->mTypeStack->setCurrentIndex( id );
  if ( alarm->audioFile().isEmpty() &&
       IncidenceEditorNG::EditorConfig::instance()->defaultAudioFileReminders() ) {
    mUi->mSoundFile->setUrl( IncidenceEditorNG::EditorConfig::instance()->audioFilePath() );
  }
}

void AlarmDialog::save( const Alarm::Ptr &alarm ) const
{
  // Offsets
  int offset = mUi->mAlarmOffset->value() * 60; // minutes
  int offsetunit = mUi->mOffsetUnit->currentIndex();
  if ( offsetunit >= 1 ) {
    offset *= 60; // hours
  }
  if ( offsetunit >= 2 ) {
    offset *= 24; // days
  }
  if ( offsetunit >= 3 ) {
    offset *= 7; // weeks
  }

  const int beforeafterpos = mUi->mBeforeAfter->currentIndex();
  if ( beforeafterpos % 2 == 0 ) { // before -> negative
    offset = -offset;
  }

  // Note: if this triggers, fix the logic at the place causing it. It really makes
  // no sense to have both disabled.
  Q_ASSERT( mAllowBeginReminders || mAllowEndReminders );

  // TODO: Add possibility to specify a given time for the reminder
  if ( mAllowBeginReminders && beforeafterpos == 0 ) // before start
    alarm->setStartOffset( Duration( offset ) );
  else if ( mAllowBeginReminders && beforeafterpos == 1 ) // after start
    alarm->setStartOffset( Duration( offset ) );

  // We assume that if mAllowBeginReminders is not set, that mAllowBeginReminders
  // is set.
  if ( !mAllowBeginReminders && beforeafterpos == 0 ) // before end
    alarm->setStartOffset( Duration( offset ) );
  else if ( !mAllowBeginReminders && beforeafterpos == 1 ) // after end
    alarm->setStartOffset( Duration( offset ) );
  else if ( beforeafterpos == 2 ) // before end
    alarm->setStartOffset( Duration( offset ) );
  else if ( beforeafterpos == 3 ) // after end
    alarm->setStartOffset( Duration( offset ) );

  if ( beforeafterpos / 2 == 0 ) { // start offset
    alarm->setStartOffset( Duration( offset ) );
  } else {
    alarm->setEndOffset( Duration( offset ) );
  }

  // Repeating
  if ( mUi->mRepeats->isChecked() ) {
    alarm->setRepeatCount( mUi->mRepeatCount->value() );
    alarm->setSnoozeTime( mUi->mRepeatInterval->value() * 60 ); // convert back to seconds
  } else {
    alarm->setRepeatCount( 0 );
  }

  if ( mUi->mTypeCombo->currentIndex() == 1 ) { // Audio
    alarm->setAudioAlarm( mUi->mSoundFile->url().toLocalFile() );
  } else if ( mUi->mTypeCombo->currentIndex() == 2 ) { // Application / script
    alarm->setProcedureAlarm( mUi->mApplication->url().toLocalFile(),
                              mUi->mAppArguments->text() );
  } else if ( mUi->mTypeCombo->currentIndex() == 3 ) { // Email
    QStringList addresses = KPIMUtils::splitAddressList( mUi->mEmailAddress->text() );
    Person::List add;
    for ( QStringList::Iterator it = addresses.begin(); it != addresses.end(); ++it ) {
      add << Person::fromFullName( *it );
    }
    // TODO: Add a subject line and possibilities for attachments
    alarm->setEmailAlarm( QString(), mUi->mEmailText->toPlainText(), add );
  } else { // Display
    alarm->setDisplayAlarm( mUi->mDisplayText->toPlainText() );
  }
}

void AlarmDialog::fillCombo()
{
  QStringList items;

  if ( mIncidenceType == Incidence::TypeTodo ) {
    mUi->mBeforeAfter->clear();

    if ( mAllowBeginReminders )
      items << i18n( "Before the to-do starts" ) << i18n( "After the to-do starts" );

    if ( mAllowEndReminders )
      items << i18n( "Before the to-do is due" ) << i18n( "After the to-do is due" );

  } else {

    if ( mAllowEndReminders )
      items << i18n( "Before the event starts" ) << i18n( "After the event starts" );

    if ( mAllowEndReminders )
      items << i18n( "Before the event ends" ) << i18n( "After the event ends" );
  }

  mUi->mBeforeAfter->clear();
  mUi->mBeforeAfter->addItems( items );
}

void AlarmDialog::setAllowBeginReminders( bool allow )
{
  mAllowBeginReminders = allow;
  fillCombo();
}

void AlarmDialog::setAllowEndReminders( bool allow )
{
  mAllowEndReminders = allow;
  fillCombo();
}

void AlarmDialog::setOffset( int offset )
{
  Q_ASSERT( offset > 0 );
  mUi->mAlarmOffset->setValue( offset );
}

void AlarmDialog::setUnit( Unit unit )
{
  mUi->mOffsetUnit->setCurrentIndex( unit );
}

void AlarmDialog::setWhen( When when )
{
  Q_ASSERT( when <= mUi->mBeforeAfter->count() );
  mUi->mBeforeAfter->setCurrentIndex( when );
}

