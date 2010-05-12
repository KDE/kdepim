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

#include <KDE/KActionCollection>
#include <KDE/KSystemTimeZones>
#include <KDE/KToolBar>

#include <KCal/Incidence>
#include <KCal/IncidenceFormatter>
#include <KCal/ICalTimeZones>

#include "editoralarms.h"
#include "editorconfig.h"
#include "ui_incidencegeneral.h"

using namespace KCal;
using namespace IncidenceEditors;

IncidenceGeneralEditor::IncidenceGeneralEditor( QWidget *parent )
  : QWidget( parent )
  , mTimeZones( new ICalTimeZones )
  , mUi( new Ui::IncidenceGeneral )
  , mRichTextCheck( new QCheckBox( i18nc( "@option:check", "Rich text" ), this ) )
{
  mUi->setupUi( this );
  mUi->mAlarmBell->setPixmap( SmallIcon( "task-reminder" ) );
  mUi->mRecurrenceEditButton->setIcon(
    KIconLoader::global()->loadIcon(
      "task-recurring", KIconLoader::Desktop, KIconLoader::SizeSmall ) );
  mUi->mTimeZoneComboStart->setAdditionalTimeZones( mTimeZones );
  mUi->mTimeZoneComboEnd->setAdditionalTimeZones( mTimeZones );
  mUi->mSecrecyCombo->addItems( Incidence::secrecyList() );

  mUi->mDescriptionEdit->setRichTextSupport( KRichTextWidget::SupportBold |
                                             KRichTextWidget::SupportBold |
                                             KRichTextWidget::SupportItalic |
                                             KRichTextWidget::SupportUnderline |
                                             KRichTextWidget::SupportStrikeOut |
                                             KRichTextWidget::SupportChangeListStyle |
                                             KRichTextWidget::SupportAlignment |
                                             KRichTextWidget::SupportFormatPainting );

  initDescriptionToolBar();
  
  connect( mUi->mAlarmEditButton, SIGNAL(clicked()), SLOT(editAlarms()) );
  connect( mUi->mHasTimeCheckbox, SIGNAL(toggled(bool)), SLOT(slotHasTimeCheckboxToggled(bool)) );
}

void IncidenceGeneralEditor::load( const KCal::Incidence::Ptr &incidence )
{
  mIncidence = incidence;
  mUi->mSummaryEdit->setText( incidence->summary() );
  mUi->mLocationEdit->setText( incidence->location() );
  mUi->mCategoriesLabel->setText( incidence->categories().join( "," ) );

  enableTimeEditors( !incidence->allDay() );
  
  mUi->mHasTimeCheckbox->setChecked( !incidence->allDay() );
  
  switch( incidence->secrecy() ) {
  case Incidence::SecrecyPublic:
    mUi->mSecrecyCombo->setCurrentIndex( 0 );
    break;
  case Incidence::SecrecyPrivate:
    mUi->mSecrecyCombo->setCurrentIndex( 1 );
    break;
  case Incidence::SecrecyConfidential:
    mUi->mSecrecyCombo->setCurrentIndex( 2 );
    break;
  }

  setDescription( incidence->description(), incidence->descriptionIsRich() );

  updateRecurrenceSummary( incidence );
  
  // set up alarm stuff
  mAlarmList.clear();
  Alarm::List::ConstIterator it;
  Alarm::List alarms = incidence->alarms();
  for ( it = alarms.constBegin(); it != alarms.constEnd(); ++it ) {
    Alarm *al = new Alarm( *(*it) );
    al->setParent( 0 );
    mAlarmList.append( al );
  }
  
  updateDefaultAlarmTime();
  updateAlarmWidgets();

//   mAttachments->readIncidence( incidence );
}

