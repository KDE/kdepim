/*
    This file is part of KOrganizer.
    Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>
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
#include <tqlistbox.h>
#include <tqspinbox.h>
#include <tqcheckbox.h>
#include <tqgroupbox.h>
#include <tqwidgetstack.h>
#include <tqradiobutton.h>
#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqwhatsthis.h>

#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <knumvalidator.h>
#include <kcalendarsystem.h>
#include <kmessagebox.h>

#include <libkdepim/kdateedit.h>
#include <libkcal/todo.h>

#include "koprefs.h"
#include "koglobals.h"

#include "koeditorrecurrence.h"
#include "koeditorrecurrence.moc"

/////////////////////////// RecurBase ///////////////////////////////

RecurBase::RecurBase( TQWidget *parent, const char *name ) :
  TQWidget( parent, name )
{
  mFrequencyEdit = new TQSpinBox( 1, 9999, 1, this );
  mFrequencyEdit->setValue( 1 );
}

TQWidget *RecurBase::frequencyEdit()
{
  return mFrequencyEdit;
}

void RecurBase::setFrequency( int f )
{
  if ( f < 1 ) f = 1;

  mFrequencyEdit->setValue( f );
}

int RecurBase::frequency()
{
  return mFrequencyEdit->value();
}

TQComboBox *RecurBase::createWeekCountCombo( TQWidget *parent, const char *name )
{
  TQComboBox *combo = new TQComboBox( parent, name );
  TQWhatsThis::add( combo,
                   i18n("The number of the week from the beginning "
                        "of the month on which this event or to-do "
                        "should recur.") );
  if ( !combo ) return 0;
  combo->insertItem( i18n("1st") );
  combo->insertItem( i18n("2nd") );
  combo->insertItem( i18n("3rd") );
  combo->insertItem( i18n("4th") );
  combo->insertItem( i18n("5th") );
  combo->insertItem( i18n("Last") );
  combo->insertItem( i18n("2nd Last") );
  combo->insertItem( i18n("3rd Last") );
  combo->insertItem( i18n("4th Last") );
  combo->insertItem( i18n("5th Last") );
  return combo;
}

TQComboBox *RecurBase::createWeekdayCombo( TQWidget *parent, const char *name )
{
  TQComboBox *combo = new TQComboBox( parent, name );
  TQWhatsThis::add( combo,
                   i18n("The weekday on which this event or to-do "
                        "should recur.") );
  if ( !combo ) return 0;
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
  for( int i = 1; i <= 7; ++i ) {
    combo->insertItem( calSys->weekDayName( i ) );
  }
  return combo;
}

TQComboBox *RecurBase::createMonthNameCombo( TQWidget *parent, const char *name )
{
  TQComboBox *combo = new TQComboBox( parent, name );
  TQWhatsThis::add( combo,
                   i18n("The month during which this event or to-do "
                        "should recur.") );
  if ( !combo ) return 0;
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
  for( int i = 1; i <= 12; ++i ) {
    // use an arbitrary year, we just need the month name...
    TQDate dt( 2005, i, 1 );
    combo->insertItem( calSys->monthName( dt ) );
  }
  return combo;
}

TQBoxLayout *RecurBase::createFrequencySpinBar( TQWidget *parent, TQLayout *layout,
    TQString everyText, TQString unitText )
{
  TQBoxLayout *freqLayout = new TQHBoxLayout( layout );

  TQString whatsThis = i18n("Sets how often this event or to-do should recur.");
  TQLabel *preLabel = new TQLabel( everyText, parent );
  TQWhatsThis::add( preLabel, whatsThis );
  freqLayout->addWidget( preLabel );

  freqLayout->addWidget( frequencyEdit() );
  preLabel->setBuddy( frequencyEdit() );
  TQWhatsThis::add( preLabel->buddy(), whatsThis );

  TQLabel *postLabel = new TQLabel( unitText, parent );
  TQWhatsThis::add( postLabel, whatsThis );
  freqLayout->addWidget( postLabel );
  freqLayout->addStretch();
  return freqLayout;
}

/////////////////////////// RecurDaily ///////////////////////////////

RecurDaily::RecurDaily( TQWidget *parent, const char *name ) :
  RecurBase( parent, name )
{
  TQBoxLayout *topLayout = new TQVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  createFrequencySpinBar( this, topLayout, i18n("&Recur every"), i18n("day(s)") );
}


/////////////////////////// RecurWeekly ///////////////////////////////

RecurWeekly::RecurWeekly( TQWidget *parent, const char *name ) :
  RecurBase( parent, name )
{
  TQBoxLayout *topLayout = new TQVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

//  topLayout->addStretch( 1 );

  createFrequencySpinBar( this, topLayout, i18n("&Recur every"), i18n("week(s) on:") );

  TQHBox *dayBox = new TQHBox( this );
  topLayout->addWidget( dayBox, 1, AlignVCenter );
  // Respect start of week setting
  int weekStart=KGlobal::locale()->weekStartDay();
  for ( int i = 0; i < 7; ++i ) {
    // i is the nr of the combobox, not the day of week!
    // label=(i+weekStart+6)%7 + 1;
    // index in CheckBox array(=day): label-1
    const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
    TQString weekDayName = calSys->weekDayName(
      (i + weekStart + 6)%7 + 1, true );
    if ( KOPrefs::instance()->mCompactDialogs ) {
      weekDayName = weekDayName.left( 1 );
    }
    mDayBoxes[ (i + weekStart + 6)%7 ] = new TQCheckBox( weekDayName, dayBox );
    TQWhatsThis::add( mDayBoxes[ (i + weekStart + 6)%7 ],
                     i18n("Day of the week on which this event or to-do "
                          "should recur.") );
  }

  topLayout->addStretch( 1 );
}

void RecurWeekly::setDays( const TQBitArray &days )
{
  for ( int i = 0; i < 7; ++i ) {
    mDayBoxes[ i ]->setChecked( days.testBit( i ) );
  }
}

TQBitArray RecurWeekly::days()
{
  TQBitArray days( 7 );

  for ( int i = 0; i < 7; ++i ) {
    days.setBit( i, mDayBoxes[ i ]->isChecked() );
  }

  return days;
}

/////////////////////////// RecurMonthly ///////////////////////////////

RecurMonthly::RecurMonthly( TQWidget *parent, const char *name ) :
  RecurBase( parent, name )
{
  TQBoxLayout *topLayout = new TQVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  createFrequencySpinBar( this, topLayout, i18n("&Recur every"), i18n("month(s)") );

  TQButtonGroup *buttonGroup = new TQButtonGroup( this );
  buttonGroup->setFrameStyle( TQFrame::NoFrame );
  topLayout->addWidget( buttonGroup, 1, AlignVCenter );

  TQGridLayout *buttonLayout = new TQGridLayout( buttonGroup, 3, 2 );
  buttonLayout->setSpacing( KDialog::spacingHint() );


  TQString recurOnText;
  if ( !KOPrefs::instance()->mCompactDialogs ) {
    recurOnText = i18n("&Recur on the");
  }

  mByDayRadio = new TQRadioButton( recurOnText, buttonGroup );
  TQWhatsThis::add( mByDayRadio,
                   i18n("Sets a specific day of the month on which "
                        "this event or to-do should recur.") );

  buttonLayout->addWidget( mByDayRadio, 0, 0 );

  TQString whatsThis = i18n("The day of the month on which this event or to-do "
                           "should recur.");
  mByDayCombo = new TQComboBox( buttonGroup );
  TQWhatsThis::add( mByDayCombo, whatsThis );
  mByDayCombo->setSizeLimit( 7 );
  mByDayCombo->insertItem( i18n("1st") );
  mByDayCombo->insertItem( i18n("2nd") );
  mByDayCombo->insertItem( i18n("3rd") );
  mByDayCombo->insertItem( i18n("4th") );
  mByDayCombo->insertItem( i18n("5th") );
  mByDayCombo->insertItem( i18n("6th") );
  mByDayCombo->insertItem( i18n("7th") );
  mByDayCombo->insertItem( i18n("8th") );
  mByDayCombo->insertItem( i18n("9th") );
  mByDayCombo->insertItem( i18n("10th") );
  mByDayCombo->insertItem( i18n("11th") );
  mByDayCombo->insertItem( i18n("12th") );
  mByDayCombo->insertItem( i18n("13th") );
  mByDayCombo->insertItem( i18n("14th") );
  mByDayCombo->insertItem( i18n("15th") );
  mByDayCombo->insertItem( i18n("16th") );
  mByDayCombo->insertItem( i18n("17th") );
  mByDayCombo->insertItem( i18n("18th") );
  mByDayCombo->insertItem( i18n("19th") );
  mByDayCombo->insertItem( i18n("20th") );
  mByDayCombo->insertItem( i18n("21st") );
  mByDayCombo->insertItem( i18n("22nd") );
  mByDayCombo->insertItem( i18n("23rd") );
  mByDayCombo->insertItem( i18n("24th") );
  mByDayCombo->insertItem( i18n("25th") );
  mByDayCombo->insertItem( i18n("26th") );
  mByDayCombo->insertItem( i18n("27th") );
  mByDayCombo->insertItem( i18n("28th") );
  mByDayCombo->insertItem( i18n("29th") );
  mByDayCombo->insertItem( i18n("30th") );
  mByDayCombo->insertItem( i18n("31st") );
  mByDayCombo->insertItem( i18n("Last") );
  mByDayCombo->insertItem( i18n("2nd Last") );
  mByDayCombo->insertItem( i18n("3rd Last") );
  mByDayCombo->insertItem( i18n("4th Last") );
  mByDayCombo->insertItem( i18n("5th Last") );
  // FIXME: After the string freeze is over, insert all possible values, not
  //        just the ones we already have translated:
/*  mByDayCombo->insertItem( i18n("6th Last") );
  mByDayCombo->insertItem( i18n("7th Last") );
  mByDayCombo->insertItem( i18n("8th Last") );
  mByDayCombo->insertItem( i18n("9th Last") );
  mByDayCombo->insertItem( i18n("10th Last") );
  mByDayCombo->insertItem( i18n("11th Last") );
  mByDayCombo->insertItem( i18n("12th Last") );
  mByDayCombo->insertItem( i18n("13th Last") );
  mByDayCombo->insertItem( i18n("14th Last") );
  mByDayCombo->insertItem( i18n("15th Last") );
  mByDayCombo->insertItem( i18n("16th Last") );
  mByDayCombo->insertItem( i18n("17th Last") );
  mByDayCombo->insertItem( i18n("18th Last") );
  mByDayCombo->insertItem( i18n("19th Last") );
  mByDayCombo->insertItem( i18n("20th Last") );
  mByDayCombo->insertItem( i18n("21st Last") );
  mByDayCombo->insertItem( i18n("22nd Last") );
  mByDayCombo->insertItem( i18n("23rd Last") );
  mByDayCombo->insertItem( i18n("24th Last") );
  mByDayCombo->insertItem( i18n("25th Last") );
  mByDayCombo->insertItem( i18n("26th Last") );
  mByDayCombo->insertItem( i18n("27th Last") );
  mByDayCombo->insertItem( i18n("28th Last") );
  mByDayCombo->insertItem( i18n("29th Last") );
  mByDayCombo->insertItem( i18n("30th Last") );
  mByDayCombo->insertItem( i18n("31st Last") );*/
  buttonLayout->addWidget( mByDayCombo, 0, 1 );

  TQLabel *byDayLabel = new TQLabel( i18n("day"), buttonGroup );
  TQWhatsThis::add( byDayLabel, whatsThis );
  buttonLayout->addWidget( byDayLabel, 0, 2 );


  mByPosRadio = new TQRadioButton( recurOnText, buttonGroup);
  TQWhatsThis::add( mByPosRadio,
                   i18n("Sets a weekday and specific week in the month "
                        "on which this event or to-do should recur") );
  buttonLayout->addWidget( mByPosRadio, 1, 0 );

  mByPosCountCombo = createWeekCountCombo( buttonGroup );
  buttonLayout->addWidget( mByPosCountCombo, 1, 1 );

  mByPosWeekdayCombo = createWeekdayCombo( buttonGroup );
  buttonLayout->addWidget( mByPosWeekdayCombo, 1, 2 );
}

