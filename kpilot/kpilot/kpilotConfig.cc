/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This is all of KPilot's config-handling stuff.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
#include "kpilotConfig.h"

#include <stdlib.h>

#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>

#include <kstandarddirs.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <kglobalsettings.h>

#include "options.h"
#include "kpilotSettings.h"


// This is a number indicating what configuration version
// we're dealing with. Whenever new configuration options are
// added that make it imperative for the user to take a
// look at the configuration of KPilot (for example the
// skipDB setting really needs user attention) we can change
// (increase) this number.
//
//
/* static */ const uint KPilotConfig::ConfigurationVersion = 520;

/* static */ int KPilotConfig::getConfigVersion()
{
	FUNCTIONSETUP;

	uint version = KPilotSettings::configVersion();

	if (version < ConfigurationVersion)
	{
		WARNINGKPILOT << "Config file has old version" << version;
	}
	else
	{
		DEBUGKPILOT << "Config file has version " << version;
	}

	return version;
}

/* static */ void KPilotConfig::updateConfigVersion()
{
	FUNCTIONSETUP;
	KPilotSettings::setConfigVersion( ConfigurationVersion );
}

/* static */ QString KPilotConfig::getDefaultDBPath()
{
	FUNCTIONSETUP;
	QString lastUser = KPilotSettings::userName();
	QString dbsubpath = CSL1("kpilot/DBBackup/");
	QString defaultDBPath = KGlobal::dirs()->
		saveLocation("data", dbsubpath + lastUser + CSL1("/"));
	return defaultDBPath;
}

/* static */ int KPilotConfig::getDebugLevel(KCmdLineArgs *p)
{
	FUNCTIONSETUP;

	if (p)
	{
		if (p->isSet("debug"))
		{
			debug_level = p->getOption("debug").toInt();
		}
	}
	if (debug_level < 0)
	{
		debug_level = 0;
	}

	return debug_level;
}

static QFont *thefont = 0L;

/* static */ const QFont & KPilotConfig::fixed()
{
	FUNCTIONSETUP;

	if (!thefont)
		thefont = new QFont(KGlobalSettings::fixedFont());

	return *thefont;
}

/* static */ QString KPilotConfig::versionDetails(int fileversion, bool run)
{
	FUNCTIONSETUP;
	QString s = CSL1("<qt><p>");
	s += i18n("The configuration file is outdated.");
	s += ' ';
	s += i18n("The configuration file has version %1, while KPilot "
		"needs version %2.",fileversion,ConfigurationVersion);
	if (run)
	{
		s += ' ';
		s += i18n("Please run KPilot and check the configuration carefully "
			"to update the file.");
	}
	s += CSL1("</p><p>");
	s += i18n("Important changes to watch for are:");
	s += ' ';
	if (fileversion < 440)
	{
		s += i18n("Renamed conduits, Kroupware and file installer have "
			"been made conduits as well.");
		s += ' ';
		s += i18n("Conflict resolution is now a global setting.");
		s += ' ';
	}
	if (fileversion < 443)
	{
		s += i18n("Changed format of no-backup databases.");
		s += ' ';
	}
	if (fileversion < 520)
	{
		s += i18n("Calendar, ToDo, and Contacts conduits are now using "
			  "KDE4's Akonadi server and require valid Akonadi "
			  "resources to sync.");
		s += ' ';
	}
	// Insert more recent additions here


	return s;
}

/* static */ void KPilotConfig::sorryVersionOutdated(int fileversion)
{
	FUNCTIONSETUP;
	KMessageBox::detailedSorry(0L,
		i18n("The configuration file for KPilot is out-of "
			"date. Please run KPilot to update it."),
		KPilotConfig::versionDetails(fileversion,true),
		i18n("Configuration File Out-of Date"));
}


/* static */ KPilotConfig::RunMode KPilotConfig::interactiveUpdate()
{
	FUNCTIONSETUP;

	int res = 0;
	unsigned int fileVersion = KPilotSettings::configVersion();
	// FIXME better config handling -> Move the config entries using kconf_update

	// It's OK if we're already at the required level.
	if (fileVersion >= KPilotConfig::ConfigurationVersion)
	{
		return Normal;
	}

	if (0 == fileVersion) // No config file at all
	{
		return ConfigureAndContinue;
	}

	res = KMessageBox::warningContinueCancel(0L,
		i18n("The configuration file for KPilot is out-of "
			"date. KPilot can update some parts of the "
			"configuration automatically. Do you wish to "
			"continue?"),
		i18n("Configuration File Out-of Date"));
	if (res!=KMessageBox::Continue)
	{
		return Cancel;
	}

	DEBUGKPILOT << "Updating from "
		<< fileVersion << " to " << ConfigurationVersion;

	KPilotConfig::updateConfigVersion();
	KPilotSettings::self()->writeConfig();
	return ConfigureAndContinue;
}

void KPilotConfig::sync()
{
	KPilotSettings::self()->config()->sync();
}
