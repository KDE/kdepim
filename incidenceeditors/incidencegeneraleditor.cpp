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

#include "incidencegeneraleditor.h"

#include <KCal/Incidence>
#include <KCal/ICalTimeZones>

#include "ui_incidencegeneral.h"

using namespace KCal;

IncidenceGeneralEditor::IncidenceGeneralEditor( Mode mode, QWidget *parent )
  : QWidget( parent )
  , mTimeZones( new ICalTimeZones )
  , mUi( new Ui::IncidenceGeneral )
  , mMode( mode )
{
  mUi->setupUi( this );
  mUi->mAlarmBell->setPixmap( SmallIcon( "task-reminder" ) );
  mUi->mRecurrenceEditButton->setIcon(
    KIconLoader::global()->loadIcon(
      "task-recurring", KIconLoader::Desktop, KIconLoader::SizeSmall ) );
  mUi->mTimeZoneComboStart->setAdditionalTimeZones( mTimeZones );
  mUi->mTimeZoneComboEnd->setAdditionalTimeZones( mTimeZones );
  mUi->mSecrecyCombo->addItems( Incidence::secrecyList() );

  connect( mUi->mHasTimeCheckbox, SIGNAL(toggled(bool)), SLOT(setTimeEditorsEnabled(bool)) );
}

/// public slots

void IncidenceGeneralEditor::setDuration()
{
  QString tmpStr, catStr;
  int hourdiff, minutediff;
  // end<date is an accepted temporary state while typing, but don't show
  // any duration if this happens
  KDateTime startDateTime =
    KDateTime( mCurrStartDateTime, mUi->mTimeZoneComboStart->selectedTimeSpec() );
  KDateTime endDateTime =
    KDateTime( mCurrEndDateTime, mUi->mTimeZoneComboEnd->selectedTimeSpec() ).
    toTimeSpec( startDateTime.timeSpec() );
  if ( startDateTime < endDateTime ) {

    if ( !mUi->mHasTimeCheckbox->isChecked() ) {
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
}

/// private slots

void IncidenceGeneralEditor::emitDateTimeStr()
{
  KLocale *l = KGlobal::locale();

  QString from, to;
  if ( !mUi->mHasTimeCheckbox->isChecked() ) {
    from = l->formatDate( mCurrStartDateTime.date() );
    to = l->formatDate( mCurrEndDateTime.date() );
  } else {
    from = l->formatDateTime( mCurrStartDateTime );
    to = l->formatDateTime( mCurrEndDateTime );
  }
  QString str = i18nc( "@label",
                       "From: %1   To: %2   %3",
                       from, to, mUi->mDurationLabel->text() );

  emit dateTimeStrChanged( str );
}

void IncidenceGeneralEditor::setTimeEditorsEnabled( bool enabled )
{
  /// TodoEditor
  switch ( mMode ) {
  case Todo:
    if( mUi->mStartCheck->isChecked() ) {
      mUi->mStartTimeEdit->setEnabled( enabled );
      mUi->mTimeZoneComboStart->setEnabled( enabled );
      mUi->mTimeZoneComboStart->setFloating( !enabled, mStartSpec );
    }
    if( mUi->mDueCheck->isChecked() ) {
      mUi->mEndTimeEdit->setEnabled( enabled );
      mUi->mTimeZoneComboEnd->setEnabled( enabled );
      mUi->mTimeZoneComboEnd->setFloating( !enabled, mEndSpec );
    }
    break;
  case Event:
    /// EventEditor
    mUi->mStartTimeEdit->setEnabled( enabled );
    mUi->mEndTimeEdit->setEnabled( enabled );

    if ( !enabled ) {
      mUi->mTimeZoneComboStart->setFloating( true );
      mUi->mTimeZoneComboEnd->setFloating( true );
    } else {
      mUi->mTimeZoneComboStart->selectLocalTimeSpec();
      mUi->mTimeZoneComboEnd->selectLocalTimeSpec();
    }

    mStartSpec = mUi->mTimeZoneComboStart->selectedTimeSpec();
    mEndSpec = mUi->mTimeZoneComboEnd->selectedTimeSpec();
    mUi->mTimeZoneComboStart->setEnabled( enabled );
    mUi->mTimeZoneComboEnd->setEnabled( enabled );

    setDuration();
    emitDateTimeStr();
  }
}  

IncidenceGeneralEditor::~IncidenceGeneralEditor()
{
  delete mTimeZones;
  delete mUi;
}

/// Event Editor specifics

EventGeneralEditor::EventGeneralEditor( QWidget *parent )
  : IncidenceGeneralEditor( IncidenceGeneralEditor::Event, parent )
{
  mUi->mTodoSpecifics->setVisible( false );
  mUi->mStartCheck->setVisible( false );
  mUi->mDueCheck->setVisible( false );

  connect( mUi->mHasTimeCheckbox, SIGNAL(toggled(bool)), SLOT(slotHasTimeCheckboxToggled(bool)) );
}

void EventGeneralEditor::slotHasTimeCheckboxToggled( bool checked )
{
  //if(alarmButton->isChecked()) alarmStuffDisable(noTime);
  emit allDayChanged( !checked );
}

/// Todo Editor specifics

TodoGeneralEditor::TodoGeneralEditor( QWidget *parent )
  : IncidenceGeneralEditor( IncidenceGeneralEditor::Todo, parent )
{
  mUi->mStartLabel->setVisible( false );
  mUi->mEndLabel->setVisible( false );
}