void RecurMonthly::setByDay( int day )
{
  mByDayRadio->setChecked( true );
  // Days from the end are after the ones from the begin, so correct for the
  // negative sign and add 30 (index starting at 0)
  if ( day > 0 && day <= 31 )
    mByDayCombo->setCurrentItem( day-1 );
  else if ( day < 0 )
    mByDayCombo->setCurrentItem( 31 - 1 - day );
}

void RecurMonthly::setByPos( int count, int weekday )
{
  mByPosRadio->setChecked( true );
  if (count>0)
    mByPosCountCombo->setCurrentItem( count - 1 );
  else
    // negative weeks means counted from the end of month
    mByPosCountCombo->setCurrentItem( -count + 4 );
  mByPosWeekdayCombo->setCurrentItem( weekday - 1 );
}

bool RecurMonthly::byDay()
{
  return mByDayRadio->isChecked();
}

bool RecurMonthly::byPos()
{
  return mByPosRadio->isChecked();
}

int RecurMonthly::day()
{
  int day = mByDayCombo->currentItem();
  if ( day >= 31 ) day = 31-day-1;
  else ++day;
  return day;
}

int RecurMonthly::count()
{
  int pos=mByPosCountCombo->currentItem();
  if (pos<=4) // positive  count
    return pos+1;
  else
    return -pos+4;
}

int RecurMonthly::weekday()
{
  return mByPosWeekdayCombo->currentItem() + 1;
}

/////////////////////////// RecurYearly ///////////////////////////////

