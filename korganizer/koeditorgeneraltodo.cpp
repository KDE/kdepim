/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
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
#include <tqfiledialog.h>
#include <tqlayout.h>
#include <tqvbox.h>
#include <tqbuttongroup.h>
#include <tqvgroupbox.h>
#include <tqwidgetstack.h>
#include <tqdatetime.h>
#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqspinbox.h>
#include <tqpushbutton.h>
#include <tqwhatsthis.h>

#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <ktextedit.h>

#include <libkcal/incidenceformatter.h>
#include <libkcal/todo.h>

#include <libkdepim/kdateedit.h>

#include "koprefs.h"
#include "koglobals.h"
#include "ktimeedit.h"

#include "koeditorgeneraltodo.h"
#include "koeditorgeneraltodo.moc"

KOEditorGeneralTodo::KOEditorGeneralTodo( TQObject *parent, const char *name )
  : KOEditorGeneral( parent, name )
{
  setType( "Todo" );
}

KOEditorGeneralTodo::~KOEditorGeneralTodo()
{
}

void KOEditorGeneralTodo::finishSetup()
{
  TQWidget::setTabOrder( mSummaryEdit, mLocationEdit );
  TQWidget::setTabOrder( mLocationEdit, mStartCheck );
  TQWidget::setTabOrder( mStartCheck, mStartDateEdit );
  TQWidget::setTabOrder( mStartDateEdit, mStartTimeEdit );
  TQWidget::setTabOrder( mStartTimeEdit, mDueCheck );
  TQWidget::setTabOrder( mDueCheck, mDueDateEdit );
  TQWidget::setTabOrder( mDueDateEdit, mDueTimeEdit );
  TQWidget::setTabOrder( mDueTimeEdit, mTimeButton );
  TQWidget::setTabOrder( mTimeButton, mRecEditButton );
  TQWidget::setTabOrder( mRecEditButton, mCompletedToggle );
  TQWidget::setTabOrder( mCompletedToggle, mCompletedCombo );
  TQWidget::setTabOrder( mCompletedCombo, mPriorityCombo );
  TQWidget::setTabOrder( mPriorityCombo, mAlarmButton );
  TQWidget::setTabOrder( mAlarmButton, mAlarmTimeEdit );
  TQWidget::setTabOrder( mAlarmTimeEdit, mAlarmIncrCombo );
  TQWidget::setTabOrder( mAlarmIncrCombo, mAlarmAdvancedButton );
  TQWidget::setTabOrder( mAlarmAdvancedButton, mDescriptionEdit );
  TQWidget::setTabOrder( mDescriptionEdit, mCategoriesButton );
  TQWidget::setTabOrder( mCategoriesButton, mSecrecyCombo );

  mSummaryEdit->setFocus();
}

