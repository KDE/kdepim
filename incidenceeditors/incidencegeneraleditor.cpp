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

#include <KActionCollection>
#include <KToolBar>

#include <KCal/Incidence>
#include <KCal/ICalTimeZones>

#include "ui_incidencegeneral.h"

using namespace KCal;

IncidenceGeneralEditor::IncidenceGeneralEditor( QWidget *parent )
  : QWidget( parent )
  , mTimeZones( new ICalTimeZones )
  , mUi( new Ui::IncidenceGeneral )
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
  connect( mUi->mHasTimeCheckbox, SIGNAL(toggled(bool)), SLOT(slotHasTimeCheckboxToggled(bool)) );
}

void IncidenceGeneralEditor::initDescriptionToolBar()
{

  QCheckBox *richTextCheck = new QCheckBox( i18nc( "@option:check", "Rich text" ), this );
  richTextCheck->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Select this option if you would like to enter rich text into "
           "the description field of this event or to-do." ) );
  richTextCheck->setToolTip( i18nc( "@info:tooltip", "Toggle Rich Text" ) );

  connect( richTextCheck, SIGNAL(toggled(bool)),
           this, SLOT(enableRichTextDescription(bool)) );
           
  KActionCollection *collection = new KActionCollection( this ); //krazy:exclude=tipsandthis
  mUi->mDescriptionEdit->createActions( collection );

  KToolBar *mEditToolBar = new KToolBar( mUi->mEditToolBarPlaceHolder );
  mEditToolBar->setToolButtonStyle( Qt::ToolButtonIconOnly );
  mEditToolBar->addWidget( richTextCheck );
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
  mUi->mDescriptionEdit->setActionsEnabled( richTextCheck->isChecked() );

  QGridLayout *layout = new QGridLayout( mUi->mEditToolBarPlaceHolder );
  layout->addWidget( mEditToolBar );
}

void IncidenceGeneralEditor::slotHasTimeCheckboxToggled( bool checked )
{
  enableTimeEditors( checked );
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

void TodoGeneralEditor::updateHasTimeCheckBox()
{
  if( mUi->mDueCheck->isChecked() || mUi->mStartCheck->isChecked() ) {
    mUi->mHasTimeCheckbox->setEnabled( true );
  } else {
    mUi->mHasTimeCheckbox->setEnabled( false );
    mUi->mHasTimeCheckbox->setChecked( false );
  }
}