RecurYearly::RecurYearly( TQWidget *parent, const char *name ) :
  RecurBase( parent, name )
{
  TQBoxLayout *topLayout = new TQVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  createFrequencySpinBar( this, topLayout, i18n("&Recur every"), i18n("year(s)") );


  TQButtonGroup *buttonGroup = new TQButtonGroup( this );
  buttonGroup->setFrameStyle( TQFrame::NoFrame );
  topLayout->addWidget( buttonGroup, 1, AlignVCenter );

  TQBoxLayout *buttonLayout = new TQVBoxLayout( buttonGroup );


  /* YearlyMonth (day n of Month Y) */
  TQBoxLayout *monthLayout = new TQHBoxLayout( buttonLayout );
  TQString recurInMonthText(
      i18n("part before XXX of 'Recur on day XXX of month YYY'",
      "&Recur on day "));
  if ( KOPrefs::instance()->mCompactDialogs ) {
    recurInMonthText = i18n("&Day ");
  }
  mByMonthRadio = new TQRadioButton( recurInMonthText, buttonGroup );
  TQWhatsThis::add( mByMonthRadio,
       i18n("Sets a specific day in a specific month on which "
      "this event or to-do should recur.") );
  monthLayout->addWidget( mByMonthRadio );
  mByMonthSpin = new TQSpinBox( 1, 31, 1, buttonGroup );
  TQWhatsThis::add( mByMonthSpin,
       i18n("The day of the month on which this event or to-do "
      "should recur.") );
  monthLayout->addWidget( mByMonthSpin );
  TQLabel *ofLabel = new TQLabel(
      i18n("part between XXX and YYY of 'Recur on day XXX of month YYY'", " &of "),
      buttonGroup );
  //What do I do here? I'm not sure if this label should have What's This in it... - Antonio
  monthLayout->addWidget( ofLabel );

  mByMonthCombo = createMonthNameCombo( buttonGroup );
  monthLayout->addWidget( mByMonthCombo );
  ofLabel->setBuddy( mByMonthCombo );

  monthLayout->addStretch( 1 );


  /* YearlyPos (weekday X of week N of month Y) */
  TQBoxLayout *posLayout = new TQHBoxLayout( buttonLayout );
  TQString recurOnPosText( i18n("Part before XXX in 'Recur on NNN. WEEKDAY of MONTH', short version", "&On" ) );
  if ( !KOPrefs::instance()->mCompactDialogs ) {
    recurOnPosText = i18n("Part before XXX in 'Recur on NNN. WEEKDAY of MONTH'", "&On the" );
  }
  mByPosRadio = new TQRadioButton( recurOnPosText, buttonGroup );
  TQWhatsThis::add( mByPosRadio,
       i18n("Sets a specific day in a specific week of a specific "
      "month on which this event or to-do should recur.") );
  posLayout->addWidget( mByPosRadio );

  mByPosDayCombo = createWeekCountCombo( buttonGroup );
  posLayout->addWidget( mByPosDayCombo );

  mByPosWeekdayCombo = createWeekdayCombo( buttonGroup );
  posLayout->addWidget( mByPosWeekdayCombo );

  ofLabel = new TQLabel(
      i18n("part between WEEKDAY and MONTH in 'Recur on NNN. WEEKDAY of MONTH'", " o&f "),
      buttonGroup );
  posLayout->addWidget( ofLabel );

  mByPosMonthCombo  = createMonthNameCombo( buttonGroup );
  posLayout->addWidget( mByPosMonthCombo );
  ofLabel->setBuddy( mByPosMonthCombo );

  posLayout->addStretch( 1 );


  /* YearlyDay (day N of the year) */
  TQBoxLayout *dayLayout = new TQHBoxLayout( buttonLayout );
  TQString recurOnDayText;
  if ( KOPrefs::instance()->mCompactDialogs ) {
    recurOnDayText = i18n("Day #");
  } else {
    recurOnDayText = i18n("Recur on &day #");
  }
  TQString whatsThis = i18n("Sets a specific day within the year on which this "
         "event or to-do should recur.");
  mByDayRadio = new TQRadioButton( recurOnDayText, buttonGroup );
  TQWhatsThis::add( mByDayRadio, whatsThis );
  dayLayout->addWidget( mByDayRadio );

  mByDaySpin = new TQSpinBox( 1, 366, 1, buttonGroup );
  TQWhatsThis::add( mByDaySpin, whatsThis );

  dayLayout->addWidget( mByDaySpin );

  TQString ofTheYear( i18n("part after NNN of 'Recur on day #NNN of the year'", " of the &year"));
  if ( KOPrefs::instance()->mCompactDialogs ) {
    ofTheYear = i18n("part after NNN of 'Recur on day #NNN of the year', short version",
        " of the year");
  }
  ofLabel = new TQLabel( ofTheYear, buttonGroup );
  TQWhatsThis::add( ofLabel, whatsThis );
  dayLayout->addWidget( ofLabel );
  ofLabel->setBuddy( mByDaySpin );

  dayLayout->addStretch( 1 );
}

void RecurYearly::setByDay( int day )
{
  mByDayRadio->setChecked( true );
  mByDaySpin->setValue( day );
}

void RecurYearly::setByPos( int count, int weekday, int month )
{
  mByPosRadio->setChecked( true );
  if ( count > 0 )
    mByPosDayCombo->setCurrentItem( count - 1 );
  else
    mByPosDayCombo->setCurrentItem( -count + 4 );
  mByPosWeekdayCombo->setCurrentItem( weekday - 1 );
  mByPosMonthCombo->setCurrentItem( month-1 );
}

void RecurYearly::setByMonth( int day, int month )
{
  mByMonthRadio->setChecked( true );
  mByMonthSpin->setValue( day );
  mByMonthCombo->setCurrentItem( month - 1 );
}

RecurYearly::YearlyType RecurYearly::getType()
{
  if ( mByMonthRadio->isChecked() ) return byMonth;
  if ( mByPosRadio->isChecked() ) return byPos;
  if ( mByDayRadio->isChecked() ) return byDay;
  return byMonth;
}

int RecurYearly::monthDay()
{
  return mByMonthSpin->value();
}

int RecurYearly::month()
{
  return mByMonthCombo->currentItem() + 1;
}

int RecurYearly::posCount()
{
  int pos = mByPosDayCombo->currentItem();
  if ( pos <= 4 ) // positive  count
    return pos + 1;
  else
    return -pos + 4;
}

int RecurYearly::posWeekday()
{
  return mByPosWeekdayCombo->currentItem() + 1;
}

int RecurYearly::posMonth()
{
  return mByPosMonthCombo->currentItem() + 1;
}

int RecurYearly::day()
{
  return mByDaySpin->value();
}

//////////////////////////// ExceptionsWidget //////////////////////////

ExceptionsWidget::ExceptionsWidget( TQWidget *parent, const char *name ) :
  TQWidget( parent, name )
{
  TQBoxLayout *topLayout = new TQVBoxLayout( this );

  TQGroupBox *groupBox = new TQGroupBox( 1, Horizontal, i18n("E&xceptions"),
                                       this );
  topLayout->addWidget( groupBox );

  TQWidget *box = new TQWidget( groupBox );

  TQGridLayout *boxLayout = new TQGridLayout( box );

  mExceptionDateEdit = new KDateEdit( box );
  TQWhatsThis::add( mExceptionDateEdit,
       i18n("A date that should be considered an exception "
      "to the recurrence rules for this event or to-do.") );
  mExceptionDateEdit->setDate( TQDate::currentDate() );
  boxLayout->addWidget( mExceptionDateEdit, 0, 0 );

  TQPushButton *addExceptionButton = new TQPushButton(
      i18n( "Add a new recurrence to the recurrence list", "&Add" ), box );
  TQWhatsThis::add( addExceptionButton,
       i18n("Add this date as an exception "
      "to the recurrence rules for this event or to-do.") );
  boxLayout->addWidget( addExceptionButton, 1, 0 );
  TQPushButton *changeExceptionButton = new TQPushButton( i18n("&Change"), box );
  TQWhatsThis::add( changeExceptionButton,
       i18n("Replace the currently selected date with this date.") );
  boxLayout->addWidget( changeExceptionButton, 2, 0 );
  TQPushButton *deleteExceptionButton = new TQPushButton( i18n("&Delete"), box );
  TQWhatsThis::add( deleteExceptionButton,
       i18n("Delete the currently selected date from the list of dates "
            "that should be considered exceptions to the recurrence rules "
            "for this event or to-do.") );
  boxLayout->addWidget( deleteExceptionButton, 3, 0 );

  mExceptionList = new TQListBox( box );
  TQWhatsThis::add( mExceptionList,
       i18n("Displays current dates that are being considered "
      "exceptions to the recurrence rules for this event "
      "or to-do.") );
  boxLayout->addMultiCellWidget( mExceptionList, 0, 3, 1, 1 );

  boxLayout->setRowStretch( 4, 1 );
  boxLayout->setColStretch( 1, 3 );

  connect( addExceptionButton, TQT_SIGNAL( clicked() ),
           TQT_SLOT( addException() ) );
  connect( changeExceptionButton, TQT_SIGNAL( clicked() ),
           TQT_SLOT( changeException() ) );
  connect( deleteExceptionButton, TQT_SIGNAL( clicked() ),
           TQT_SLOT( deleteException() ) );
}

