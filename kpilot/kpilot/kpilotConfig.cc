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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <stdlib.h>

#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>

#ifndef _KLOCALE_H
#include <klocale.h>
#endif
#ifndef _KGLOBAL_H
#include <kglobal.h>
#endif
#ifndef _KSTDDIRS_H
#include <kstddirs.h>
#endif
#ifndef _KCONFIG_H
#include <kconfig.h>
#endif
#include <ksimpleconfig.h>

#ifdef DEBUG
#include <kcmdlineargs.h>
#endif

#ifndef _KPILOT_KPILOTCONFIG_H
#include "kpilotConfig.h"
#endif

static const char *kpilotconfig_id =
	"$Id$";

// This is a number indicating what configuration version
// we're dealing with. Whenever new configuration options are
// added that make it imperative for the user to take a
// look at the configuration of KPilot (for example the
// skipDB setting really needs user attention) we can change
// (increase) this number.
//
//
/* static */ const int KPilotConfig::ConfigurationVersion = 403;

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
	QString dbsubpath = "kpilot/DBBackup/";
	QString defaultDBPath = KGlobal::dirs()->
		saveLocation("data", dbsubpath + lastUser + "/");
	return defaultDBPath;
}

#ifndef DEBUG
/* static */ int KPilotConfig::getDebugLevel(KPilotConfigSettings &)
{
	return 0;
}

/* static */ int KPilotConfig::getDebugLevel(bool)
{
	return 0;
}
#else
/* static */ int KPilotConfig::getDebugLevel(KPilotConfigSettings & c)
{
	FUNCTIONSETUP;

	int d = c.getDebug();

	debug_level |= d;

	if (debug_level)
	{
		DEBUGKPILOT << fname
			<< ": Debug level set to " << debug_level << endl;
	}

	return debug_level;
}

/* static */ int KPilotConfig::getDebugLevel(bool useDebugId)
{
	FUNCTIONSETUP;

	KCmdLineArgs *p = KCmdLineArgs::parsedArgs(useDebugId ? "debug" : 0L);

	if (p)
	{
		if (p->isSet("debug"))
		{
			debug_level = atoi(p->getOption("debug"));
		}
	}

	getDebugLevel(getConfig());

	return debug_level;
}
#endif

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
		KGlobal::dirs()->findResource("config", "kpilotrc");


	if (existingConfig.isNull())
	{
#ifdef DEBUG
		DEBUGDB << fname << ": Making a new config file" << endl;
#endif
		KSimpleConfig *c = new KSimpleConfig("kpilotrc", false);

		c->writeEntry("Configured", ConfigurationVersion);
		c->writeEntry("NextUniqueID", 61440);
		c->sync();
		delete c;

		theconfig = new KPilotConfigSettings("kpilotrc");
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

	if (thefont)
	{
		return *thefont;
	}

	KConfig KDEGlobalConfig(QString::null);

	KDEGlobalConfig.setGroup("General");
	QString s = KDEGlobalConfig.readEntry("fixed");

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Creating font " << s << endl;
#endif

	thefont = new QFont(KDEGlobalConfig.readFontEntry("fixed"));

	if (!thefont)
	{
		kdError() << k_funcinfo
			<< ": **\n"
			<< ": ** No font was created! (Expect crash now)\n"
			<< ": **" << endl;
	}

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
	int KPilotConfigSettings::get##a(QComboBox *p) { \
	int i = readNumEntry(key,defl); \
	if ((i<0) || (i>m)) i=0; \
	if (p) p->setCurrentItem(i); \
	return i; } \
	void KPilotConfigSettings::set##a(QComboBox *p) { \
	set##a(p->currentItem()); } \
	void KPilotConfigSettings::set##a(int i) { \
	if ((i<0) || (i>m)) i=0; writeEntry(key,i); }

IntProperty_(PilotType, "PilotType", 0, 3)
IntProperty_(PilotSpeed, "PilotSpeed", 0, 4)
IntProperty_(AddressDisplayMode, "AddressDisplay", 0, 1)
IntProperty_(Version, "Configured", 0, 100000)
IntProperty_(Debug, "Debug", 0, 1023)

#define BoolProperty_(a,key,defl) \
	bool KPilotConfigSettings::get##a(QCheckBox *p) { \
	bool b = readBoolEntry(key,defl); if (p) p->setChecked(b); return b; } \
	void KPilotConfigSettings::set##a(QCheckBox *p) { \
	set##a(p->isChecked()); } \
	void KPilotConfigSettings::set##a(bool b) { \
	writeEntry(key,b); }
	BoolProperty_(DockDaemon, "DockDaemon", true)

BoolProperty_(KillDaemonOnExit, "StopDaemonAtExit", false)
BoolProperty_(StartDaemonAtLogin, "StartDaemonAtLogin", true)
BoolProperty_(ShowSecrets, "ShowSecrets", false)
BoolProperty_(SyncFiles, "SyncFiles", true)
BoolProperty_(SyncWithKMail, "SyncWithKMail", false)
BoolProperty_(UseKeyField, "UseKeyField", false)


#define StringProperty_(a,key,defl) \
	QString KPilotConfigSettings::get##a(QLineEdit *p) { \
	QString s = readEntry(key,defl); if (p) p->setText(s); return s; } \
	void  KPilotConfigSettings::set##a(QLineEdit *p) { \
	set##a(p->text()); } \
	void  KPilotConfigSettings::set##a(const QString &s) { \
	writeEntry(key,s); }


StringProperty_(PilotDevice, "PilotDevice", "/dev/pilot")
StringProperty_(Encoding, "Encoding", QString::null)


StringProperty_(User, "UserName", QString::null)
StringProperty_(BackupOnly, "BackupForSync", "Arng,PmDB,lnch")
StringProperty_(Skip, "SkipSync", "AvGo")


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

void KPilotConfigSettings::setDatabaseConduit(const QString & database,
	const QString & conduit)
{
	FUNCTIONSETUP;
	setDatabaseGroup();
	writeEntry(database, conduit);
}


