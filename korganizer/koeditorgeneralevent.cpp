/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <tqtooltip.h>
#include <tqlayout.h>
#include <tqvbox.h>
#include <tqbuttongroup.h>
#include <tqvgroupbox.h>
#include <tqwidgetstack.h>
#include <tqspinbox.h>
#include <tqdatetime.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqpushbutton.h>
#include <tqwhatsthis.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <ktextedit.h>

#include <libkcal/event.h>
#include <libkcal/incidenceformatter.h>

#include "ktimeedit.h"
#include <libkdepim/kdateedit.h>

#include "koprefs.h"
#include "koglobals.h"

#include "koeditorgeneralevent.h"
#include "koeditorgeneralevent.moc"

KOEditorGeneralEvent::KOEditorGeneralEvent(TQObject* parent,
                                           const char* name) :
  KOEditorGeneral( parent, name)
{
  connect( this, TQT_SIGNAL( dateTimesChanged( const TQDateTime &, const TQDateTime & )),
           TQT_SLOT( setDuration() ) );
  connect( this, TQT_SIGNAL( dateTimesChanged( const TQDateTime &, const TQDateTime & )),
           TQT_SLOT( emitDateTimeStr() ));
}

KOEditorGeneralEvent::~KOEditorGeneralEvent()
{
}

void KOEditorGeneralEvent::finishSetup()
{
  TQWidget::setTabOrder( mSummaryEdit, mLocationEdit );
  TQWidget::setTabOrder( mLocationEdit, mStartDateEdit );
  TQWidget::setTabOrder( mStartDateEdit, mStartTimeEdit );
  TQWidget::setTabOrder( mStartTimeEdit, mEndDateEdit );
  TQWidget::setTabOrder( mEndDateEdit, mEndTimeEdit );
  TQWidget::setTabOrder( mEndTimeEdit, mAlldayEventCheckbox );
  TQWidget::setTabOrder( mAlldayEventCheckbox, mRecEditButton );
  TQWidget::setTabOrder( mRecEditButton, mAlarmButton );
  TQWidget::setTabOrder( mAlarmButton, mAlarmTimeEdit );
  TQWidget::setTabOrder( mAlarmTimeEdit, mAlarmIncrCombo );
  TQWidget::setTabOrder( mAlarmIncrCombo, mAlarmAdvancedButton );
  TQWidget::setTabOrder( mAlarmAdvancedButton, mFreeTimeCombo );
  TQWidget::setTabOrder( mFreeTimeCombo, mDescriptionEdit );
  TQWidget::setTabOrder( mDescriptionEdit, mCategoriesButton );
  TQWidget::setTabOrder( mCategoriesButton, mSecrecyCombo );

  mSummaryEdit->setFocus();
}

