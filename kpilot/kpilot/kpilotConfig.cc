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

#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

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
/* static */ const int KPilotConfig::ConfigurationVersion = 402;

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
		DEBUGDB << fname
			<< ": Config file has version "
			<< version
			<< endl;
	}

	return version;
}

/* static */ void KPilotConfig::updateConfigVersion()
{
	FUNCTIONSETUP;

	KConfig &config = getConfig();
	config.writeEntry("Configured",ConfigurationVersion);
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
{
	return 0;
}

/* static */ int KPilotConfig::getDebugLevel(KCmdLineArgs *)
{
	return 0;
}
#else
/* static */ int KPilotConfig::getDebugLevel(KConfig&, const QString&)
{
	FUNCTIONSETUP;

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
}

/* static */ int KPilotConfig::getDebugLevel(KCmdLineArgs *p)
{
	FUNCTIONSETUP;

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
		DEBUGDB << fname 
			<< ": Making a new config file"
			<< endl;
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
		kdError() << __FUNCTION__
			<< ": **\n"
			<< ": ** No font was created! (Expect crash now)\n"
			<< ": **"
			<< endl;
	}

	return *thefont;
}

// $Log$
// Revision 1.5  2001/05/25 16:06:52  adridg
// DEBUG breakage
//
// Revision 1.4  2001/04/16 13:54:17  adridg
// --enable-final file inclusion fixups
//
// Revision 1.3  2001/03/27 23:54:43  stern
// Broke baseConduit functionality out into PilotConduitDatabase and added support for local mode in BaseConduit
//
// Revision 1.2  2001/02/25 12:39:15  adridg
// Removed stupid crash from ::fixed()
//
// Revision 1.1  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