void KOEditorGeneralTodo::initTime(TQWidget *parent,TQBoxLayout *topLayout)
{
  kdDebug(5850) << k_funcinfo << endl;
  TQBoxLayout *timeLayout = new TQVBoxLayout(topLayout);

  TQGroupBox *timeGroupBox = new TQGroupBox(1,TQGroupBox::Horizontal,
                                          i18n("Date && Time"),parent);
  timeLayout->addWidget(timeGroupBox);

  TQFrame *timeBoxFrame = new TQFrame(timeGroupBox);
  TQWhatsThis::add( timeBoxFrame,
                   i18n("Sets options for due and start dates and times "
                        "for this to-do.") );

  TQGridLayout *layoutTimeBox = new TQGridLayout(timeBoxFrame,1,1);
  layoutTimeBox->setSpacing(topLayout->spacing());


  TQString whatsThis = i18n("Sets the start date for this to-do");
  mStartCheck = new TQCheckBox(i18n("Sta&rt:"),timeBoxFrame);
  TQWhatsThis::add( mStartCheck, whatsThis );
  layoutTimeBox->addWidget(mStartCheck,0,0);
  connect(mStartCheck,TQT_SIGNAL(toggled(bool)),TQT_SLOT(enableStartEdit(bool)));
  connect(mStartCheck,TQT_SIGNAL(toggled(bool)),TQT_SLOT(startDateModified()));

  mStartDateEdit = new KDateEdit(timeBoxFrame);
  TQWhatsThis::add( mStartDateEdit, whatsThis );
  layoutTimeBox->addWidget(mStartDateEdit,0,1);
  connect(mStartDateEdit,TQT_SIGNAL(dateChanged(const TQDate&)),TQT_SLOT(startDateModified()));

  mStartTimeEdit = new KTimeEdit(timeBoxFrame);
  TQWhatsThis::add( mStartTimeEdit,
                   i18n("Sets the start time for this to-do.") );
  layoutTimeBox->addWidget(mStartTimeEdit,0,2);
  connect(mStartTimeEdit,TQT_SIGNAL(timeChanged(TQTime)),TQT_SLOT(startDateModified()));

  whatsThis = i18n("Sets the due date for this to-do.");
  mDueCheck = new TQCheckBox(i18n("&Due:"),timeBoxFrame);
  TQWhatsThis::add( mDueCheck, whatsThis );
  layoutTimeBox->addWidget(mDueCheck,1,0);
  connect(mDueCheck,TQT_SIGNAL(toggled(bool)),TQT_SLOT(enableDueEdit(bool)));
  connect(mDueCheck,TQT_SIGNAL(toggled(bool)),TQT_SIGNAL(dueDateEditToggle(bool)));
  connect(mDueCheck,TQT_SIGNAL(toggled(bool)),TQT_SLOT(dateChanged()));

  mDueDateEdit = new KDateEdit(timeBoxFrame);
  TQWhatsThis::add( mDueDateEdit, whatsThis );
  layoutTimeBox->addWidget(mDueDateEdit,1,1);
  connect(mDueDateEdit,TQT_SIGNAL(dateChanged(const TQDate&)),TQT_SLOT(dateChanged()));

  mDueTimeEdit = new KTimeEdit(timeBoxFrame);
  TQWhatsThis::add( mDueTimeEdit,
                   i18n("Sets the due time for this to-do.") );
  layoutTimeBox->addWidget(mDueTimeEdit,1,2);
  connect(mDueTimeEdit,TQT_SIGNAL(timeChanged( TQTime )),TQT_SLOT(dateChanged()));

  mTimeButton = new TQCheckBox(i18n("Ti&me associated"),timeBoxFrame);
  TQWhatsThis::add( mTimeButton,
                   i18n("Sets whether or not this to-do's start and due dates "
                        "have times associated with them.") );
  layoutTimeBox->addWidget( mTimeButton, 0, 3 );
  connect(mTimeButton,TQT_SIGNAL(toggled(bool)),TQT_SLOT(enableTimeEdits(bool)));
  connect(mTimeButton,TQT_SIGNAL(toggled(bool)),TQT_SLOT(dateChanged()));

  TQLabel *label = new TQLabel( i18n( "Recurrence:" ), timeBoxFrame );
  layoutTimeBox->addWidget( label, 3, 0 );
  TQBoxLayout *recLayout = new TQHBoxLayout();
  layoutTimeBox->addMultiCellLayout( recLayout, 3, 3, 1, 4 );
  mRecEditButton = new TQPushButton( timeBoxFrame );
  mRecEditButton->setIconSet( KOGlobals::self()->smallIconSet( "recur", 16 ) );
  recLayout->addWidget( mRecEditButton );
  connect( mRecEditButton, TQT_SIGNAL(clicked()), TQT_SIGNAL(editRecurrence()) );
  mRecEditLabel = new TQLabel( TQString(), timeBoxFrame );
  recLayout->addWidget( mRecEditLabel );
  recLayout->addStretch( 1 );

  label = new TQLabel( i18n("Reminder:"), timeBoxFrame );
  layoutTimeBox->addWidget( label, 4, 0 );
  TQBoxLayout *alarmLineLayout = new TQHBoxLayout();
  layoutTimeBox->addMultiCellLayout( alarmLineLayout, 4, 4, 1, 4 );
  initAlarm( timeBoxFrame, alarmLineLayout );
  alarmLineLayout->addStretch( 1 );

  // some more layouting
  layoutTimeBox->setColStretch( 3, 1 );

  TQBoxLayout *secLayout = new TQHBoxLayout();
  layoutTimeBox->addLayout( secLayout, 0, 4 );
  initSecrecy( timeBoxFrame, secLayout );
}