void KOEditorGeneralEvent::initTime(TQWidget *parent,TQBoxLayout *topLayout)
{
  TQBoxLayout *timeLayout = new TQVBoxLayout(topLayout);

  TQGroupBox *timeGroupBox = new TQGroupBox(1,TQGroupBox::Horizontal,
                                          i18n("Date && Time"),parent);
  TQWhatsThis::add( timeGroupBox,
       i18n("Sets options related to the date and time of the "
            "event or to-do.") );
  timeLayout->addWidget(timeGroupBox);

  TQFrame *timeBoxFrame = new TQFrame(timeGroupBox);

  TQGridLayout *layoutTimeBox = new TQGridLayout( timeBoxFrame );
  layoutTimeBox->setSpacing(topLayout->spacing());
  layoutTimeBox->setColStretch( 3, 1 );

  mStartDateLabel = new TQLabel(i18n("&Start:"),timeBoxFrame);
  layoutTimeBox->addWidget(mStartDateLabel,0,0);

  mStartDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mStartDateEdit,0,1);
  mStartDateLabel->setBuddy( mStartDateEdit );

  mStartTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mStartTimeEdit,0,2);

  mEndDateLabel = new TQLabel(i18n("&End:"),timeBoxFrame);
  layoutTimeBox->addWidget(mEndDateLabel,1,0);

  mEndDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mEndDateEdit,1,1);
  mEndDateLabel->setBuddy( mEndDateEdit );

  mEndTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mEndTimeEdit,1,2);

  mAlldayEventCheckbox = new TQCheckBox(i18n("All-&day"),timeBoxFrame);
  layoutTimeBox->addWidget( mAlldayEventCheckbox, 0, 3 );
  connect(mAlldayEventCheckbox, TQT_SIGNAL(toggled(bool)),TQT_SLOT(associateTime(bool)));

  mDurationLabel = new TQLabel( timeBoxFrame );
  layoutTimeBox->addWidget( mDurationLabel, 1, 3 );

  // time widgets are checked if they contain a valid time
  connect(mStartTimeEdit, TQT_SIGNAL(timeChanged(TQTime)),
          this, TQT_SLOT(startTimeChanged(TQTime)));
  connect(mEndTimeEdit, TQT_SIGNAL(timeChanged(TQTime)),
          this, TQT_SLOT(endTimeChanged(TQTime)));

  // date widgets are checked if they contain a valid date
  connect(mStartDateEdit, TQT_SIGNAL(dateChanged(const TQDate&)),
          this, TQT_SLOT(startDateChanged(const TQDate&)));
  connect(mEndDateEdit, TQT_SIGNAL(dateChanged(const TQDate&)),
          this, TQT_SLOT(endDateChanged(const TQDate&)));

  TQLabel *label = new TQLabel( i18n( "Recurrence:" ), timeBoxFrame );
  layoutTimeBox->addWidget( label, 2, 0 );
  TQBoxLayout *recLayout = new TQHBoxLayout();
  layoutTimeBox->addMultiCellLayout( recLayout, 2, 2, 1, 4 );
  mRecEditButton = new TQPushButton( timeBoxFrame );
  mRecEditButton->setIconSet( KOGlobals::self()->smallIconSet( "recur", 16 ) );
  recLayout->addWidget( mRecEditButton );
  connect( mRecEditButton, TQT_SIGNAL(clicked()), TQT_SIGNAL(editRecurrence()) );
  mRecEditLabel = new TQLabel( TQString(), timeBoxFrame );
  recLayout->addWidget( mRecEditLabel );
  recLayout->addStretch( 1 );

  label = new TQLabel( i18n("Reminder:"), timeBoxFrame );
  layoutTimeBox->addWidget( label, 3, 0 );
  TQBoxLayout *alarmLineLayout = new TQHBoxLayout();
  layoutTimeBox->addMultiCellLayout( alarmLineLayout, 3, 3, 1, 4 );
  initAlarm( timeBoxFrame, alarmLineLayout );
  alarmLineLayout->addStretch( 1 );

  TQBoxLayout *secLayout = new TQHBoxLayout();
  layoutTimeBox->addLayout( secLayout, 0, 4 );
  initSecrecy( timeBoxFrame, secLayout );

  TQBoxLayout *classLayout = new TQHBoxLayout();
  layoutTimeBox->addLayout( classLayout, 1, 4 );
  initClass( timeBoxFrame, classLayout );
}

void KOEditorGeneralEvent::initClass(TQWidget *parent,TQBoxLayout *topLayout)
{
  TQBoxLayout *classLayout = new TQHBoxLayout(topLayout);

  TQLabel *freeTimeLabel = new TQLabel(i18n("S&how time as:"),parent);
  TQString whatsThis = i18n("Sets how this time will appear on your Free/Busy "
                           "information.");
  TQWhatsThis::add( freeTimeLabel, whatsThis );
  classLayout->addWidget(freeTimeLabel);

  mFreeTimeCombo = new TQComboBox(false, parent);
  TQWhatsThis::add( mFreeTimeCombo, whatsThis );
  mFreeTimeCombo->insertItem(i18n("Busy"));
  mFreeTimeCombo->insertItem(i18n("Free"));
  classLayout->addWidget(mFreeTimeCombo);
  freeTimeLabel->setBuddy( mFreeTimeCombo );
}

