/*
    This file is part of Kontact.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2004 Allen Winter <winter@kde.org>

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

#include <tqbuttongroup.h>
#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqradiobutton.h>
#include <tqspinbox.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kaccelmanager.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>

#include "kcmsdsummary.h"

#include <kdepimmacros.h>

extern "C"
{
  KDE_EXPORT KCModule *create_sdsummary( TQWidget *parent, const char * )
  {
    return new KCMSDSummary( parent, "kcmsdsummary" );
  }
}

KCMSDSummary::KCMSDSummary( TQWidget *parent, const char *name )
  : KCModule( parent, name )
{
  initGUI();

  customDaysChanged( 1 );

  connect( mDaysGroup, TQT_SIGNAL( clicked( int ) ), TQT_SLOT( modified() ) );
  connect( mDaysGroup, TQT_SIGNAL( clicked( int ) ), TQT_SLOT( buttonClicked( int ) ) );
  connect( mCalendarGroup, TQT_SIGNAL( clicked( int ) ), TQT_SLOT( modified() ) );
  connect( mContactGroup, TQT_SIGNAL( clicked( int ) ), TQT_SLOT( modified() ) );
  connect( mCustomDays, TQT_SIGNAL( valueChanged( int ) ), TQT_SLOT( modified() ) );
  connect( mCustomDays, TQT_SIGNAL( valueChanged( int ) ), TQT_SLOT( customDaysChanged( int ) ) );

  KAcceleratorManager::manage( this );

  load();
}

void KCMSDSummary::modified()
{
  emit changed( true );
}

void KCMSDSummary::buttonClicked( int id )
{
  mCustomDays->setEnabled( id == 4 );
}

void KCMSDSummary::customDaysChanged( int value )
{
  mCustomDays->setSuffix( i18n( " day",  " days", value ) );
}

void KCMSDSummary::initGUI()
{
  TQGridLayout *layout = new TQGridLayout( this, 3, 2, KDialog::spacingHint() );

  mDaysGroup = new TQButtonGroup( 0, Vertical, i18n( "Special Dates Summary" ), this );
  TQVBoxLayout *boxLayout = new TQVBoxLayout( mDaysGroup->layout(),
                                            KDialog::spacingHint() );

  TQLabel *label = new TQLabel( i18n( "How many days should the special dates summary show at once?" ), mDaysGroup );
  boxLayout->addWidget( label );

  TQRadioButton *button = new TQRadioButton( i18n( "One day" ), mDaysGroup );
  boxLayout->addWidget( button );

  button = new TQRadioButton( i18n( "Five days" ), mDaysGroup );
  boxLayout->addWidget( button );

  button = new TQRadioButton( i18n( "One week" ), mDaysGroup );
  boxLayout->addWidget( button );

  button = new TQRadioButton( i18n( "One month" ), mDaysGroup );
  boxLayout->addWidget( button );

  TQHBoxLayout *hbox = new TQHBoxLayout( boxLayout, KDialog::spacingHint() );

  button = new TQRadioButton( "", mDaysGroup );
  hbox->addWidget( button );

  mCustomDays = new TQSpinBox( 1, 365, 1, mDaysGroup );
  mCustomDays->setEnabled( false );
  hbox->addWidget( mCustomDays );

  hbox->addStretch( 1 );

  layout->addMultiCellWidget( mDaysGroup, 0, 0, 0, 1 );

  mCalendarGroup = new TQButtonGroup( 1, Horizontal, i18n( "Special Dates From Calendar" ), this );

  mShowBirthdaysFromCal = new TQCheckBox( i18n( "Show birthdays" ), mCalendarGroup );
  mShowAnniversariesFromCal = new TQCheckBox( i18n( "Show anniversaries" ), mCalendarGroup );
  mShowHolidays = new TQCheckBox( i18n( "Show holidays" ), mCalendarGroup );

  mShowSpecialsFromCal = new TQCheckBox( i18n( "Show special occasions" ), mCalendarGroup );

  mContactGroup = new TQButtonGroup( 1, Horizontal, i18n( "Special Dates From Contact List" ), this );

  mShowBirthdaysFromKAB = new TQCheckBox( i18n( "Show birthdays" ), mContactGroup );
  mShowAnniversariesFromKAB = new TQCheckBox( i18n( "Show anniversaries" ), mContactGroup );

  layout->addWidget( mCalendarGroup, 1, 0 );
  layout->addWidget( mContactGroup, 1, 1 );

  layout->setRowStretch( 2, 1 );
}

void KCMSDSummary::load()
{
  KConfig config( "kcmsdsummaryrc" );

  config.setGroup( "Days" );
  int days = config.readNumEntry( "DaysToShow", 7 );
  if ( days == 1 )
    mDaysGroup->setButton( 0 );
  else if ( days == 5 )
    mDaysGroup->setButton( 1 );
  else if ( days == 7 )
    mDaysGroup->setButton( 2 );
  else if ( days == 31 )
    mDaysGroup->setButton( 3 );
  else {
    mDaysGroup->setButton( 4 );
    mCustomDays->setValue( days );
    mCustomDays->setEnabled( true );
  }

  config.setGroup( "EventTypes" );

  mShowBirthdaysFromKAB->
    setChecked( config.readBoolEntry( "ShowBirthdaysFromContacts", true ) );
  mShowBirthdaysFromCal->
    setChecked( config.readBoolEntry( "ShowBirthdaysFromCalendar", true ) );

  mShowAnniversariesFromKAB->
    setChecked( config.readBoolEntry( "ShowAnniversariesFromContacts", true ) );
  mShowAnniversariesFromCal->
    setChecked( config.readBoolEntry( "ShowAnniversariesFromCalendar", true ) );

  mShowHolidays->
    setChecked( config.readBoolEntry( "ShowHolidays", true ) );

  mShowSpecialsFromCal->
    setChecked( config.readBoolEntry( "ShowSpecialsFromCalendar", true ) );

  emit changed( false );
}

void KCMSDSummary::save()
{
  KConfig config( "kcmsdsummaryrc" );

  config.setGroup( "Days" );

  int days;
  switch ( mDaysGroup->selectedId() ) {
    case 0: days = 1; break;
    case 1: days = 5; break;
    case 2: days = 7; break;
    case 3: days = 31; break;
    case 4:
    default: days = mCustomDays->value(); break;
  }
  config.writeEntry( "DaysToShow", days );

  config.setGroup( "EventTypes" );

  config.writeEntry( "ShowBirthdaysFromContacts",
                     mShowBirthdaysFromKAB->isChecked() );
  config.writeEntry( "ShowBirthdaysFromCalendar",
                     mShowBirthdaysFromCal->isChecked() );

  config.writeEntry( "ShowAnniversariesFromContacts",
                     mShowAnniversariesFromKAB->isChecked() );
  config.writeEntry( "ShowAnniversariesFromCalendar",
                     mShowAnniversariesFromCal->isChecked() );

  config.writeEntry( "ShowHolidays",
                     mShowHolidays->isChecked() );

  config.writeEntry( "ShowSpecialsFromCalendar",
                     mShowSpecialsFromCal->isChecked() );

  config.sync();

  emit changed( false );
}

void KCMSDSummary::defaults()
{
  mDaysGroup->setButton( 7 );
  mShowBirthdaysFromKAB->setChecked( true );
  mShowBirthdaysFromCal->setChecked( true );
  mShowAnniversariesFromKAB->setChecked( true );
  mShowAnniversariesFromCal->setChecked( true );
  mShowHolidays->setChecked( true );
  mShowSpecialsFromCal->setChecked( true );

  emit changed( true );
}

const KAboutData* KCMSDSummary::aboutData() const
{
  KAboutData *about = new KAboutData( I18N_NOOP( "kcmsdsummary" ),
                                      I18N_NOOP( "Special Dates Configuration Dialog" ),
                                      0, 0, KAboutData::License_GPL,
                                      I18N_NOOP( "(c) 2004 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );
  about->addAuthor( "Allen Winter", 0, "winter@kde.org" );

  return about;
}

#include "kcmsdsummary.moc"
