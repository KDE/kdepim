/* kpilotConfig.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
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
/* static */ const int KPilotConfig::ConfigurationVersion = 440;

/* static */ int KPilotConfig::getConfigVersion(KConfig * config)
{
	FUNCTIONSETUP;

	if (!config)
		return 0;
	else
		return getConfigVersion(*config);
	/* NOTREACHED */
	(void) kpilotconfig_id;
}

/* static */ int KPilotConfig::getConfigVersion(KConfig & config)
{
	FUNCTIONSETUP;

	config.setGroup(QString::null);
	int version = config.readNumEntry("Configured", 0);

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

	KPilotConfigSettings & config = getConfig();
	config.setVersion(ConfigurationVersion);
}

/* static */ QString KPilotConfig::getDefaultDBPath()
{
	FUNCTIONSETUP;
	QString lastUser = getConfig().getUser();
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

static KPilotConfigSettings *theconfig = 0L;

KPilotConfigSettings & KPilotConfig::getConfig()
{
	FUNCTIONSETUP;

	if (theconfig)
	{
		return *theconfig;
	}

	/**
	* This causes a crash if no instance has been created
	* yet. A standard KDE error message reports this fact.
	* It is a grave programming error, so we will let that
	* stand.
	*/
	QString existingConfig =
		KGlobal::dirs()->findResource("config", CSL1("kpilotrc"));


	if (existingConfig.isNull())
	{
#ifdef DEBUG
		DEBUGDB << fname << ": Making a new config file" << endl;
#endif
		KSimpleConfig *c = new KSimpleConfig(CSL1("kpilotrc"), false);

		c->writeEntry("Configured", ConfigurationVersion);
		c->writeEntry("NextUniqueID", 61440);
		c->sync();
		delete c;

		theconfig = new KPilotConfigSettings(CSL1("kpilotrc"));
	}
	else
	{
#ifdef DEBUG
		DEBUGDB << fname
			<< ": Re-using existing config file "
			<< existingConfig << endl;
#endif

		theconfig = new KPilotConfigSettings(existingConfig);
	}

	if (theconfig == 0L)
	{
		kdWarning() << k_funcinfo
			<< ": No configuration was found." << endl;
	}

	return *theconfig;
}

static QFont *thefont = 0L;

/* static */ const QFont & KPilotConfig::fixed()
{
	FUNCTIONSETUP;

	if (!thefont)
		thefont = new QFont(KGlobalSettings::fixedFont());

	return *thefont;
}

KPilotConfigSettings::KPilotConfigSettings(const QString & f, bool b) :
	KSimpleConfig(f, b)
{
	FUNCTIONSETUP;
}

KPilotConfigSettings::~KPilotConfigSettings()
{
	FUNCTIONSETUP;
}

#define IntProperty_(a,key,defl,m) \
	int KPilotConfigSettings::get##a() const { \
	int i = readNumEntry(key,defl); \
	if ((i<0) || (i>m)) i=0; \
	return i; } \
	void KPilotConfigSettings::set##a(int i) { \
	if ((i<0) || (i>m)) i=0; writeEntry(key,i); }

IntProperty_(PilotSpeed, "PilotSpeed", 0, 4)
IntProperty_(SyncType, "SyncType", 0, 4)
IntProperty_(ConflictResolution, "ConflictResolution", 0,4)
IntProperty_(AddressDisplayMode, "AddressDisplay", 0, 1)
IntProperty_(Version, "Configured", 0, 100000)
IntProperty_(Debug, "Debug", 0, 1023)

#define BoolProperty_(a,key,defl) \
	bool KPilotConfigSettings::get##a() const { \
	bool b = readBoolEntry(key,defl); return b; } \
	void KPilotConfigSettings::set##a(bool b) { \
	writeEntry(key,b); }

BoolProperty_(StartDaemonAtLogin, "StartDaemonAtLogin", true)
BoolProperty_(DockDaemon, "DockDaemon", true)
BoolProperty_(KillDaemonOnExit, "StopDaemonAtExit", false)
BoolProperty_(QuitAfterSync, "QuitAfterSync", false);
BoolProperty_(FullSyncOnPCChange, "FullSyncOnPCChange", true)
// BoolProperty_(SyncFiles, "SyncFiles", true)
// BoolProperty_(SyncWithKMail, "SyncWithKMail", false)
BoolProperty_(ShowSecrets, "ShowSecrets", false)
BoolProperty_(UseKeyField, "UseKeyField", false)
BoolProperty_(InternalEditors, "InternalEditorsWritable", true)


#define StringProperty_(a,key,defl) \
	QString KPilotConfigSettings::get##a() const { \
	QString s = readEntry(key,defl); return s; } \
	void  KPilotConfigSettings::set##a(const QString &s) { \
	writeEntry(key,s); }


StringProperty_(PilotDevice, "PilotDevice", CSL1("/dev/pilot"))
StringProperty_(Encoding, "Encoding", QString::null)

