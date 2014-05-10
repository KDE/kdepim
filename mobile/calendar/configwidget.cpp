/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Tobias Koenig <tobias.koenig@kdab.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "configwidget.h"

#include "settings.h"
#include "stylesheetloader.h"
#include "ui_configwidget.h"

#include <calendarsupport/kcalprefs.h>
#include <kconfigdialogmanager.h>
#include <KHolidays/kholidays/holidays.h>
#include <qplatformdefs.h>

#include <KGlobal>
#include <KLocale>

using namespace CalendarSupport;

ConfigWidget::ConfigWidget( QWidget *parent )
  : QWidget( parent )
{
  mUi = new Ui_ConfigWidget;
  mUi->setupUi( this );

  mUi->kcfg_DayBegins->setProperty( "kcfg_property", QByteArray( "dateTime" ) );
  mUi->kcfg_DailyStartingHour->setProperty( "kcfg_property", QByteArray( "dateTime" ) );
  mUi->kcfg_DailyEndingHour->setProperty( "kcfg_property", QByteArray( "dateTime" ) );
  mUi->kcfg_DefaultAppointmentTime->setProperty( "kcfg_property", QByteArray( "dateTime" ) );

  mUi->kcfg_AgendaViewColorUsage->addItem( i18n( "Category inside, calendar outside" ) );
  mUi->kcfg_AgendaViewColorUsage->addItem( i18n( "Calendar inside, category outside" ) );
  mUi->kcfg_AgendaViewColorUsage->addItem( i18n( "Only category" ) );
  mUi->kcfg_AgendaViewColorUsage->addItem( i18n( "Only calendar" ) );

  mUi->kcfg_MonthViewColorUsage->addItem( i18n( "Category inside, calendar outside" ) );
  mUi->kcfg_MonthViewColorUsage->addItem( i18n( "Calendar inside, category outside" ) );
  mUi->kcfg_MonthViewColorUsage->addItem( i18n( "Only category" ) );
  mUi->kcfg_MonthViewColorUsage->addItem( i18n( "Only calendar" ) );

  mHolidayCombo = mUi->kcfg_HolidayRegion;
  mWorkDays << mUi->workingPeriodMonday;
  mWorkDays << mUi->workingPeriodTuesday;
  mWorkDays << mUi->workingPeriodWednesday;
  mWorkDays << mUi->workingPeriodThursday;
  mWorkDays << mUi->workingPeriodFriday;
  mWorkDays << mUi->workingPeriodSaturday;
  mWorkDays << mUi->workingPeriodSunday;

  mManager = new KConfigDialogManager( this, Settings::self() );

  // fill holiday combobox
  const QStringList regions = KHolidays::HolidayRegion::regionCodes();
  QMap<QString, QString> regionsMap;

  foreach ( const QString & regionCode, regions ) {
    const QString name = KHolidays::HolidayRegion::name( regionCode );
    const QString languageName = KLocale::global()->languageCodeToName( KHolidays::HolidayRegion::languageCode( regionCode ) );

    QString label;
    if ( languageName.isEmpty() ) {
      label = name;
    } else {
      label = i18nc( "Holday region, region language", "%1 (%2)", name, languageName );
    }

    regionsMap.insert( label, regionCode );
  }

  mHolidayCombo->addItem( i18nc( "No holiday region", "None"), QString() );
  QMapIterator<QString, QString> it( regionsMap );
  while ( it.hasNext() ) {
    it.next();
    mHolidayCombo->addItem( it.key(), it.value() );
  }

  mUi->kcfg_DayBegins->installEventFilter( this );
  mUi->kcfg_DailyStartingHour->installEventFilter( this );
  mUi->kcfg_DailyEndingHour->installEventFilter( this );
  mUi->kcfg_DefaultAppointmentTime->installEventFilter( this );

  connect( this, SIGNAL(dayBeginsFocus(QObject*)), SLOT(showClock(QObject*)) );
  connect( this, SIGNAL(dailyStartingHourFocus(QObject*)), SLOT(showClock(QObject*)) );
  connect( this, SIGNAL(dailyEndingHourFocus(QObject*)), SLOT(showClock(QObject*)) );
  connect( this, SIGNAL(defaultAppointmentTimeFocus(QObject*)), SLOT(showClock(QObject*)) );
}

ConfigWidget::~ConfigWidget()
{
  delete mUi;
}

void ConfigWidget::setPreferences( const EventViews::PrefsPtr &preferences )
{
  mViewPrefs = preferences;
  load();
}