void KOEditorGeneralTodo::initCompletion( TQWidget *parent, TQBoxLayout *topLayout )
{
  TQHBoxLayout *completionLayout = new TQHBoxLayout( topLayout );

  TQLabel *label = new TQLabel( i18n( "&Completed:" ), parent );
  completionLayout->addWidget( label );

  mCompletedToggle = new TQCheckBox( parent );
  TQToolTip::add( mCompletedToggle,
                 i18n( "Toggle between 0% and 100% complete" ) );
  TQWhatsThis::add( mCompletedToggle,
                   i18n( "Click this checkbox to toggle the completed percentage of the to-do "
                         "between 0% or 100%" ) );
  connect( mCompletedToggle, TQT_SIGNAL(clicked()), TQT_SLOT(completedChanged()) );
  completionLayout->addWidget( mCompletedToggle );
  label->setBuddy( mCompletedToggle );

  mCompletedCombo = new TQComboBox( parent );
  TQToolTip::add( mCompletedCombo,
                 i18n( "Select the completed percentage" ) );
  TQWhatsThis::add( mCompletedCombo,
                   i18n( "Use this combobox to set the completion percentage of the to-do." ) );
  for ( int i = 0; i <= 100; i+=10 ) {
    // xgettext:no-c-format
    TQString label = i18n( "Percent complete", "%1 %" ).arg( i );
    mCompletedCombo->insertItem( label );
  }
  connect( mCompletedCombo, TQT_SIGNAL(activated(int)), TQT_SLOT(completedChanged(int)) );
  completionLayout->addWidget( mCompletedCombo );

  mCompletedLabel = new TQLabel( i18n( "completed on", "on" ), parent );
  mCompletedLabel->hide();
  completionLayout->addWidget( mCompletedLabel );

  mCompletionDateEdit = new KDateEdit( parent );
  mCompletionDateEdit->hide();
  completionLayout->addWidget( mCompletionDateEdit );

  mCompletionTimeEdit = new KTimeEdit( parent, TQTime() );
  mCompletionTimeEdit->hide();
  completionLayout->addWidget( mCompletionTimeEdit );
}

void KOEditorGeneralTodo::initPriority(TQWidget *parent, TQBoxLayout *topLayout)
{
  TQLabel *priorityLabel = new TQLabel( i18n( "&Priority:" ), parent );
  topLayout->addWidget( priorityLabel );

  mPriorityCombo = new TQComboBox( parent );
  TQToolTip::add( mPriorityCombo,
                 i18n( "Set the priority of the to-do" ) );
  TQWhatsThis::add( mPriorityCombo,
                   i18n( "Sets the priority of this to-do on a scale from one to nine, "
                         "with one being the highest priority, five being a medium priority, "
                         "and nine being the lowest. In programs that have a different scale, "
                         "the numbers will be adjusted to match the appropriate scale." ) );
  mPriorityCombo->insertItem( i18n( "unspecified" ) );
  mPriorityCombo->insertItem( i18n( "1 (highest)" ) );
  mPriorityCombo->insertItem( i18n( "2" ) );
  mPriorityCombo->insertItem( i18n( "3" ) );
  mPriorityCombo->insertItem( i18n( "4" ) );
  mPriorityCombo->insertItem( i18n( "5 (medium)" ) );
  mPriorityCombo->insertItem( i18n( "6" ) );
  mPriorityCombo->insertItem( i18n( "7" ) );
  mPriorityCombo->insertItem( i18n( "8" ) );
  mPriorityCombo->insertItem( i18n( "9 (lowest)" ) );
  topLayout->addWidget( mPriorityCombo );
  priorityLabel->setBuddy( mPriorityCombo );
}

void KOEditorGeneralTodo::initStatus(TQWidget *parent,TQBoxLayout *topLayout)
{
  TQBoxLayout *statusLayout = new TQHBoxLayout(topLayout);

  initCompletion( parent, statusLayout );

  statusLayout->addStretch( 1 );

  initPriority( parent, statusLayout );
}

