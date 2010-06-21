#include "alarmdialog.h"

#include <KCal/Alarm>
#include <KPIMUtils/Email>

#include "ui_alarmdialog.h"

using namespace IncidenceEditorsNG;
using namespace KCal;

AlarmDialog::AlarmDialog()
  : mUi( new Ui::AlarmDialog )
{
  setWindowTitle( i18n( "Create a new alarm" ) );
  mUi->setupUi( mainWidget() );
}

void AlarmDialog::load( Alarm *alarm )
{
  setWindowTitle( i18n( "Edit existing alarm" ) );
  if ( !alarm ) {
    return;
  }

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
    QList<Person> addresses = alarm->mailAddresses();
    QStringList add;
    for ( QList<Person>::ConstIterator it = addresses.constBegin();
          it != addresses.constEnd(); ++it ) {
      add << (*it).fullName();
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
}

void AlarmDialog::save( Alarm *alarm ) const
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

  int beforeafterpos = mUi->mBeforeAfter->currentIndex();
  if ( beforeafterpos % 2 == 0 ) { // before -> negative
    offset = -offset;
  }

  // TODO: Add possibility to specify a given time for the reminder
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
    QList<Person> add;
    for ( QStringList::Iterator it = addresses.begin(); it != addresses.end(); ++it ) {
      add << Person::fromFullName( *it );
    }
    // TODO: Add a subject line and possibilities for attachments
    alarm->setEmailAlarm( QString(), mUi->mEmailText->toPlainText(), add );
  } else { // Display
    alarm->setDisplayAlarm( mUi->mDisplayText->toPlainText() );
  }
}

