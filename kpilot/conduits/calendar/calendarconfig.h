#ifndef CALENDARCONFIG_H
#define CALENDARCONFIG_H
/* calendarconfig.h			KPilot
**
** Copyright (C) 2008 by Bertjan Broeksema <b.broeksema@kdemail.net>
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "plugin.h"
#include "ui_settingswidget.h"

class AkonadiSetupWidget;

class CalendarConfig : public ConduitConfigBase
{
	Q_OBJECT
public:
	explicit CalendarConfig( QWidget*, const QVariantList& = QVariantList() );

	/* virtual */~CalendarConfig();

	/**
	 * Load or save the config widget's settings in the given
	 * KConfig object; leave the group unchanged. load() and
	 * commit() should both call unmodified() to indicate that
	 * the current settings match the on-disk ones.
	 */
	/* virtual */ void load();

  /**
	 * Load or save the config widget's settings in the given
	 * KConfig object; leave the group unchanged. load() and
	 * commit() should both call unmodified() to indicate that
	 * the current settings match the on-disk ones.
	 */
	/* virtual */ void commit();

private:
	AkonadiSetupWidget* fAkonadiWidget;
	QGridLayout* fLayout;
	Ui::SettingsWidget fUi;

};

#endif