void KOEditorGeneralTodo::setDefaults( const TQDateTime &due, bool allDay )
{
  kdDebug(5850) << k_funcinfo << due <<endl;
  KOEditorGeneral::setDefaults(allDay);

  mTimeButton->setChecked( !allDay );
  mTimeButton->setEnabled( mTimeButton->isChecked() /* i.e. !allDay */ );

  enableTimeEdits( !allDay );

  mDueCheck->setChecked( due.isValid() );
  enableDueEdit( due.isValid() );

  mStartCheck->setChecked(false);
  enableStartEdit(false);

  if ( due.isValid() ) {
    mDueDateEdit->setDate( due.date() );
    mDueTimeEdit->setTime( due.time() );
  } else {
    // Make it due tomorrow.
    mDueDateEdit->setDate( TQDate::currentDate().addDays(1) );
    mDueTimeEdit->setTime( TQTime::currentTime() );
  }

  if ( !due.isValid() || (TQDateTime::currentDateTime() < due) ) {
    mStartDateEdit->setDate( TQDate::currentDate() );
    mStartTimeEdit->setTime( TQTime::currentTime() );
  } else {
    mStartDateEdit->setDate( due.date().addDays( -1 ) );
    mStartTimeEdit->setTime( due.time() );
  }
  mStartDateModified = false;

  mPriorityCombo->setCurrentItem( 5 );

  mCompletedToggle->setChecked( false );
  mCompletedCombo->setCurrentItem( 0 );
}

void KOEditorGeneralTodo::readTodo(Todo *todo, Calendar *calendar, const TQDate &date )
{
  KOEditorGeneral::readIncidence(todo, calendar);

  TQDateTime dueDT;

  if (todo->hasDueDate()) {
    dueDT = todo->dtDue();
    if ( todo->doesRecur() && date.isValid() ) {
      TQDateTime dt( date, TQTime( 0, 0, 0 ) );
      dt = dt.addSecs( -1 );
      dueDT.setDate( todo->recurrence()->getNextDateTime( dt ).date() );
    }
    mDueDateEdit->setDate(dueDT.date());
    mDueTimeEdit->setTime(dueDT.time());
    mDueCheck->setChecked(true);
  } else {
    mDueDateEdit->setEnabled(false);
    mDueTimeEdit->setEnabled(false);
    mDueDateEdit->setDate(TQDate::currentDate());
    mDueTimeEdit->setTime(TQTime::currentTime());
    mDueCheck->setChecked(false);
  }

  if (todo->hasStartDate()) {
    TQDateTime startDT = todo->dtStart();
    if ( todo->doesRecur() && date.isValid() && todo->hasDueDate() ) {
      int days = todo->dtStart( true ).daysTo( todo->dtDue( true ) );
      startDT.setDate( date.addDays( -days ) );
    }
    mStartDateEdit->setDate(startDT.date());
    mStartTimeEdit->setTime(startDT.time());
    mStartCheck->setChecked(true);
  } else {
    mStartDateEdit->setEnabled(false);
    mStartTimeEdit->setEnabled(false);
    mStartDateEdit->setDate(TQDate::currentDate());
    mStartTimeEdit->setTime(TQTime::currentTime());
    mStartCheck->setChecked(false);
  }

  mTimeButton->setChecked( !todo->doesFloat() );

  updateRecurrenceSummary( todo );

  mAlreadyComplete = false;
  mCompletedCombo->setCurrentItem( todo->percentComplete() / 10 );
  if ( todo->isCompleted() && todo->hasCompletedDate() ) {
    mCompletedDateTime = todo->completed();
    mCompletedToggle->setChecked( true );
    mAlreadyComplete = true;
  }
  setCompletedDate();

  mPriorityCombo->setCurrentItem( todo->priority() );
  mStartDateModified = false;
}