Alarm *IncidenceGeneralEditor::alarmFromSimplePage() const
{
  if ( mUi->mAlarmButton->isChecked() ) {
    Alarm *alarm = new Alarm( 0 );
    alarm->setDisplayAlarm( "" );
    alarm->setEnabled( true );
    QString tmpStr = mUi->mAlarmTimeEdit->text();
    int j = mUi->mAlarmTimeEdit->value() * -60;
    if ( mUi->mAlarmIncrCombo->currentIndex() == 1 ) {
      j = j * 60;
    } else if ( mUi->mAlarmIncrCombo->currentIndex() == 2 ) {
      j = j * ( 60 * 24 );
    }

    if ( setAlarmOffset( alarm, j ) ) {
      return alarm;
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}

void IncidenceGeneralEditor::enableAlarmEditor( bool enable )
{
  mUi->mAlarmStack->setEnabled( enable );
  mUi->mAlarmEditButton->setEnabled( enable );
}

void IncidenceGeneralEditor::initDescriptionToolBar()
{
  mRichTextCheck->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Select this option if you would like to enter rich text into "
           "the description field of this event or to-do." ) );
  mRichTextCheck->setToolTip( i18nc( "@info:tooltip", "Toggle Rich Text" ) );

  connect( mRichTextCheck, SIGNAL(toggled(bool)),
           this, SLOT(enableRichTextDescription(bool)) );
           
  KActionCollection *collection = new KActionCollection( this ); //krazy:exclude=tipsandthis
  mUi->mDescriptionEdit->createActions( collection );

  KToolBar *mEditToolBar = new KToolBar( mUi->mEditToolBarPlaceHolder );
  mEditToolBar->setToolButtonStyle( Qt::ToolButtonIconOnly );
  mEditToolBar->addWidget( mRichTextCheck );
  mEditToolBar->addAction( collection->action( "format_text_bold" ) );
  mEditToolBar->addAction( collection->action( "format_text_italic" ) );
  mEditToolBar->addAction( collection->action( "format_text_underline" ) );
  mEditToolBar->addAction( collection->action( "format_text_strikeout" ) );
  mEditToolBar->addSeparator();

  mEditToolBar->addAction( collection->action( "format_list_style" ) );
  mEditToolBar->addSeparator();

  mEditToolBar->addAction( collection->action( "format_align_left" ) );
  mEditToolBar->addAction( collection->action( "format_align_center" ) );
  mEditToolBar->addAction( collection->action( "format_align_right" ) );
  mEditToolBar->addAction( collection->action( "format_align_justify" ) );
  mEditToolBar->addSeparator();

  mEditToolBar->addAction( collection->action( "format_painter" ) );
  mUi->mDescriptionEdit->setActionsEnabled( mRichTextCheck->isChecked() );

  QGridLayout *layout = new QGridLayout( mUi->mEditToolBarPlaceHolder );
  layout->addWidget( mEditToolBar );
}

void IncidenceGeneralEditor::setDescription( const QString &text, bool isRich )
{
  mRichTextCheck->setChecked( isRich );
  if ( isRich )
    mUi->mDescriptionEdit->setHtml( text );
  else
    mUi->mDescriptionEdit->setPlainText( text );
}

void IncidenceGeneralEditor::slotHasTimeCheckboxToggled( bool checked )
{
  enableTimeEditors( checked );
}

void IncidenceGeneralEditor::editAlarms()
{
  if ( mUi->mAlarmStack->indexOf( mUi->mAlarmStack->currentWidget() ) == SimpleAlarmPage ) {
    mAlarmList.clear();
    Alarm *al = alarmFromSimplePage();
    if ( al ) {
      mAlarmList.append( al );
    }
  }

  QPointer<EditorAlarms> dlg = new EditorAlarms( mIncidence->type(), &mAlarmList, mUi->mAlarmEditButton );
  if ( dlg->exec() != KDialog::Cancel ) {
    updateAlarmWidgets();
  }
  delete dlg;
}

void IncidenceGeneralEditor::enableRichTextDescription( bool rich )
{
  mUi->mDescriptionEdit->setActionsEnabled( rich );
  if ( !rich ) {
    mUi->mDescriptionEdit->switchToPlainText();
  } else {
    mUi->mDescriptionEdit->enableRichTextMode();
  }
}