void ExceptionsWidget::addException()
{
  TQDate date = mExceptionDateEdit->date();
  TQString dateStr = KGlobal::locale()->formatDate( date );
  if( !mExceptionList->findItem( dateStr ) ) {
    mExceptionDates.append( date );
    mExceptionList->insertItem( dateStr );
  }
}

void ExceptionsWidget::changeException()
{
  int pos = mExceptionList->currentItem();
  if ( pos < 0 ) return;

  TQDate date = mExceptionDateEdit->date();
  mExceptionDates[ pos ] = date;
  mExceptionList->changeItem( KGlobal::locale()->formatDate( date ), pos );
}

void ExceptionsWidget::deleteException()
{
  int pos = mExceptionList->currentItem();
  if ( pos < 0 ) return;

  mExceptionDates.remove( mExceptionDates.at( pos ) );
  mExceptionList->removeItem( pos );
}

void ExceptionsWidget::setDates( const DateList &dates )
{
  mExceptionList->clear();
  mExceptionDates.clear();
  DateList::ConstIterator dit;
  for ( dit = dates.begin(); dit != dates.end(); ++dit ) {
    mExceptionList->insertItem( KGlobal::locale()->formatDate(* dit ) );
    mExceptionDates.append( *dit );
  }
}

DateList ExceptionsWidget::dates()
{
  return mExceptionDates;
}

///////////////////////// ExceptionsDialog ///////////////////////////

ExceptionsDialog::ExceptionsDialog( TQWidget *parent, const char *name ) :
  KDialogBase( parent, name, true, i18n("Edit Exceptions"), Ok|Cancel )
{
  mExceptions = new ExceptionsWidget( this );
  setMainWidget( mExceptions );
}

void ExceptionsDialog::setDates( const DateList &dates )
{
  mExceptions->setDates( dates );
}

DateList ExceptionsDialog::dates()
{
  return mExceptions->dates();
}

///////////////////////// RecurrenceRangeWidget ///////////////////////////

RecurrenceRangeWidget::RecurrenceRangeWidget( TQWidget *parent,
                                              const char *name )
  : TQWidget( parent, name )
{
  TQBoxLayout *topLayout = new TQVBoxLayout( this );

  mRangeGroupBox = new TQGroupBox( 1, Horizontal, i18n("Recurrence Range"),
                                  this );
  TQWhatsThis::add( mRangeGroupBox,
       i18n("Sets a range for which these recurrence rules will "
      "apply to this event or to-do.") );
  topLayout->addWidget( mRangeGroupBox );

  TQWidget *rangeBox = new TQWidget( mRangeGroupBox );
  TQVBoxLayout *rangeLayout = new TQVBoxLayout( rangeBox );
  rangeLayout->setSpacing( KDialog::spacingHint() );

  mStartDateLabel = new TQLabel( i18n("Begin on:"), rangeBox );
  TQWhatsThis::add( mStartDateLabel,
       i18n("The date on which the recurrences for this event or to-do "
      "should begin.") );
  rangeLayout->addWidget( mStartDateLabel );

  TQButtonGroup *rangeButtonGroup = new TQButtonGroup( this );
  rangeButtonGroup->hide();

  mNoEndDateButton = new TQRadioButton( i18n("&No ending date"), rangeBox );
  TQWhatsThis::add( mNoEndDateButton,
       i18n("Sets the event or to-do to recur forever.") );
  rangeButtonGroup->insert( mNoEndDateButton );
  rangeLayout->addWidget( mNoEndDateButton );

  TQBoxLayout *durationLayout = new TQHBoxLayout( rangeLayout );
  durationLayout->setSpacing( KDialog::spacingHint() );

  mEndDurationButton = new TQRadioButton( i18n("End &after"), rangeBox );
  TQWhatsThis::add( mEndDurationButton,
       i18n("Sets the event or to-do to stop recurring after a "
      "certain number of occurrences.") );
  rangeButtonGroup->insert( mEndDurationButton );
  durationLayout->addWidget( mEndDurationButton );

  TQString whatsThis = i18n("Number of times the event or to-do should recur "
           "before stopping.");
  mEndDurationEdit = new TQSpinBox( 1, 9999, 1, rangeBox );
  TQWhatsThis::add( mEndDurationEdit, whatsThis );
  durationLayout->addWidget( mEndDurationEdit );

  TQLabel *endDurationLabel = new TQLabel( i18n("&occurrence(s)"), rangeBox );
  TQWhatsThis::add( endDurationLabel, whatsThis );
  durationLayout ->addWidget( endDurationLabel );
  endDurationLabel->setBuddy( mEndDurationEdit );

  TQBoxLayout *endDateLayout = new TQHBoxLayout( rangeLayout );
  endDateLayout->setSpacing( KDialog::spacingHint() );

  mEndDateButton = new TQRadioButton( i18n("End &on:"), rangeBox );
  TQWhatsThis::add( mEndDateButton,
                   i18n("Sets the event or to-do to stop recurring on "
                        "a certain date.") );
  rangeButtonGroup->insert( mEndDateButton );
  endDateLayout->addWidget( mEndDateButton );

  mEndDateEdit = new KDateEdit( rangeBox );
  TQWhatsThis::add( mEndDateEdit,
                   i18n("Date after which the event or to-do should stop "
                        "recurring") );
  endDateLayout->addWidget( mEndDateEdit );

  endDateLayout->addStretch( 1 );

  connect( mNoEndDateButton, TQT_SIGNAL( toggled( bool ) ),
           TQT_SLOT( showCurrentRange() ) );
  connect( mEndDurationButton, TQT_SIGNAL( toggled( bool ) ),
           TQT_SLOT( showCurrentRange() ) );
  connect( mEndDateButton, TQT_SIGNAL( toggled( bool ) ),
           TQT_SLOT( showCurrentRange() ) );
}

void RecurrenceRangeWidget::setDefaults( const TQDateTime &from  )
{
  mNoEndDateButton->setChecked( true );

  setDateTimes( from );
  setEndDate( from.date() );
}

void RecurrenceRangeWidget::setDuration( int duration )
{
  if ( duration == -1 ) {
    mNoEndDateButton->setChecked( true );
  } else if ( duration == 0 ) {
    mEndDateButton->setChecked( true );
  } else {
    mEndDurationButton->setChecked( true );
    mEndDurationEdit->setValue( duration );
  }
}

int RecurrenceRangeWidget::duration()
{
  if ( mNoEndDateButton->isChecked() ) {
    return -1;
  } else if ( mEndDurationButton->isChecked() ) {
    return mEndDurationEdit->value();
  } else {
    return 0;
  }
}

void RecurrenceRangeWidget::setEndDate( const TQDate &date )
{
  mEndDateEdit->setDate( date );
}

TQDate RecurrenceRangeWidget::endDate()
{
  return mEndDateEdit->date();
}

void RecurrenceRangeWidget::showCurrentRange()
{
  mEndDurationEdit->setEnabled( mEndDurationButton->isChecked() );
  mEndDateEdit->setEnabled( mEndDateButton->isChecked() );
}