void KOEditorGeneralTodo::writeTodo(Todo *todo)
{
  KOEditorGeneral::writeIncidence(todo);

  // temp. until something better happens.
  TQString tmpStr;

  todo->setHasDueDate(mDueCheck->isChecked());
  todo->setHasStartDate(mStartCheck->isChecked());

  TQDate tmpSDate, tmpDDate;
  TQTime tmpSTime, tmpDTime;
  TQDateTime tmpStartDT, tmpDueDT;
  if ( mTimeButton->isChecked() ) {
    todo->setFloats(false);

    // set due date/time
    tmpDDate = mDueDateEdit->date();
    tmpDTime = mDueTimeEdit->getTime();
    tmpDueDT.setDate(tmpDDate);
    tmpDueDT.setTime(tmpDTime);

    // set start date/time
    if ( mStartCheck->isChecked() ) {
      tmpSDate = mStartDateEdit->date();
      tmpSTime = mStartTimeEdit->getTime();
      tmpStartDT.setDate(tmpSDate);
      tmpStartDT.setTime(tmpSTime);
    } else {
      tmpStartDT = tmpDueDT;
    }
  } else {
    todo->setFloats(true);

    // need to change this.
    tmpDDate = mDueDateEdit->date();
    tmpDTime.setHMS(0,0,0);
    tmpDueDT.setDate(tmpDDate);
    tmpDueDT.setTime(tmpDTime);

    if ( mStartCheck->isChecked() ) {
      tmpSDate = mStartDateEdit->date();
      tmpSTime.setHMS(0,0,0);
      tmpStartDT.setDate(tmpSDate);
      tmpStartDT.setTime(tmpSTime);
    } else {
      tmpStartDT = tmpDueDT;
    }
  }

  // TODO: Don't use the due date for the recurrence, but the start date (cf. rfc 2445)
  if ( todo->doesRecur() && !mStartDateModified ) {
    todo->setDtDue( tmpDueDT );
  } else {
    todo->setDtDue( tmpDueDT, true );
    todo->setDtStart( tmpStartDT );
    todo->setDtRecurrence( tmpDueDT );
  }

  todo->setPriority( mPriorityCombo->currentItem() );

  // set completion state
  todo->setPercentComplete( mCompletedCombo->currentItem() * 10 );

  if (mCompletedCombo->currentItem() == 10 && mCompletedDateTime.isValid()) {
    TQDateTime completed( mCompletionDateEdit->date(),
                         mCompletionTimeEdit->getTime() );
    int difference = mCompletedDateTime.secsTo( completed );
    if ( (difference < 60) && (difference > -60) &&
         (completed.time().minute() == mCompletedDateTime.time().minute() ) ) {
      // completion time wasn't changed substantially (only the seconds
      // truncated, but that's an effect done by KTimeEdit automatically).
      completed = mCompletedDateTime;
    }
    todo->setCompleted( completed );
  }
}

void KOEditorGeneralTodo::enableDueEdit(bool enable)
{
  mDueDateEdit->setEnabled( enable );

  if(mDueCheck->isChecked() || mStartCheck->isChecked()) {
    mTimeButton->setEnabled(true);
  } else {
    mTimeButton->setEnabled(false);
  }

  if (enable) {
    mDueTimeEdit->setEnabled( mTimeButton->isChecked() );
  } else {
    mDueTimeEdit->setEnabled( false );
  }
}

void KOEditorGeneralTodo::enableStartEdit( bool enable )
{
  mStartDateEdit->setEnabled( enable );

  if(mDueCheck->isChecked() || mStartCheck->isChecked()) {
    mTimeButton->setEnabled(true);
  }
  else {
    mTimeButton->setEnabled(false);
    mTimeButton->setChecked(false);
  }

  if (enable) {
    mStartTimeEdit->setEnabled( mTimeButton->isChecked() );
  } else {
    mStartTimeEdit->setEnabled( false );
  }
}

void KOEditorGeneralTodo::enableTimeEdits(bool enable)
{
  if(mStartCheck->isChecked()) {
    mStartTimeEdit->setEnabled( enable );
  }
  if(mDueCheck->isChecked()) {
    mDueTimeEdit->setEnabled( enable );
  }
}

bool KOEditorGeneralTodo::validateInput()
{
  if (mDueCheck->isChecked()) {
    if (!mDueDateEdit->date().isValid()) {
      KMessageBox::sorry(0,i18n("Please specify a valid due date."));
      return false;
    }
    if (mTimeButton->isChecked()) {
      if (!mDueTimeEdit->inputIsValid()) {
        KMessageBox::sorry(0,i18n("Please specify a valid due time."));
        return false;
      }
    }
  }

  if (mStartCheck->isChecked()) {
    if (!mStartDateEdit->date().isValid()) {
      KMessageBox::sorry(0,i18n("Please specify a valid start date."));
      return false;
    }
    if (mTimeButton->isChecked()) {
      if (!mStartTimeEdit->inputIsValid()) {
        KMessageBox::sorry(0,i18n("Please specify a valid start time."));
        return false;
      }
    }
  }

  if (mStartCheck->isChecked() && mDueCheck->isChecked()) {
    TQDateTime startDate;
    TQDateTime dueDate;
    startDate.setDate(mStartDateEdit->date());
    dueDate.setDate(mDueDateEdit->date());
    if (mTimeButton->isChecked()) {
      startDate.setTime(mStartTimeEdit->getTime());
      dueDate.setTime(mDueTimeEdit->getTime());
    }
    if (startDate > dueDate) {
      KMessageBox::sorry(0,
                         i18n("The start date cannot be after the due date."));
      return false;
    }
  }

  return KOEditorGeneral::validateInput();
}