void IncidenceGeneralEditor::updateAlarmWidgets()
{
  if ( mAlarmList.isEmpty() ) {
    mUi->mAlarmStack->setCurrentIndex( SimpleAlarmPage );
    bool on = false;
    if ( mIncidence->type() == "Event" ) {
      on = EditorConfig::instance()->defaultEventReminders();
    } else if ( mIncidence->type() == "Todo" ) {
      on = EditorConfig::instance()->defaultTodoReminders();
    }
    mUi->mAlarmButton->setChecked( on );
  } else if ( mAlarmList.count() > 1 ) {
    mUi->mAlarmEditButton->setEnabled( true );
    mUi->mAlarmStack->setCurrentIndex( AdvancedAlarmLabel );
    mUi->mAlarmInfoLabel->setText( i18ncp( "@label",
                                           "1 reminder configured",
                                           "%1  reminders configured",
                                           mAlarmList.count() ) );
  } else {
    mUi->mAlarmEditButton->setEnabled( true );
    Alarm *alarm = mAlarmList.first();
    // Check if it is the trivial type of alarm, which can be
    // configured with a simply spin box...

    if ( alarm->type() == Alarm::Display && alarm->text().isEmpty() &&
         alarm->repeatCount() == 0 && !alarm->hasTime() &&
         alarm->hasStartOffset() && alarm->startOffset().asSeconds() < 0 ) {
      mUi->mAlarmStack->setCurrentIndex( SimpleAlarmPage );
      mUi->mAlarmButton->setChecked( true );
      int offset = alarm->startOffset().asSeconds();

      offset = offset / -60; // make minutes
      int useoffset = offset;
      if ( offset % ( 24 * 60 ) == 0 ) { // divides evenly into days?
        useoffset = offset / ( 24 * 60 );
        mUi->mAlarmIncrCombo->setCurrentIndex( 2 );
      } else if ( offset % 60 == 0 ) { // divides evenly into hours?
        useoffset = offset / 60;
        mUi->mAlarmIncrCombo->setCurrentIndex( 1 );
      } else {
        mUi->mAlarmIncrCombo->setCurrentIndex( 0 );
      }
      mUi->mAlarmTimeEdit->setValue( useoffset );
    } else {
      mUi->mAlarmStack->setCurrentIndex( AdvancedAlarmLabel );
      mUi->mAlarmInfoLabel->setText( i18nc( "@label", "1 advanced reminder configured" ) );
    }
  }
}

void IncidenceGeneralEditor::updateDefaultAlarmTime()
{
  const int reminderTime = EditorConfig::instance()->reminderTime();
  int index = EditorConfig::instance()->reminderTimeUnits();
  if ( index < 0 || index > 2 )
    index = 0;

  mUi->mAlarmTimeEdit->setValue( reminderTime );
  mUi->mAlarmIncrCombo->setCurrentIndex( index );
}