void RecurrenceRangeWidget::setDateTimes( const TQDateTime &start,
                                          const TQDateTime & )
{
  mStartDateLabel->setText( i18n("Begins on: %1")
      .arg( KGlobal::locale()->formatDate( start.date() ) ) );
}

///////////////////////// RecurrenceRangeDialog ///////////////////////////

RecurrenceRangeDialog::RecurrenceRangeDialog( TQWidget *parent,
                                              const char *name ) :
  KDialogBase( parent, name, true, i18n("Edit Recurrence Range"), Ok|Cancel )
{
  mRecurrenceRangeWidget = new RecurrenceRangeWidget( this );
  setMainWidget( mRecurrenceRangeWidget );
}

void RecurrenceRangeDialog::setDefaults( const TQDateTime &from )
{
  mRecurrenceRangeWidget->setDefaults( from );
}

void RecurrenceRangeDialog::setDuration( int duration )
{
  mRecurrenceRangeWidget->setDuration( duration );
}

int RecurrenceRangeDialog::duration()
{
  return mRecurrenceRangeWidget->duration();
}

void RecurrenceRangeDialog::setEndDate( const TQDate &date )
{
  mRecurrenceRangeWidget->setEndDate( date );
}

TQDate RecurrenceRangeDialog::endDate()
{
  return mRecurrenceRangeWidget->endDate();
}

void RecurrenceRangeDialog::setDateTimes( const TQDateTime &start,
                                          const TQDateTime &end )
{
  mRecurrenceRangeWidget->setDateTimes( start, end );
}

//////////////////////////// RecurrenceChooser ////////////////////////

RecurrenceChooser::RecurrenceChooser( TQWidget *parent, const char *name ) :
  TQWidget( parent, name )
{
  TQBoxLayout *topLayout = new TQVBoxLayout( this );

  if ( KOPrefs::instance()->mCompactDialogs ) {
    mTypeCombo = new TQComboBox( this );
    TQWhatsThis::add( mTypeCombo,
                     i18n("Sets the type of recurrence this event or to-do "
                          "should have.") );
    mTypeCombo->insertItem( i18n("Daily") );
    mTypeCombo->insertItem( i18n("Weekly") );
    mTypeCombo->insertItem( i18n("Monthly") );
    mTypeCombo->insertItem( i18n("Yearly") );

    topLayout->addWidget( mTypeCombo );

    connect( mTypeCombo, TQT_SIGNAL( activated( int ) ), TQT_SLOT( emitChoice() ) );
  } else {
    mTypeCombo = 0;

    TQButtonGroup *ruleButtonGroup = new TQButtonGroup( 1, Horizontal, this );
    ruleButtonGroup->setFrameStyle( TQFrame::NoFrame );
    topLayout->addWidget( ruleButtonGroup );

    mDailyButton = new TQRadioButton( i18n("&Daily"), ruleButtonGroup );
    TQWhatsThis::add( mDailyButton,
                     i18n("Sets the event or to-do to recur daily according "
                          "to the specified rules.") );
    mWeeklyButton = new TQRadioButton( i18n("&Weekly"), ruleButtonGroup );
    TQWhatsThis::add( mWeeklyButton,
                     i18n("Sets the event or to-do to recur weekly according "
                          "to the specified rules.") );
    mMonthlyButton = new TQRadioButton( i18n("&Monthly"), ruleButtonGroup );
    TQWhatsThis::add( mMonthlyButton,
                     i18n("Sets the event or to-do to recur monthly according "
                          "to the specified rules.") );
    mYearlyButton = new TQRadioButton( i18n("&Yearly"), ruleButtonGroup );
    TQWhatsThis::add( mYearlyButton,
                     i18n("Sets the event or to-do to recur yearly according "
                          "to the specified rules.") );

    connect( mDailyButton, TQT_SIGNAL( toggled( bool ) ),
             TQT_SLOT( emitChoice() ) );
    connect( mWeeklyButton, TQT_SIGNAL( toggled( bool ) ),
             TQT_SLOT( emitChoice() ) );
    connect( mMonthlyButton, TQT_SIGNAL( toggled( bool ) ),
             TQT_SLOT( emitChoice() ) );
    connect( mYearlyButton, TQT_SIGNAL( toggled( bool ) ),
             TQT_SLOT( emitChoice() ) );
  }
}

int RecurrenceChooser::type()
{
  if ( mTypeCombo ) {
    return mTypeCombo->currentItem();
  } else {
    if ( mDailyButton->isChecked() ) return Daily;
    else if ( mWeeklyButton->isChecked() ) return Weekly;
    else if ( mMonthlyButton->isChecked() ) return Monthly;
    else return Yearly;
  }
}

void RecurrenceChooser::setType( int type )
{
  if ( mTypeCombo ) {
    mTypeCombo->setCurrentItem( type );
  } else {
    switch ( type ) {
      case Daily:
        mDailyButton->setChecked( true );
        break;
      case Weekly:
        mWeeklyButton->setChecked( true );
        break;
      case Monthly:
        mMonthlyButton->setChecked( true );
        break;
      case Yearly:
      default:
        mYearlyButton->setChecked( true );
        break;
    }
  }
}

void RecurrenceChooser::emitChoice()
{
  emit chosen ( type() );
}

/////////////////////////////// Main Widget /////////////////////////////