void KOEditorGeneralEvent::initInvitationBar(TQWidget * parent, TQBoxLayout * layout)
{
  TQBoxLayout *topLayout = new TQHBoxLayout( layout );
  mInvitationBar = new TQFrame( parent );
  mInvitationBar->setPaletteBackgroundColor( KGlobalSettings::alternateBackgroundColor() );
  topLayout->addWidget( mInvitationBar );

  TQBoxLayout *barLayout = new TQHBoxLayout( mInvitationBar );
  barLayout->setSpacing( layout->spacing() );
  TQLabel *label = new TQLabel( i18n("You have not yet definitely responded to this invitation." ), mInvitationBar );
  barLayout->addWidget( label );
  barLayout->addStretch( 1 );
  TQPushButton *button = new TQPushButton( i18n("Accept"), mInvitationBar );
  connect( button, TQT_SIGNAL(clicked()), TQT_SIGNAL(acceptInvitation()) );
  connect( button, TQT_SIGNAL(clicked()), mInvitationBar, TQT_SLOT(hide()) );
  barLayout->addWidget( button );
  button = new TQPushButton( i18n("Decline"), mInvitationBar );
  connect( button, TQT_SIGNAL(clicked()), TQT_SIGNAL(declineInvitation()) );
  connect( button, TQT_SIGNAL(clicked()), mInvitationBar, TQT_SLOT(hide()) );
  barLayout->addWidget( button );

  mInvitationBar->hide();
}

void KOEditorGeneralEvent::timeStuffDisable(bool disable)
{
  mStartTimeEdit->setEnabled( !disable );
  mEndTimeEdit->setEnabled( !disable );

  setDuration();
  emitDateTimeStr();
}

void KOEditorGeneralEvent::associateTime(bool time)
{
  timeStuffDisable(time);
  allDayChanged(time);
}

void KOEditorGeneralEvent::setDateTimes( const TQDateTime &start, const TQDateTime &end )
{
//  kdDebug(5850) << "KOEditorGeneralEvent::setDateTimes(): Start DateTime: " << start.toString() << endl;

  mStartDateEdit->setDate(start.date());
  // KTimeEdit seems to emit some signals when setTime() is called.
  mStartTimeEdit->blockSignals( true );
  mStartTimeEdit->setTime(start.time());
  mStartTimeEdit->blockSignals( false );
  mEndDateEdit->setDate(end.date());
  mEndTimeEdit->setTime(end.time());

  mCurrStartDateTime = start;
  mCurrEndDateTime = end;

  setDuration();
  emitDateTimeStr();
}

void KOEditorGeneralEvent::startTimeChanged( TQTime newtime )
{
  kdDebug(5850) << "KOEditorGeneralEvent::startTimeChanged() " << newtime.toString() << endl;

  int secsep = mCurrStartDateTime.secsTo(mCurrEndDateTime);

  mCurrStartDateTime.setTime(newtime);

  // adjust end time so that the event has the same duration as before.
  mCurrEndDateTime = mCurrStartDateTime.addSecs(secsep);
  mEndTimeEdit->setTime(mCurrEndDateTime.time());
  mEndDateEdit->setDate(mCurrEndDateTime.date());

  emit dateTimesChanged(mCurrStartDateTime,mCurrEndDateTime);
}

void KOEditorGeneralEvent::endTimeChanged( TQTime newtime )
{
//  kdDebug(5850) << "KOEditorGeneralEvent::endTimeChanged " << newtime.toString() << endl;

  TQDateTime newdt(mCurrEndDateTime.date(), newtime);
  mCurrEndDateTime = newdt;

  emit dateTimesChanged(mCurrStartDateTime,mCurrEndDateTime);
}

void KOEditorGeneralEvent::startDateChanged( const TQDate &newdate )
{
  if ( !newdate.isValid() )
    return;

  int daysep = mCurrStartDateTime.daysTo(mCurrEndDateTime);

  mCurrStartDateTime.setDate(newdate);

  // adjust end date so that the event has the same duration as before
  mCurrEndDateTime.setDate(mCurrStartDateTime.date().addDays(daysep));
  mEndDateEdit->setDate(mCurrEndDateTime.date());

  emit dateTimesChanged(mCurrStartDateTime,mCurrEndDateTime);
}