void IncidenceGeneralEditor::updateRecurrenceSummary( const KCal::Incidence::Ptr &incidence )
{
  if ( incidence->recurs() ) {
    mUi->mRecurrenceLabel->setText( IncidenceFormatter::recurrenceString( incidence.get() ) );
  } else {
    mUi->mRecurrenceLabel->setText( QString() );
  }
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

IncidenceGeneralEditor::~IncidenceGeneralEditor()
{
  delete mTimeZones;
  delete mUi;
}

/// Event Editor specifics

EventGeneralEditor::EventGeneralEditor( QWidget *parent )
  : IncidenceGeneralEditor( parent )
{
  mUi->mTodoSpecifics->setVisible( false );
  mUi->mStartCheck->setVisible( false );
  mUi->mDueCheck->setVisible( false );

  connect( mUi->mHasTimeCheckbox, SIGNAL(toggled(bool)), SLOT(slotHasTimeCheckboxToggled(bool)) );
}

void EventGeneralEditor::enableTimeEditors( bool enabled )
{
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
}

bool EventGeneralEditor::setAlarmOffset( Alarm *alarm, int value ) const
{
  alarm->setStartOffset( value );
  return true;
}

void EventGeneralEditor::slotHasTimeCheckboxToggled( bool checked )
{
  IncidenceGeneralEditor::slotHasTimeCheckboxToggled( checked );

  setDuration();
  emitDateTimeStr();
  
  //if(alarmButton->isChecked()) alarmStuffDisable(noTime);
  emit allDayChanged( !checked );
}

/// Todo Editor specifics

TodoGeneralEditor::TodoGeneralEditor( QWidget *parent )
  : IncidenceGeneralEditor( parent )
{
  mUi->mStartLabel->setVisible( false );
  mUi->mEndLabel->setVisible( false );

  connect( mUi->mStartCheck, SIGNAL(toggled(bool)), SLOT(enableStartEdit(bool)) );
//   connect( mUi->mStartCheck, SIGNAL(toggled(bool)), SLOT(startDateModified()) );
  connect( mUi->mDueCheck, SIGNAL(toggled(bool)), SLOT(enableEndEdit(bool)) );
//   connect( mUi->mDueCheck, SIGNAL(toggled(bool)), SLOT(showAlarm()) );
//   connect( mUi->mDueCheck, SIGNAL(toggled(bool)), SIGNAL(dueDateEditToggle(bool)) );
//   connect( mUi->mDueCheck, SIGNAL(toggled(bool)), SLOT(dateChanged()) );
  connect( mUi->mCompletedCombo, SIGNAL(activated(int)), SLOT(completedChanged(int)) );
}

void TodoGeneralEditor::load( const KCal::Todo::Ptr &todo,
                              const QDate &date,
                              bool tmpl )
{
  IncidenceGeneralEditor::load( todo );

  KDateTime dueDT = todo->dtDue();

  if ( todo->hasStartDate() ) {
    KDateTime startDT = todo->dtStart();
    if ( todo->recurs() && date.isValid() && todo->hasDueDate() ) {
      int days = todo->dtStart( true ).daysTo( todo->dtDue( true ) );
      startDT.setDate( dueDT.date().addDays( -days ) );
    }

    if ( startDT.isUtc() )
      startDT = startDT.toLocalZone();
    
    mUi->mStartDateEdit->setDate( startDT.date() );
    mUi->mStartTimeEdit->setTime( startDT.time() );
    mUi->mStartCheck->setChecked( true );
    mStartSpec = todo->dtStart().timeSpec();
    mUi->mTimeZoneComboStart->selectTimeSpec( todo->dtStart().timeSpec() );
  } else {
    mUi->mStartDateEdit->setEnabled( false );
    mUi->mStartTimeEdit->setEnabled( false );
    mUi->mStartDateEdit->setDate( QDate::currentDate() );
    mUi->mStartTimeEdit->setTime( QTime::currentTime() );
    mUi->mStartCheck->setChecked( false );
    mUi->mTimeZoneComboStart->setEnabled( false );
  }

  if ( todo->hasDueDate() ) {
    enableAlarmEditor( true );
    if ( todo->recurs() && date.isValid() ) {
      KDateTime dt( date, QTime( 0, 0, 0 ) );
      dt = dt.addSecs( -1 );
      dueDT.setDate( todo->recurrence()->getNextDateTime( dt ).date() );
    }
    if ( dueDT.isUtc() ) {
      dueDT = dueDT.toLocalZone();
    }
    mUi->mEndDateEdit->setDate( dueDT.date() );
    mUi->mEndTimeEdit->setTime( dueDT.time() );
    mUi->mDueCheck->setChecked( true );
    mEndSpec = todo->dtDue().timeSpec();
    mUi->mTimeZoneComboEnd->selectTimeSpec( todo->dtDue().timeSpec() );
  } else {
    enableAlarmEditor( false );
    mUi->mEndDateEdit->setEnabled( false );
    mUi->mEndTimeEdit->setEnabled( false );
    mUi->mEndDateEdit->setDate( QDate::currentDate() );
    mUi->mEndTimeEdit->setTime( QTime::currentTime() );
    mUi->mDueCheck->setChecked( false );
    mUi->mTimeZoneComboEnd->setEnabled( false );
  }

  mAlreadyComplete = false;
  mUi->mCompletedCombo->setCurrentIndex( todo->percentComplete() / 10 );
  if ( todo->isCompleted() && todo->hasCompletedDate() ) {
    mCompleted = todo->completed().toTimeSpec( KSystemTimeZones::local() ).dateTime();
    mAlreadyComplete = true;
  }
  setCompletedDate();

  mUi->mPriorityCombo->setCurrentIndex( todo->priority() );
//   mStartDateModified = false;
}

void TodoGeneralEditor::completedChanged( int index )
{
  if ( index == 10 ) {
    mCompleted = QDateTime::currentDateTime();
  }
  setCompletedDate();
}

void TodoGeneralEditor::enableTimeEditors( bool enabled )
{
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
}

void TodoGeneralEditor::enableTimeEdit( QWidget *dateEdit,
                                        QWidget *timeEdit,
                                        KPIM::KTimeZoneComboBox *timeZoneCmb,
                                        bool enable )
{
  dateEdit->setEnabled( enable );

  updateHasTimeCheckBox();

  if ( enable ) {
    timeEdit->setEnabled( mUi->mHasTimeCheckbox->isChecked() );
    timeZoneCmb->setEnabled( mUi->mHasTimeCheckbox->isChecked() );
  } else {
    timeEdit->setEnabled( false );
    timeZoneCmb->setEnabled( false );
  }

  timeZoneCmb->setFloating( !timeZoneCmb->isEnabled() );
}

void TodoGeneralEditor::enableStartEdit( bool enable )
{
  enableTimeEdit( mUi->mStartDateEdit,
                  mUi->mStartTimeEdit,
                  mUi->mTimeZoneComboStart,
                  enable );
}

void TodoGeneralEditor::enableEndEdit( bool enable )
{
  enableTimeEdit( mUi->mEndDateEdit,
                  mUi->mEndTimeEdit,
                  mUi->mTimeZoneComboEnd,
                  enable );
}

void TodoGeneralEditor::setCompletedDate()
{
  if ( mUi->mCompletedCombo->currentIndex() == 10 && mCompleted.isValid() ) {
    mUi->mCompletedLabel->setText( i18nc( "to-do completed on datetime", "co&mpleted on" ) );
//        .arg(KGlobal::locale()->formatDateTime(mCompleted)));
    mUi->mCompletionDateEdit->show();
    mUi->mCompletionTimeEdit->show();
    mUi->mCompletionDateEdit->setDate( mCompleted.date() );
    mUi->mCompletionTimeEdit->setTime( mCompleted.time() );
  } else {
    mUi->mCompletedLabel->setText( i18nc( "to-do completed", "co&mpleted" ) );
    mUi->mCompletionDateEdit->hide();
    mUi->mCompletionTimeEdit->hide();
  }
}

void TodoGeneralEditor::updateHasTimeCheckBox()
{
  if( mUi->mDueCheck->isChecked() || mUi->mStartCheck->isChecked() ) {
    mUi->mHasTimeCheckbox->setEnabled( true );
  } else {
    mUi->mHasTimeCheckbox->setEnabled( false );
    mUi->mHasTimeCheckbox->setChecked( false );
  }
}

bool TodoGeneralEditor::setAlarmOffset( Alarm *alarm, int value ) const
{
  if ( mUi->mEndDateEdit->isEnabled() ) {
    alarm->setEndOffset( value );
    return true;
  } else if ( mUi->mStartDateEdit->isEnabled() ) {
    alarm->setStartOffset( value );
    return true;
  } else {
    // Can't have alarms
    return false;
  }
}