KOEditorRecurrence::KOEditorRecurrence( TQWidget* parent, const char *name ) :
  TQWidget( parent, name )
{
  TQGridLayout *topLayout = new TQGridLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  mEnabledCheck = new TQCheckBox( i18n("&Enable recurrence"), this );
  TQWhatsThis::add( mEnabledCheck,
                   i18n("Enables recurrence for this event or to-do according "
                        "to the specified rules.") );
  connect( mEnabledCheck, TQT_SIGNAL( toggled( bool ) ),
           TQT_SLOT( setRecurrenceEnabled( bool ) ) );
  topLayout->addMultiCellWidget( mEnabledCheck, 0, 0, 0, 1 );


  mTimeGroupBox = new TQGroupBox( 1, Horizontal, i18n("Appointment Time "),
                                 this );
  TQWhatsThis::add( mTimeGroupBox,
                   i18n("Displays appointment time information.") );
  topLayout->addMultiCellWidget( mTimeGroupBox, 1, 1 , 0 , 1 );

  if ( KOPrefs::instance()->mCompactDialogs ) {
    mTimeGroupBox->hide();
  }

//  TQFrame *timeFrame = new TQFrame( mTimeGroupBox );
//  TQBoxLayout *layoutTimeFrame = new TQHBoxLayout( timeFrame );
//  layoutTimeFrame->setSpacing( KDialog::spacingHint() );

  mDateTimeLabel = new TQLabel( mTimeGroupBox );
//  mDateTimeLabel = new TQLabel( timeFrame );
//  layoutTimeFrame->addWidget( mDateTimeLabel );

  Qt::Orientation orientation;
  if ( KOPrefs::instance()->mCompactDialogs ) orientation = Horizontal;
  else orientation = Vertical;

  mRuleBox = new TQGroupBox( 1, orientation, i18n("Recurrence Rule"), this );
  TQWhatsThis::add( mRuleBox,
                   i18n("Options concerning the type of recurrence this event "
                        "or to-do should have.") );
  if ( KOPrefs::instance()->mCompactDialogs ) {
    topLayout->addWidget( mRuleBox, 2, 0 );
  } else {
    topLayout->addMultiCellWidget( mRuleBox, 2, 2, 0, 1 );
  }

  mRecurrenceChooser = new RecurrenceChooser( mRuleBox );
  connect( mRecurrenceChooser, TQT_SIGNAL( chosen( int ) ),
           TQT_SLOT( showCurrentRule( int ) ) );

  if ( !KOPrefs::instance()->mCompactDialogs ) {
    TQFrame *ruleSepFrame = new TQFrame( mRuleBox );
    ruleSepFrame->setFrameStyle( TQFrame::VLine | TQFrame::Sunken );
  }

  mRuleStack = new TQWidgetStack( mRuleBox );

  mDaily = new RecurDaily( mRuleStack );
  mRuleStack->addWidget( mDaily, 0 );

  mWeekly = new RecurWeekly( mRuleStack );
  mRuleStack->addWidget( mWeekly, 0 );

  mMonthly = new RecurMonthly( mRuleStack );
  mRuleStack->addWidget( mMonthly, 0 );

  mYearly = new RecurYearly( mRuleStack );
  mRuleStack->addWidget( mYearly, 0 );

  showCurrentRule( mRecurrenceChooser->type() );

  if ( KOPrefs::instance()->mCompactDialogs ) {
    mRecurrenceRangeWidget = 0;
    mRecurrenceRangeDialog = new RecurrenceRangeDialog( this );
    mRecurrenceRange = mRecurrenceRangeDialog;
    mRecurrenceRangeButton = new TQPushButton( i18n("Recurrence Range..."),
                                              this );
    TQWhatsThis::add( mRecurrenceRangeButton,
                     i18n("Options concerning the time range during which "
                          "this event or to-do should recur.") );
    topLayout->addWidget( mRecurrenceRangeButton, 3, 0 );
    connect( mRecurrenceRangeButton, TQT_SIGNAL( clicked() ),
             TQT_SLOT( showRecurrenceRangeDialog() ) );

    mExceptionsWidget = 0;
    mExceptionsDialog = new ExceptionsDialog( this );
    mExceptions = mExceptionsDialog;
    mExceptionsButton = new TQPushButton( i18n("Exceptions..."), this );
    topLayout->addWidget( mExceptionsButton, 4, 0 );
    connect( mExceptionsButton, TQT_SIGNAL( clicked() ),
             TQT_SLOT( showExceptionsDialog() ) );

  } else {
    mRecurrenceRangeWidget = new RecurrenceRangeWidget( this );
    TQWhatsThis::add( mRecurrenceRangeWidget,
                     i18n("Options concerning the time range during which "
                          "this event or to-do should recur.") );
    mRecurrenceRangeDialog = 0;
    mRecurrenceRange = mRecurrenceRangeWidget;
    mRecurrenceRangeButton = 0;
    topLayout->addWidget( mRecurrenceRangeWidget, 3, 0 );

    mExceptionsWidget = new ExceptionsWidget( this );
    mExceptionsDialog = 0;
    mExceptions = mExceptionsWidget;
    mExceptionsButton = 0;
    topLayout->addWidget( mExceptionsWidget, 3, 1 );
  }

  // set some initial defaults for the saved recurrence
  mSaveRec.setDuration( -1 ); // never ending
}

KOEditorRecurrence::~KOEditorRecurrence()
{
}

void KOEditorRecurrence::setRecurrenceEnabled( bool enabled )
{
//  kdDebug(5850) << "KOEditorRecurrence::setRecurrenceEnabled(): " << (enabled ? "on" : "off") << endl;

  mEnabledCheck->setChecked( enabled );
  mTimeGroupBox->setEnabled( enabled );
  mRuleBox->setEnabled( enabled );
  if ( mRecurrenceRangeWidget ) mRecurrenceRangeWidget->setEnabled( enabled );
  if ( mRecurrenceRangeButton ) mRecurrenceRangeButton->setEnabled( enabled );
  if ( mExceptionsWidget ) mExceptionsWidget->setEnabled( enabled );
  if ( mExceptionsButton ) mExceptionsButton->setEnabled( enabled );
}

void KOEditorRecurrence::showCurrentRule( int current )
{
  switch ( current ) {
    case Daily:
      mRuleStack->raiseWidget( mDaily );
      break;
    case Weekly:
      mRuleStack->raiseWidget( mWeekly );
      break;
    case Monthly:
      mRuleStack->raiseWidget( mMonthly );
      break;
    default:
    case Yearly:
      mRuleStack->raiseWidget( mYearly );
      break;
  }
}

void KOEditorRecurrence::setDateTimes( const TQDateTime &start, const TQDateTime &end )
{
//  kdDebug(5850) << "KOEditorRecurrence::setDateTimes" << endl;

  mEventStartDt = start;
  mRecurrenceRange->setDateTimes( start, end );
  mDaily->setDateTimes( start, end );
  mWeekly->setDateTimes( start, end );
  mMonthly->setDateTimes( start, end );
  mYearly->setDateTimes( start, end );

  // Now set the defaults for all unused types, use the start time for it
  bool enabled = mEnabledCheck->isChecked();
  int type = mRecurrenceChooser->type();

  if ( !enabled || type != RecurrenceChooser::Weekly ) {
    TQBitArray days( 7 );
    days.fill( 0 );
    days.setBit( (start.date().dayOfWeek()+6) % 7 );
    mWeekly->setDays( days );
  }
  if ( !enabled || type != RecurrenceChooser::Monthly ) {
    mMonthly->setByPos( ( start.date().day() - 1 ) / 7 + 1, start.date().dayOfWeek() - 1 );
    mMonthly->setByDay( start.date().day() );
  }
  if ( !enabled || type != RecurrenceChooser::Yearly ) {
    mYearly->setByDay( start.date().dayOfYear() );
    mYearly->setByPos( ( start.date().day() - 1 ) / 7 + 1,
        start.date().dayOfWeek() - 1, start.date().month() );
    mYearly->setByMonth( start.date().day(), start.date().month() );
  }
}

void KOEditorRecurrence::setDefaults( const TQDateTime &from, const TQDateTime &to, bool )
{
  setDateTimes( from, to );

  setRecurrenceEnabled( false );

  mRecurrenceRange->setDefaults( from );

  mRecurrenceChooser->setType( RecurrenceChooser::Weekly );
  showCurrentRule( mRecurrenceChooser->type() );

  mDaily->setFrequency( 1 );

  mWeekly->setFrequency( 1 );
  TQBitArray days( 7 );
  days.fill( 0 );
  days.setBit( (from.date().dayOfWeek()+6) % 7 );
  mWeekly->setDays( days );

  mMonthly->setFrequency( 1 );
  mMonthly->setByPos( ( from.date().day() - 1 ) / 7 + 1, from.date().dayOfWeek() );
  mMonthly->setByDay( from.date().day() );

  mYearly->setFrequency( 1 );
  mYearly->setByDay( from.date().dayOfYear() );
  mYearly->setByPos( ( from.date().day() - 1 ) / 7 + 1,
      from.date().dayOfWeek(), from.date().month() );
  mYearly->setByMonth( from.date().day(), from.date().month() );
}