void KOEditorGeneralEvent::endDateChanged( const TQDate &newdate )
{
  if ( !newdate.isValid() )
    return;

  TQDateTime newdt(newdate, mCurrEndDateTime.time());
  mCurrEndDateTime = newdt;

  emit dateTimesChanged(mCurrStartDateTime,mCurrEndDateTime);
}

void KOEditorGeneralEvent::setDefaults( const TQDateTime &from,
                                        const TQDateTime &to, bool allDay)
{
  KOEditorGeneral::setDefaults(allDay);

  mAlldayEventCheckbox->setChecked(allDay);
  timeStuffDisable(allDay);

  setDateTimes(from,to);
}

void KOEditorGeneralEvent::readEvent( Event *event, Calendar *calendar, const TQDate &date, bool tmpl )
{
  TQString tmpStr;

  mAlldayEventCheckbox->setChecked(event->doesFloat());
  timeStuffDisable(event->doesFloat());

  if ( !tmpl ) {
    TQDateTime startDT = event->dtStart();
    TQDateTime endDT = event->dtEnd();
    if ( event->doesRecur() && date.isValid() ) {
      // Consider the active date when editing recurring Events.
      TQDateTime kdt( date, TQTime( 0, 0, 0 ) );
      const int eventLength = startDT.daysTo( endDT );
      kdt = kdt.addSecs( -1 );
      startDT.setDate( event->recurrence()->getNextDateTime( kdt ).date() );
      if ( event->hasEndDate() ) {
        endDT.setDate( startDT.addDays( eventLength ).date() );
      } else {
        if ( event->hasDuration() ) {
          endDT = startDT.addSecs( event->duration() );
        } else {
          endDT = startDT;
        }
      }
    }
    // the rest is for the events only
    setDateTimes( startDT, endDT );
  }

  switch( event->transparency() ) {
  case Event::Transparent:
    mFreeTimeCombo->setCurrentItem(1);
    break;
  case Event::Opaque:
    mFreeTimeCombo->setCurrentItem(0);
    break;
  }

  updateRecurrenceSummary( event );

  Attendee *me = event->attendeeByMails( KOPrefs::instance()->allEmails() );
  if ( event->attendeeCount() > 1 &&
       me && ( me->status() == Attendee::NeedsAction ||
       me->status() == Attendee::Tentative ||
       me->status() == Attendee::InProcess ) ) {
    mInvitationBar->show();
  } else {
    mInvitationBar->hide();
  }

  readIncidence(event, calendar);
}

void KOEditorGeneralEvent::writeEvent(Event *event)
{
//  kdDebug(5850) << "KOEditorGeneralEvent::writeEvent()" << endl;

  writeIncidence(event);

  TQDate tmpDate;
  TQTime tmpTime;
  TQDateTime tmpDT;

  // temp. until something better happens.
  TQString tmpStr;

  if (mAlldayEventCheckbox->isChecked()) {
    event->setFloats(true);
    // need to change this.
    tmpDate = mStartDateEdit->date();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtStart(tmpDT);

    tmpDate = mEndDateEdit->date();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtEnd(tmpDT);
  } else {
    event->setFloats(false);

    // set date/time end
    tmpDate = mEndDateEdit->date();
    tmpTime = mEndTimeEdit->getTime();
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtEnd(tmpDT);

    // set date/time start
    tmpDate = mStartDateEdit->date();
    tmpTime = mStartTimeEdit->getTime();
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtStart(tmpDT);
  } // check for float

  event->setTransparency(mFreeTimeCombo->currentItem() > 0
                         ? KCal::Event::Transparent
                         : KCal::Event::Opaque);

//  kdDebug(5850) << "KOEditorGeneralEvent::writeEvent() done" << endl;
}

