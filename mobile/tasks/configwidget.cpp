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

using namespace CalendarSupport;

ConfigWidget::ConfigWidget( QWidget *parent )
  : QWidget( parent )
{
  Ui_ConfigWidget ui;
  ui.setupUi( this );

  mManager = new KConfigDialogManager( this, Settings::self() );
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
}

void ConfigWidget::save()
{
  mManager->updateSettings();
  saveToExternalSettings();
}

void ConfigWidget::loadFromExternalSettings()
{
  // Date and Time
  Settings::self()->setRemindersForNewTodos( KCalPrefs::instance()->defaultTodoReminders() );
  Settings::self()->setReminderDefaultTime( KCalPrefs::instance()->reminderTime() );
  Settings::self()->setReminderDefaultUnit( KCalPrefs::instance()->reminderTimeUnits() );

  // Views
  Settings::self()->setTodoDueColor( mViewPrefs->todoDueTodayColor() );
  Settings::self()->setTodoOverdueColor( mViewPrefs->todoOverdueColor() );

}

void ConfigWidget::saveToExternalSettings()
{
  // Date and Time
  KCalPrefs::instance()->setDefaultTodoReminders( Settings::self()->remindersForNewTodos() );
  KCalPrefs::instance()->setReminderTime( Settings::self()->reminderDefaultTime() );
  KCalPrefs::instance()->setReminderTimeUnits( Settings::self()->reminderDefaultUnit() );

  // Views
  mViewPrefs->setTodoDueTodayColor( Settings::self()->todoDueColor() );
  mViewPrefs->setTodoOverdueColor( Settings::self()->todoOverdueColor() );

  Settings::self()->save();
  KCalPrefs::instance()->writeConfig();
  mViewPrefs->writeConfig();
}

DeclarativeConfigWidget::DeclarativeConfigWidget( QGraphicsItem *parent )
  : DeclarativeWidgetBase<ConfigWidget, MainView, &MainView::setConfigWidget>( parent )
{
}

DeclarativeConfigWidget::~DeclarativeConfigWidget()
{
}

void DeclarativeConfigWidget::setPreferences( const EventViews::PrefsPtr &preferences )
{
  widget()->setPreferences( preferences );
}

void DeclarativeConfigWidget::load()
{
  widget()->load();
}

void DeclarativeConfigWidget::save()
{
  widget()->save();
}