void KOEditorRecurrence::readIncidence(Incidence *incidence)
{
  if (!incidence) return;

  TQBitArray rDays( 7 );
  int day = 0;
  int count = 0;
  int month = 0;

  if ( incidence->type() == "Todo" ) {
    Todo *todo = static_cast<Todo *>(incidence);
    setDefaults( todo->dtStart(true), todo->dtDue(), todo->doesFloat() );
  } else {
    setDefaults( incidence->dtStart(), incidence->dtEnd(), incidence->doesFloat() );
  }

  uint recurs = incidence->recurrenceType();
  int f = 0;
  Recurrence *r = 0;

  if ( recurs ) {
    r = incidence->recurrence();
    f = r->frequency();
  }

  setRecurrenceEnabled( recurs );

  int recurrenceType = RecurrenceChooser::Weekly;

  switch ( recurs ) {
    case Recurrence::rNone:
      break;
    case Recurrence::rDaily:
      recurrenceType = RecurrenceChooser::Daily;
      mDaily->setFrequency( f );
      break;
    case Recurrence::rWeekly:
      recurrenceType = RecurrenceChooser::Weekly;
      mWeekly->setFrequency( f );
      mWeekly->setDays( r->days() );
      break;
    case Recurrence::rMonthlyPos: {
      // TODO: we only handle one possibility in the list right now,
      // so I have hardcoded calls with first().  If we make the GUI
      // more extended, this can be changed.
      recurrenceType = RecurrenceChooser::Monthly;

      TQValueList<RecurrenceRule::WDayPos> rmp = r->monthPositions();
      if ( !rmp.isEmpty() ) {
        mMonthly->setByPos( rmp.first().pos(), rmp.first().day() );
      }

      mMonthly->setFrequency( f );

      break; }
    case Recurrence::rMonthlyDay: {
      recurrenceType = RecurrenceChooser::Monthly;

      TQValueList<int> rmd = r->monthDays();
      // check if we have any setting for which day (vcs import is broken and
      // does not set any day, thus we need to check)
      if ( rmd.isEmpty() ) {
        day = incidence->dtStart().date().day();
      } else {
        day = rmd.first();
      }
      mMonthly->setByDay( day );

      mMonthly->setFrequency( f );

      break; }
    case Recurrence::rYearlyMonth: {
      recurrenceType = RecurrenceChooser::Yearly;
      TQValueList<int> rmd = r->yearDates();
      if ( rmd.isEmpty() ) {
        day = incidence->dtStart().date().day();
      } else {
        day = rmd.first();
      }
      int month = incidence->dtStart().date().month();
      rmd = r->yearMonths();
      if ( !rmd.isEmpty() )
        month = rmd.first();
      mYearly->setByMonth( day, month );
      mYearly->setFrequency( f );
      break; }
    case Recurrence::rYearlyPos: {
      recurrenceType = RecurrenceChooser::Yearly;

      TQValueList<int> months = r->yearMonths();
      if ( months.isEmpty() ) {
        month = incidence->dtStart().date().month();
      } else {
        month = months.first();
      }

      TQValueList<RecurrenceRule::WDayPos> pos = r->yearPositions();

      if ( pos.isEmpty() ) {
        // Use dtStart if nothing is given (shouldn't happen!)
        count = ( incidence->dtStart().date().day() - 1 ) / 7;
        day = incidence->dtStart().date().dayOfWeek();
      } else {
        count = pos.first().pos();
        day = pos.first().day();
      }
      mYearly->setByPos( count, day, month );
      mYearly->setFrequency( f );
      break; }
    case Recurrence::rYearlyDay: {
      recurrenceType = RecurrenceChooser::Yearly;
      TQValueList<int> days = r->yearDays();
      if ( days.isEmpty() ) {
        day = incidence->dtStart().date().dayOfYear();
      } else {
        day = days.first();
      }
      mYearly->setByDay( day );

      mYearly->setFrequency( f );
      break; }
    default:
      break;
  }

  mRecurrenceChooser->setType( recurrenceType );
  showCurrentRule( recurrenceType );

  mRecurrenceRange->setDateTimes( incidence->recurrence()->startDateTime() );

  if ( incidence->doesRecur() ) {
    mRecurrenceRange->setDuration( r->duration() );
    if ( r->duration() == 0 ) mRecurrenceRange->setEndDate( r->endDate() );
  }

  mExceptions->setDates( incidence->recurrence()->exDates() );
}

void KOEditorRecurrence::writeIncidence( Incidence *incidence )
{
  if ( !mEnabledCheck->isChecked() || !isEnabled() )
  {
    if ( incidence->doesRecur() )
      incidence->recurrence()->unsetRecurs();
    return;
  }

  Recurrence *r = incidence->recurrence();

  // clear out any old settings;
  r->unsetRecurs();

  int duration = mRecurrenceRange->duration();
  TQDate endDate;
  if ( duration == 0 ) endDate = mRecurrenceRange->endDate();

  int recurrenceType = mRecurrenceChooser->type();
  if ( recurrenceType == RecurrenceChooser::Daily ) {
    r->setDaily( mDaily->frequency() );
  } else if ( recurrenceType == RecurrenceChooser::Weekly ) {
    r->setWeekly( mWeekly->frequency(), mWeekly->days() );
  } else if ( recurrenceType == RecurrenceChooser::Monthly ) {
    r->setMonthly( mMonthly->frequency() );

    if ( mMonthly->byPos() ) {
      int pos = mMonthly->count();

      TQBitArray days( 7 );
      days.fill( false );
      days.setBit( mMonthly->weekday() - 1 );
      r->addMonthlyPos( pos, days );
    } else {
      // it's by day
      r->addMonthlyDate( mMonthly->day() );
    }
  } else if ( recurrenceType == RecurrenceChooser::Yearly ) {
    r->setYearly( mYearly->frequency() );

    switch ( mYearly->getType() ) {
      case RecurYearly::byMonth:
        r->addYearlyDate( mYearly->monthDay() );
        r->addYearlyMonth( mYearly->month() );
        break;
      case RecurYearly::byPos:  {
        r->addYearlyMonth( mYearly->posMonth() );
        TQBitArray days( 7 );
        days.fill( false );
        days.setBit( mYearly->posWeekday() - 1 );
        r->addYearlyPos( mYearly->posCount(), days );
        break; }
      case RecurYearly::byDay:
        r->addYearlyDay( mYearly->day() );
        break;
    }
  } // end "Yearly"

  if ( duration > 0 )
    r->setDuration( duration );
  else if ( duration == 0 )
    r->setEndDate( endDate );
  incidence->recurrence()->setExDates( mExceptions->dates() );
}

void KOEditorRecurrence::setDateTimeStr( const TQString &str )
{
  mDateTimeLabel->setText( str );
}

bool KOEditorRecurrence::validateInput()
{
  // Check input here.
  // Check if the recurrence (if set to end at a date) is scheduled to end before the event starts.
  if ( mEnabledCheck->isChecked() && (mRecurrenceRange->duration()==0) &&
       mEventStartDt.isValid() && ((mRecurrenceRange->endDate())<mEventStartDt.date()) ) {
    KMessageBox::sorry( 0,
      i18n("The end date '%1' of the recurrence must be after the start date '%2' of the event.")
      .arg( KGlobal::locale()->formatDate( mRecurrenceRange->endDate() ) )
      .arg( KGlobal::locale()->formatDate( mEventStartDt.date() ) ) );
    return false;
  }
  int recurrenceType = mRecurrenceChooser->type();
  // Check if a weekly recurrence has at least one day selected
  // TODO: Get rid of this, it's not really needed (by default the day should be taken from dtStart)
  if( mEnabledCheck->isChecked() && recurrenceType == RecurrenceChooser::Weekly ) {
    const TQBitArray &days = mWeekly->days();
    bool valid = false;
    for ( int i=0; i<7; ++i ) valid = valid || days.testBit( i );
    if ( !valid ) {
      KMessageBox::sorry( 0,
        i18n("A weekly recurring event or task has to have at least one weekday "
             "associated with it.") );
      return false;
    }
  }
  return true;
}

void KOEditorRecurrence::showExceptionsDialog()
{
  DateList dates = mExceptions->dates();
  int result = mExceptionsDialog->exec();
  if ( result == TQDialog::Rejected ) mExceptions->setDates( dates );
}