void KOEditorGeneralTodo::updateRecurrenceSummary( Todo *todo )
{
  if ( todo->doesRecur() ) {
    mRecEditLabel->setText( IncidenceFormatter::recurrenceString( todo ) );
  } else {
    mRecEditLabel->setText( TQString() );
  }
}

void KOEditorGeneralTodo::completedChanged( int index )
{
  if ( index == 10 ) {
    mCompletedToggle->setChecked( true );
    mCompletedDateTime = TQDateTime::currentDateTime();
  } else {
    mCompletedToggle->setChecked( false );
  }
  setCompletedDate();
}

void KOEditorGeneralTodo::completedChanged()
{
  if ( mCompletedToggle->isChecked() ) {
    mCompletedCombo->setCurrentItem( 10 );
    mCompletedDateTime = TQDateTime::currentDateTime();
  } else {
    mCompletedCombo->setCurrentItem( 0 );
  }
  setCompletedDate();
}

void KOEditorGeneralTodo::dateChanged()
{
  KLocale *l = KGlobal::locale();
  TQString dateTimeStr = "";

  if ( mStartCheck->isChecked() ) {
    dateTimeStr += i18n("Start: %1").arg(
                                     l->formatDate( mStartDateEdit->date() ) );
    if ( mTimeButton->isChecked() )
      dateTimeStr += TQString(" %1").arg(
                                   l->formatTime( mStartTimeEdit->getTime() ) );
  }

  if ( mDueCheck->isChecked() ) {
    dateTimeStr += i18n("   Due: %1").arg(
                                      l->formatDate( mDueDateEdit->date() ) );
    if ( mTimeButton->isChecked() )
      dateTimeStr += TQString(" %1").arg(
                                    l->formatTime( mDueTimeEdit->getTime() ) );
  }

  emit dateTimeStrChanged( dateTimeStr );
  TQDateTime endDt( mDueDateEdit->date(), mDueTimeEdit->getTime() );
  emit signalDateTimeChanged( endDt, endDt );
}

void KOEditorGeneralTodo::startDateModified()
{
  mStartDateModified = true;
  dateChanged();
}

void KOEditorGeneralTodo::setCompletedDate()
{
  if ( mCompletedCombo->currentItem() == 10 && mCompletedDateTime.isValid() ) {
    mCompletedLabel->show();
    mCompletionDateEdit->show();
    mCompletionTimeEdit->show();
    mCompletionDateEdit->setDate( mCompletedDateTime.date() );
    mCompletionTimeEdit->setTime( mCompletedDateTime.time() );
  } else {
    mCompletedLabel->hide();
    mCompletionDateEdit->hide();
    mCompletionTimeEdit->hide();
  }
}

void KOEditorGeneralTodo::modified (Todo* todo, KOGlobals::HowChanged modification)
{
  switch (modification) {
  case KOGlobals::PRIORITY_MODIFIED:
    mPriorityCombo->setCurrentItem( todo->priority() );
    break;
  case KOGlobals::COMPLETION_MODIFIED:
    mCompletedCombo->setCurrentItem(todo->percentComplete() / 10);
    if (todo->isCompleted() && todo->hasCompletedDate()) {
      mCompletedDateTime = todo->completed();
      mCompletedToggle->setChecked( true );
    }
    setCompletedDate();
    break;
  case KOGlobals::CATEGORY_MODIFIED:
    setCategories( todo->categories() );
    break;
  case KOGlobals::UNKNOWN_MODIFIED: // fall through
  default:
    readTodo( todo, 0, TQDate() );
    break;
  }
}
