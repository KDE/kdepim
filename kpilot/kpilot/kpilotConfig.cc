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
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#include "options.h"

#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kconfig.h>

#include "kpilotConfig.h"


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
/* static */ const int KPilotConfig::ConfigurationVersion = 401;

/* static */ int KPilotConfig::getConfigVersion(KConfig *config)
{

	if (!config)	return 0;
	else		return getConfigVersion(*config);
	/* NOTREACHED */
	(void) kpilotconfig_id;
}

/* static */ int KPilotConfig::getConfigVersion(KConfig& config)
{
	FUNCTIONSETUP;

	config.setGroup(QString::null);
	int version=config.readNumEntry("Configured",0);
	if (version<ConfigurationVersion)
	{
		kdWarning() << __FUNCTION__ << ": Config file has old version "
			<< version
			<< endl;
	}
	else
	{
#ifdef DEBUG
		if (debug_level & UI_MINOR)
		{
			kdDebug() << fname
				<< ": Config file has version "
				<< version
				<< endl;
		}
#endif
	}

	return version;
}

/* static */ QString KPilotConfig::getDefaultDBPath()
    {
    KConfig& config = getConfig();
    QString lastUser = config.readEntry("UserName");
    QString dbsubpath = "kpilot/DBBackup/";
    QString defaultDBPath = KGlobal::dirs()->
	saveLocation("data", dbsubpath + lastUser + "/");
    return defaultDBPath;
    }

#ifdef DEBUG
/* static */ int KPilotConfig::getDebugLevel(KConfig& c,const QString& group)
#else
/* static */ int KPilotConfig::getDebugLevel(KConfig&, const QString&)
#endif
{
	FUNCTIONSETUP;

#ifdef DEBUG
	if (!group.isNull())
	{
		c.setGroup(group);
	}

	int d=c.readNumEntry("Debug",0);
	debug_level |= d;

	if (debug_level)
	{
		kdDebug() << fname 
			<< ": Debug level set to "
			<< debug_level
			<< endl;
	}

	return debug_level ;
#else
	return 0;
#endif
}

static KConfig *theconfig = 0L;
KConfig& KPilotConfig::getConfig(const QString &s)
{
	FUNCTIONSETUP;

	if (theconfig)
	{
		theconfig->setGroup(s);
		return *theconfig;
	}

	/**
	* This causes a crash if no instance has been created
	* yet. A standard KDE error message reports this fact.
	* It is a grave programming error, so we will let that
	* stand.
	*/
	QString existingConfig=
		KGlobal::dirs()->findResource("config", "kpilotrc");

	
	if (existingConfig.isNull())
	{
#ifdef DEBUG
		if (debug_level & UI_MAJOR)
		{
			kdDebug() << fname 
				<< ": Making a new config file"
				<< endl;
		}
#endif
		theconfig=new KConfig("kpilotrc",false,false);
	}
	else
	{
		theconfig=new KConfig(existingConfig,false,false);
	}

	if (theconfig == 0L)
	{
		kdWarning() << __FUNCTION__ << ": No configuration was found."
			<< endl;
	}

	theconfig->setGroup(s);
	return *theconfig;
}

static QFont *thefont=0L;

/* static */ const QFont& KPilotConfig::fixed()
{
	FUNCTIONSETUP;

	if (thefont)
	{
#ifdef DEBUG
		if (debug_level && UI_TEDIOUS)
		{
			DEBUGKPILOT << fname
				<< ": Font already set."
				<< endl;
		}
#endif

		return *thefont;
	}

	KConfig KDEGlobalConfig(QString::null);
	KDEGlobalConfig.setGroup("General");
	QString s = KDEGlobalConfig.readEntry("fixed");

	DEBUGKPILOT << fname
		<< ": Creating font "
		<< s 
		<< endl;

	thefont = new QFont(KDEGlobalConfig.readFontEntry("fixed"));

	if (!thefont)
	{
		kdError() << fname
			<< ": **\n"
			<< ": ** No font was created! (Expect crash now)\n"
			<< ": **"
			<< endl;
	}

	return *thefont;
}

// $Log$
// Revision 1.2  2001/02/25 12:39:15  adridg
// Removed stupid crash from ::fixed()
//
// Revision 1.1  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