StringProperty_(User, "UserName", QString::null)
StringProperty_(BackupOnly, "BackupForSync", CSL1("Arng,PmDB,lnch"))
StringProperty_(Skip, "SkipSync", CSL1("AvGo"))


KPilotConfigSettings & KPilotConfigSettings::setAddressGroup()
{
	FUNCTIONSETUP;
	setGroup("Address Widget");
	return *this;
}

KPilotConfigSettings & KPilotConfigSettings::setConduitGroup()
{
	FUNCTIONSETUP;
	setGroup("Conduit Names");
	return *this;
}

KPilotConfigSettings & KPilotConfigSettings::setDatabaseGroup()
{
	FUNCTIONSETUP;
	setGroup("Database Names");
	return *this;
}

QStringList KPilotConfigSettings::getInstalledConduits()
{
	FUNCTIONSETUP;
	KConfigGroupSaver cgs(this,"Conduit Names");
	return readListEntry("InstalledConduits");
}

void KPilotConfigSettings::setInstalledConduits(const QStringList & l)
{
	FUNCTIONSETUP;
	KConfigGroupSaver cgs(this,"Conduit Names");
	writeEntry("InstalledConduits", l);
}

QStringList KPilotConfigSettings::getDirtyDatabases()
{
	FUNCTIONSETUP;
	KConfigGroupSaver cgs(this,"Internal Editors");
	return readListEntry("Changed Databases");
}

void KPilotConfigSettings::setDirtyDatabases(const QStringList &l)
{
	FUNCTIONSETUP;
	KConfigGroupSaver cgs(this,"Internal Editors");
	writeEntry("Changed Databases", l);
}

void KPilotConfigSettings::addDirtyDatabase(QString db)
{
	FUNCTIONSETUP;
	QStringList l(getDirtyDatabases());
	if (!l.contains(db))
	{
		l.append(db);
		setDirtyDatabases(l);
	}
}


QStringList KPilotConfigSettings::getAppBlockChangedDatabases()
{
	FUNCTIONSETUP;
	KConfigGroupSaver cgs(this,"Internal Editors");
	return readListEntry("AppBlock Changed");
}

void KPilotConfigSettings::setAppBlockChangedDatabases(const QStringList &l)
{
	FUNCTIONSETUP;
	KConfigGroupSaver cgs(this,"Internal Editors");
	writeEntry("AppBlock Changed", l);
}

void KPilotConfigSettings::addAppBlockChangedDatabase(QString db)
{
	QStringList l(getAppBlockChangedDatabases());
	if (!l.contains(db))
	{
		l.append(db);
		setAppBlockChangedDatabases(l);
	}
}


QStringList KPilotConfigSettings::getFlagsChangedDatabases()
{
	FUNCTIONSETUP;
	KConfigGroupSaver cgs(this,"Internal Editors");
	return readListEntry("Flags Changed");
}

void KPilotConfigSettings::setFlagsChangedDatabases(const QStringList &l)
{
	FUNCTIONSETUP;
	KConfigGroupSaver cgs(this,"Internal Editors");
	writeEntry("Flags Changed", l);
}

void KPilotConfigSettings::addFlagsChangedDatabase(QString db)
{
	QStringList l(getFlagsChangedDatabases());
	if (!l.contains(db))
	{
		l.append(db);
		setFlagsChangedDatabases(l);
	}
}


void KPilotConfigSettings::setDatabaseConduit(const QString & database,
	const QString & conduit)
{
	FUNCTIONSETUP;
	setDatabaseGroup();
	writeEntry(database, conduit);
}


/* static */ QString KPilotConfig::versionDetails(int fileversion, bool run)
{
	FUNCTIONSETUP;
	QString s = CSL1("<qt><p>");
	s = i18n("The configuration file is outdated.");
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


/* static */ void KPilotConfig::interactiveUpdate()
{
	FUNCTIONSETUP;
	KPilotConfigSettings &c = KPilotConfig::getConfig();
	int fileversion = c.getVersion();
	int res = 0;

	res = KMessageBox::warningContinueCancel(0L,
		i18n("The configuration file for KPilot is out-of "
			"date. KPilot can update some parts of the "
			"configuration automatically. Do you wish to "
			"continue?"),
		i18n("Configuration File Out-of Date"));
	if (res!=KMessageBox::Continue) return;

	// Try to update conduit list
	{
	QStringList conduits( c.getInstalledConduits() );
	c.resetGroup();
	bool useKroupware = c.readBoolEntry("SyncWithKMail",false);
	bool installFiles = c.readBoolEntry("SyncFiles",true);
	if (useKroupware) conduits.append( CSL1("internal_kroupware") );
	if (installFiles) conduits.append( CSL1("internal_fileinstall") );
	c.deleteEntry("SyncWithKMail");
	c.deleteEntry("SyncFiles");
	c.setInstalledConduits(conduits);
	c.sync();
	if (useKroupware || installFiles)
		KMessageBox::information(0L,
			i18n("The settings for Kroupware syncing with KMail "
				"and the file installer have been moved to the "
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

	KPilotConfig::updateConfigVersion();
	c.sync();
}