void ConfigWidget::load()
{
  loadFromExternalSettings();
  mManager->updateWidgets();

  if ( !Settings::self()->holidayRegion().isEmpty() )
    mHolidayCombo->setCurrentIndex( mHolidayCombo->findData( Settings::self()->holidayRegion() ) );
  else
    mHolidayCombo->setCurrentIndex( 0 );

  for ( int i = 0; i < 7; ++i ) {
    mWorkDays[ i ]->setChecked( ( 1 << i ) & ( Settings::self()->workWeekMask() ) );
  }
}

void ConfigWidget::save()
{
  mManager->updateSettings();

  Settings::self()->setHolidayRegion( mHolidayCombo->itemData( mHolidayCombo->currentIndex() ).toString() );

  int mask = 0;
  for ( int i = 0; i < 7; ++i ) {
    if ( mWorkDays[ i ]->isChecked() ) {
      mask = mask | ( 1 << i );
    }
  }

  Settings::self()->setWorkWeekMask( mask );

  saveToExternalSettings();
}

void ConfigWidget::loadFromExternalSettings()
{
  // Date and Time
  Settings::self()->setHolidayRegion( KCalPrefs::instance()->holidays() );
  Settings::self()->setExcludeHolidays( KCalPrefs::instance()->excludeHolidays() );
  Settings::self()->setWorkWeekMask( KCalPrefs::instance()->workWeekMask() );
  Settings::self()->setDefaultAppointmentTime( KCalPrefs::instance()->startTime() );
  Settings::self()->setDefaultAppointmentDuration( KCalPrefs::instance()->defaultDuration() );
  Settings::self()->setRemindersForNewEvents( KCalPrefs::instance()->defaultEventReminders() );
  Settings::self()->setReminderDefaultTime( KCalPrefs::instance()->reminderTime() );
  Settings::self()->setReminderDefaultUnit( KCalPrefs::instance()->reminderTimeUnits() );
  Settings::self()->setDayBegins( mViewPrefs->dayBegins() );
  Settings::self()->setDailyStartingHour( mViewPrefs->workingHoursStart() );
  Settings::self()->setDailyEndingHour( mViewPrefs->workingHoursEnd() );

  // Views
  Settings::self()->setTodosUseCategoryColors( mViewPrefs->todosUseCategoryColors() );
  Settings::self()->setHourSize( mViewPrefs->hourSize() );
  Settings::self()->setShowIconsInAgendaView( mViewPrefs->enableAgendaItemIcons() );
  Settings::self()->setShowTodosInAgendaView( mViewPrefs->showTodosAgendaView() );
  Settings::self()->setShowCurrentTimeLine( mViewPrefs->marcusBainsEnabled() );
  Settings::self()->setAgendaViewColorUsage( mViewPrefs->agendaViewColors() );
  Settings::self()->setColorBusyDaysInAgendaView( mViewPrefs->colorAgendaBusyDays() );
  Settings::self()->setShowTodosInMonthView( mViewPrefs->showTodosMonthView() );
  Settings::self()->setMonthViewColorUsage( mViewPrefs->monthViewColors() );
  Settings::self()->setColorBusyDaysInMonthView( mViewPrefs->colorMonthBusyDays() );

  // Colors
  Settings::self()->setHolidayColor( mViewPrefs->holidayColor() );
  Settings::self()->setAgendaViewBackgroundColor( mViewPrefs->agendaViewBackgroundColor() );
  Settings::self()->setBusyDaysBackgroundColor( mViewPrefs->viewBgBusyColor() );
  Settings::self()->setAgendaViewTimeLineColor( mViewPrefs->agendaMarcusBainsLineLineColor() );
  Settings::self()->setWorkingHoursColor( mViewPrefs->workingHoursColor() );
  Settings::self()->setTodoDueColor( mViewPrefs->todoDueTodayColor() );
  Settings::self()->setTodoOverdueColor( mViewPrefs->todoOverdueColor() );

  // Group scheduling
  Settings::self()->setUseGroupwareCommunication( KCalPrefs::instance()->useGroupwareCommunication() );
}