void KOEditorRecurrence::showRecurrenceRangeDialog()
{
  int duration = mRecurrenceRange->duration();
  TQDate endDate = mRecurrenceRange->endDate();

  int result = mRecurrenceRangeDialog->exec();
  if ( result == TQDialog::Rejected ) {
    mRecurrenceRange->setDuration( duration );
    mRecurrenceRange->setEndDate( endDate );
  }
}

bool KOEditorRecurrence::doesRecur()
{
  return mEnabledCheck->isChecked();
}

void KOEditorRecurrence::saveValues()
{
  int duration = mRecurrenceRange->duration();
  TQDate endDate;
  if ( duration == 0 ) {
    endDate = mRecurrenceRange->endDate();
  }

  int recurrenceType = mRecurrenceChooser->type();
  if ( recurrenceType == RecurrenceChooser::Daily ) {
    mSaveRec.setDaily( mDaily->frequency() );
  } else if ( recurrenceType == RecurrenceChooser::Weekly ) {
    mSaveRec.setWeekly( mWeekly->frequency(), mWeekly->days() );
  } else if ( recurrenceType == RecurrenceChooser::Monthly ) {
    mSaveRec.setMonthly( mMonthly->frequency() );

    if ( mMonthly->byPos() ) {
      int pos = mMonthly->count();

      TQBitArray days( 7 );
      days.fill( false );
      days.setBit( mMonthly->weekday() - 1 );
      mSaveRec.addMonthlyPos( pos, days );
    } else {
      // it's by day
      mSaveRec.addMonthlyDate( mMonthly->day() );
    }
  } else if ( recurrenceType == RecurrenceChooser::Yearly ) {
    mSaveRec.setYearly( mYearly->frequency() );

    switch ( mYearly->getType() ) {
    case RecurYearly::byMonth:
      mSaveRec.addYearlyDate( mYearly->monthDay() );
      mSaveRec.addYearlyMonth( mYearly->month() );
      break;

    case RecurYearly::byPos:
    {
      mSaveRec.addYearlyMonth( mYearly->posMonth() );
      TQBitArray days( 7 );
      days.fill( false );
      days.setBit( mYearly->posWeekday() - 1 );
      mSaveRec.addYearlyPos( mYearly->posCount(), days );
      break;
    }

    case RecurYearly::byDay:
      mSaveRec.addYearlyDay( mYearly->day() );
      break;
    }
  }

 if ( duration > 0 ) {
    mSaveRec.setDuration( duration );
  } else if ( duration == 0 ) {
    mSaveRec.setEndDate( endDate );
  }

  mSaveRec.setExDates( mExceptions->dates() );
}

void KOEditorRecurrence::restoreValues()
{
  TQBitArray rDays( 7 );
  int day = 0;
  int count = 0;
  int month = 0;

  if ( mSaveRec.startDateTime().isValid() && mSaveRec.endDateTime().isValid() ) {
    setDefaults( mSaveRec.startDateTime(), mSaveRec.endDateTime(), mSaveRec.doesFloat() );
  }

  int recurrenceType;
  switch ( mSaveRec.recurrenceType() ) {
  case Recurrence::rNone:
    recurrenceType = RecurrenceChooser::Weekly;
    break;

  case Recurrence::rDaily:
    recurrenceType = RecurrenceChooser::Daily;
    mDaily->setFrequency( mSaveRec.frequency() );
    break;

  case Recurrence::rWeekly:
    recurrenceType = RecurrenceChooser::Weekly;

    mWeekly->setFrequency( mSaveRec.frequency() );
    mWeekly->setDays( mSaveRec.days() );
    break;

  case Recurrence::rMonthlyPos:
  {
    // TODO: we only handle one possibility in the list right now,
    // so I have hardcoded calls with first().  If we make the GUI
    // more extended, this can be changed.
    recurrenceType = RecurrenceChooser::Monthly;

    TQValueList<RecurrenceRule::WDayPos> rmp = mSaveRec.monthPositions();
    if ( !rmp.isEmpty() ) {
      mMonthly->setByPos( rmp.first().pos(), rmp.first().day() );
    }
    mMonthly->setFrequency( mSaveRec.frequency() );
    break;
  }

  case Recurrence::rMonthlyDay:
  {
    recurrenceType = RecurrenceChooser::Monthly;

    TQValueList<int> rmd = mSaveRec.monthDays();
    // check if we have any setting for which day (vcs import is broken and
    // does not set any day, thus we need to check)
    if ( !rmd.isEmpty() ) {
      day = rmd.first();
    }
    if ( day > 0 ) {
      mMonthly->setByDay( day );
      mMonthly->setFrequency( mSaveRec.frequency() );
    }
    break;
  }

  case Recurrence::rYearlyMonth:
  {
    recurrenceType = RecurrenceChooser::Yearly;

    TQValueList<int> rmd = mSaveRec.yearDates();
    if ( !rmd.isEmpty() ) {
      day = rmd.first();
    }
    rmd = mSaveRec.yearMonths();
    if ( !rmd.isEmpty() ) {
      month = rmd.first();
    }
    if ( day > 0 && month > 0 ) {
      mYearly->setByMonth( day, month );
      mYearly->setFrequency( mSaveRec.frequency() );
    }
    break;
  }

  case Recurrence::rYearlyPos:
  {
    recurrenceType = RecurrenceChooser::Yearly;

    TQValueList<int> months = mSaveRec.yearMonths();
    if ( !months.isEmpty() ) {
      month = months.first();
    }
    TQValueList<RecurrenceRule::WDayPos> pos = mSaveRec.yearPositions();
    if ( !pos.isEmpty() ) {
      count = pos.first().pos();
      day = pos.first().day();
    }
    if ( count > 0 && day > 0 && month > 0 ) {
      mYearly->setByPos( count, day, month );
      mYearly->setFrequency( mSaveRec.frequency() );
    }
    break;
  }

  case Recurrence::rYearlyDay:
  {
    recurrenceType = RecurrenceChooser::Yearly;

    TQValueList<int> days = mSaveRec.yearDays();
    if ( !days.isEmpty() ) {
      day = days.first();
    }
    if ( day > 0 ) {
      mYearly->setByDay( day );
      mYearly->setFrequency( mSaveRec.frequency() );
    }
    break;
  }
  default:
    break;
  }

  mRecurrenceChooser->setType( recurrenceType );
  showCurrentRule( recurrenceType );

  if ( mSaveRec.startDateTime().isValid() ) {
    mRecurrenceRange->setDateTimes( mSaveRec.startDateTime() );
  }

  mRecurrenceRange->setDuration( mSaveRec.duration() );
  if ( mSaveRec.duration() == 0 && mSaveRec.endDate().isValid() ) {
    mRecurrenceRange->setEndDate( mSaveRec.endDate() );
  }

  mExceptions->setDates( mSaveRec.exDates() );
}

KOEditorRecurrenceDialog::KOEditorRecurrenceDialog(TQWidget * parent)
  : KDialogBase( parent, 0, false, i18n("Recurrence"), Ok|Cancel ), mRecurEnabled( false )
{
  mRecurrence = new KOEditorRecurrence( this );
  setMainWidget( mRecurrence );
}

void KOEditorRecurrenceDialog::slotOk()
{
  mRecurEnabled = mRecurrence->doesRecur();
  mRecurrence->saveValues();
  emit okClicked(); // tell the incidence editor to update the recurrenceString
  accept();
}

void KOEditorRecurrenceDialog::slotCancel()
{
  mRecurrence->setRecurrenceEnabled( mRecurEnabled );
  mRecurrence->restoreValues();
  reject();
}
