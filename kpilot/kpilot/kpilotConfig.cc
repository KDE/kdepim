/* kpilotConfig.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <stdlib.h>

#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>

#include <kstddirs.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <kglobalsettings.h>

#include "kpilotSettings.h"
#include "kpilotConfig.h"

static const char kpilotconfig_id[] =
	"$Id$";

// This is a number indicating what configuration version
// we're dealing with. Whenever new configuration options are
// added that make it imperative for the user to take a
// look at the configuration of KPilot (for example the
// skipDB setting really needs user attention) we can change
// (increase) this number.
//
//
/* static */ const uint KPilotConfig::ConfigurationVersion = 443;

/* static */ int KPilotConfig::getConfigVersion()
{
	FUNCTIONSETUP;

	uint version = KPilotSettings::configVersion();

	if (version < ConfigurationVersion)
	{
		kdWarning() << k_funcinfo <<
			": Config file has old version " << version << endl;
	}
	else
	{
#ifdef DEBUG
		DEBUGDB << fname
			<< ": Config file has version " << version << endl;
#endif
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


void KPilotConfig::addDirtyDatabase(QString db)
{
	FUNCTIONSETUP;
	QStringList l(KPilotSettings::dirtyDatabases());
	if (!l.contains(db))
	{
		l.append(db);
		KPilotSettings::setDirtyDatabases(l);
	}
}


void KPilotConfig::addAppBlockChangedDatabase(QString db)
{
	QStringList l(KPilotSettings::appBlockChangedDatabases());
	if (!l.contains(db))
	{
		l.append(db);
		KPilotSettings::setAppBlockChangedDatabases(l);
	}
}

void KPilotConfig::addFlagsChangedDatabase(QString db)
{
	QStringList l(KPilotSettings::flagsChangedDatabases());
	if (!l.contains(db))
	{
		l.append(db);
		KPilotSettings::setFlagsChangedDatabases(l);
	}
}




/* static */ QString KPilotConfig::versionDetails(int fileversion, bool run)
{
	FUNCTIONSETUP;
	QString s = CSL1("<qt><p>");
	s += i18n("The configuration file is outdated.");
	s += ' ';
	s += i18n("The configuration file has version %1, while KPilot "
		"needs version %2.").arg(fileversion).arg(ConfigurationVersion);
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

static void update440()
{
	// Try to update conduit list
	{
	QStringList conduits( KPilotSettings::installedConduits() );
	KConfig*c = KPilotSettings::self()->config();
///	c->resetGroup();
	c->setGroup( QString::null );
	bool installFiles = c->readBoolEntry("SyncFiles",true);
	if (installFiles) conduits.append( CSL1("internal_fileinstall") );
	c->deleteEntry("SyncFiles");
	KPilotSettings::setInstalledConduits(conduits);
	c->sync();
	if (installFiles)
		KMessageBox::information(0L,
			i18n("The settings for the file installer have been moved to the "
				"conduits configuration. Check the installed "
				"conduits list."),
			i18n("Settings Updated"));

	}

	// Check if individual conduits have conflict settings?

	// Search for old conduit libraries.
	{
	QStringList foundlibs ;
	static const char *oldconduits[] = { "null", "address", "doc",
		"knotes", "sysinfo", "time", "todo", "vcal", 0L } ;
	const char **s = oldconduits;
	while (*s)
	{
		QString libname = CSL1("kde3/lib%1conduit.so").arg(*s);
		QString foundlib = ::locate("lib",libname);
		if (!foundlib.isEmpty())
		{
			foundlibs.append(foundlib);
		}
		s++;
	}

	if (!foundlibs.isEmpty())
		KMessageBox::informationList(0L,
			i18n("<qt>The following old conduits were found on "
				"your system. It is a good idea to remove "
				"them and the associated <tt>.la</tt> "
				"and <tt>.so.0</tt> files.</qt>"),
			foundlibs,
			i18n("Old Conduits Found"));
	}
}

static void update443()
{
	FUNCTIONSETUP;

	QStringList skip = KPilotSettings::skipBackupDB();
	QStringList fixSkip;
	bool fixedSome = false;
#ifdef DEBUG
	DEBUGKPILOT << fname << ": Skip databases are: "
		<< skip.join(",") << endl;
#endif

	for (QStringList::const_iterator i = skip.begin(); i!=skip.end(); ++i)
	{
		if ((*i).length()==4)
		{
			fixSkip.append(CSL1("[%1]").arg(*i));
			fixedSome = true;
		}
		else
		{
			fixSkip.append(*i);
		}
	}

	if (fixedSome)
	{
		KMessageBox::informationList(0L,
			i18n("<qt>The no backup databases listed in your configuration file "
				"have been adjusted to the new format. Database creator IDs "
				"have been changed to use square brackets []."),
			fixSkip,
			i18n("No Backup Databases Updated"));
	}
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
		res = KMessageBox::questionYesNoCancel(0L,
			i18n("KPilot is not configured for use. You may use "
				"the configuration wizard or the normal configure dialog "
				"to configure KPilot."),
			i18n("Not Configured"),
			i18n("Use &Wizard"),
			i18n("Use &Dialog"));
		if (res == KMessageBox::Yes) return WizardAndContinue;
		if (res == KMessageBox::No) return ConfigureAndContinue;

		return Cancel;
	}

	res = KMessageBox::warningContinueCancel(0L,
		i18n("The configuration file for KPilot is out-of "
			"date. KPilot can update some parts of the "
			"configuration automatically. Do you wish to "
			"continue?"),
		i18n("Configuration File Out-of Date"));
	if (res!=KMessageBox::Continue) return Cancel;

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Updating from "
		<< fileVersion << " to " << ConfigurationVersion << endl;
#endif

	if (fileVersion < 440) update440();
	if (fileVersion < 443) update443();

	KPilotConfig::updateConfigVersion();
	KPilotSettings::writeConfig();
	return ConfigureAndContinue;
}

void KPilotConfig::sync()
{
	KPilotSettings::self()->config()->sync();
}