void ConfigWidget::saveToExternalSettings()
{
  // Date and Time
  KCalPrefs::instance()->setHolidays( Settings::self()->holidayRegion() );
  KCalPrefs::instance()->setExcludeHolidays( Settings::self()->excludeHolidays() );
  KCalPrefs::instance()->setWorkWeekMask( Settings::self()->workWeekMask() );
  KCalPrefs::instance()->setStartTime( Settings::self()->defaultAppointmentTime() );
  KCalPrefs::instance()->setDefaultDuration( Settings::self()->defaultAppointmentDuration() );
  KCalPrefs::instance()->setDefaultEventReminders( Settings::self()->remindersForNewEvents() );
  KCalPrefs::instance()->setReminderTime( Settings::self()->reminderDefaultTime() );
  KCalPrefs::instance()->setReminderTimeUnits( Settings::self()->reminderDefaultUnit() );
  mViewPrefs->setDayBegins( Settings::self()->dayBegins() );
  mViewPrefs->setWorkingHoursStart( Settings::self()->dailyStartingHour() );
  mViewPrefs->setWorkingHoursEnd( Settings::self()->dailyEndingHour() );

  // Views
  mViewPrefs->setTodosUseCategoryColors( Settings::self()->todosUseCategoryColors() );
  mViewPrefs->setHourSize( Settings::self()->hourSize() );
  mViewPrefs->setEnableAgendaItemIcons( Settings::self()->showIconsInAgendaView() );
  mViewPrefs->setShowTodosAgendaView( Settings::self()->showTodosInAgendaView() );
  mViewPrefs->setMarcusBainsEnabled( Settings::self()->showCurrentTimeLine() );
  mViewPrefs->setAgendaViewColors( Settings::self()->agendaViewColorUsage() );
  mViewPrefs->setColorAgendaBusyDays( Settings::self()->colorBusyDaysInAgendaView() );
  mViewPrefs->setShowTodosMonthView( Settings::self()->showTodosInMonthView() );
  mViewPrefs->setMonthViewColors( Settings::self()->monthViewColorUsage() );
  mViewPrefs->setColorMonthBusyDays( Settings::self()->colorBusyDaysInMonthView() );

  // Colors
  mViewPrefs->setHolidayColor( Settings::self()->holidayColor() );
  mViewPrefs->setAgendaViewBackgroundColor( Settings::self()->agendaViewBackgroundColor() );
  mViewPrefs->setViewBgBusyColor( Settings::self()->busyDaysBackgroundColor() );
  mViewPrefs->setAgendaMarcusBainsLineLineColor( Settings::self()->agendaViewTimeLineColor() );
  mViewPrefs->setWorkingHoursColor( Settings::self()->workingHoursColor() );
  mViewPrefs->setTodoDueTodayColor( Settings::self()->todoDueColor() );
  mViewPrefs->setTodoOverdueColor( Settings::self()->todoOverdueColor() );

  // Group scheduling
  KCalPrefs::instance()->setUseGroupwareCommunication( Settings::self()->useGroupwareCommunication() );

  Settings::self()->save();
  KCalPrefs::instance()->save();
  mViewPrefs->writeConfig();
}

void ConfigWidget::showClock( QObject *object )
{
  setFocus();
  mFocusedTimeWidget = qobject_cast<KTimeComboBox*>( object );
  if ( !mFocusedTimeWidget )
    return;

  const QTime time = mFocusedTimeWidget->time();
  emit showClockWidget( time.hour(), time.minute() );
}

void ConfigWidget::setNewTime( int hour, int minute )
{
  if ( mFocusedTimeWidget == 0 )
    return;

  mFocusedTimeWidget->setTime( QTime( hour, minute ) );
}

bool ConfigWidget::eventFilter( QObject *object, QEvent *event )
{
  if ( event->type() == QEvent::FocusIn ) {
    if ( object == mUi->kcfg_DayBegins ) {
      emit dayBeginsFocus( object );
    } else if ( object == mUi->kcfg_DailyStartingHour ) {
      emit dailyStartingHourFocus( object );
    } else if ( object == mUi->kcfg_DailyEndingHour ) {
      emit dailyEndingHourFocus( object );
    } else if ( object == mUi->kcfg_DefaultAppointmentTime ) {
      emit defaultAppointmentTimeFocus( object );
    }

    return true;
  } else {
    // standard event processing
    return QObject::eventFilter( object, event );
  }
}

DeclarativeConfigWidget::DeclarativeConfigWidget( QGraphicsItem *parent )
  : DeclarativeWidgetBase< ConfigWidget, MainView, &MainView::setConfigWidget>( parent )
{
  connect( this, SIGNAL(configChanged()), widget(), SIGNAL(configChanged()) );
  connect( widget(), SIGNAL(showClockWidget(int,int)), this, SIGNAL(showClockWidget(int,int)) );
}

DeclarativeConfigWidget::~DeclarativeConfigWidget()
{
}

void DeclarativeConfigWidget::load()
{
  widget()->load();
}

void DeclarativeConfigWidget::save()
{
  widget()->save();
  emit configChanged();
}

void DeclarativeConfigWidget::setNewTime( int hour, int minute )
{
  widget()->setNewTime( hour, minute );
}