void KOEditorGeneralEvent::setDuration()
{
  TQString tmpStr, catStr;
  int hourdiff, minutediff;
  // end<date is an accepted temporary state while typing, but don't show
  // any duration if this happens
  if(mCurrEndDateTime >= mCurrStartDateTime) {

    if (mAlldayEventCheckbox->isChecked()) {
      int daydiff = mCurrStartDateTime.date().daysTo(mCurrEndDateTime.date()) + 1;
      tmpStr = i18n("Duration: ");
      tmpStr.append(i18n("1 Day","%n Days",daydiff));
    } else {
      hourdiff = mCurrStartDateTime.date().daysTo(mCurrEndDateTime.date()) * 24;
      hourdiff += mCurrEndDateTime.time().hour() -
                  mCurrStartDateTime.time().hour();
      minutediff = mCurrEndDateTime.time().minute() -
                   mCurrStartDateTime.time().minute();
      // If minutediff is negative, "borrow" 60 minutes from hourdiff
      if (minutediff < 0 && hourdiff > 0) {
        hourdiff -= 1;
        minutediff += 60;
      }
      if (hourdiff || minutediff){
        tmpStr = i18n("Duration: ");
        if (hourdiff){
          catStr = i18n("1 hour","%n hours",hourdiff);
          tmpStr.append(catStr);
        }
        if (hourdiff && minutediff){
          tmpStr += i18n(", ");
        }
        if (minutediff){
          catStr = i18n("1 minute","%n minutes",minutediff);
          tmpStr += catStr;
        }
      } else tmpStr = "";
    }
  }
  mDurationLabel->setText(tmpStr);
  TQWhatsThis::add( mDurationLabel,
       i18n("Shows the duration of the event or to-do with the "
      "current start and end dates and times.") );
}

void KOEditorGeneralEvent::emitDateTimeStr()
{
  KLocale *l = KGlobal::locale();

  TQString from,to;
  if (mAlldayEventCheckbox->isChecked()) {
    from = l->formatDate(mCurrStartDateTime.date());
    to = l->formatDate(mCurrEndDateTime.date());
  } else {
    from = l->formatDateTime(mCurrStartDateTime);
    to = l->formatDateTime(mCurrEndDateTime);
  }

  TQString str = i18n("From: %1   To: %2   %3").arg(from).arg(to)
                .arg(mDurationLabel->text());

  emit dateTimeStrChanged(str);
}

bool KOEditorGeneralEvent::validateInput()
{
//  kdDebug(5850) << "KOEditorGeneralEvent::validateInput()" << endl;

  if (!mAlldayEventCheckbox->isChecked()) {
    if (!mStartTimeEdit->inputIsValid()) {
      KMessageBox::sorry( 0,
          i18n("Please specify a valid start time, for example '%1'.")
          .arg( KGlobal::locale()->formatTime( TQTime::currentTime() ) ) );
      return false;
    }

    if (!mEndTimeEdit->inputIsValid()) {
      KMessageBox::sorry( 0,
          i18n("Please specify a valid end time, for example '%1'.")
          .arg( KGlobal::locale()->formatTime( TQTime::currentTime() ) ) );
      return false;
    }
  }

  if (!mStartDateEdit->date().isValid()) {
    KMessageBox::sorry( 0,
        i18n("Please specify a valid start date, for example '%1'.")
        .arg( KGlobal::locale()->formatDate( TQDate::currentDate() ) ) );
    return false;
  }

  if (!mEndDateEdit->date().isValid()) {
    KMessageBox::sorry( 0,
        i18n("Please specify a valid end date, for example '%1'.")
        .arg( KGlobal::locale()->formatDate( TQDate::currentDate() ) ) );
    return false;
  }

  TQDateTime startDt,endDt;
  startDt.setDate(mStartDateEdit->date());
  endDt.setDate(mEndDateEdit->date());
  if (!mAlldayEventCheckbox->isChecked()) {
    startDt.setTime(mStartTimeEdit->getTime());
    endDt.setTime(mEndTimeEdit->getTime());
  }

  if ( startDt > endDt ) {
    KMessageBox::sorry(
      0,
      i18n( "The event ends before it starts.\n"
            "Please correct dates and times." ) );
    return false;
  }

  return KOEditorGeneral::validateInput();
}

void KOEditorGeneralEvent::updateRecurrenceSummary( Event *event )
{
  if ( event->doesRecur() ) {
    mRecEditLabel->setText( IncidenceFormatter::recurrenceString( event ) );
  } else {
    mRecEditLabel->setText( TQString() );
  }
}
