#ifndef KPILOT_KPILOTCONFIG_H
#define KPILOT_KPILOTCONFIG_H
/* kpilotConfig.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This class concentrates all the configuration
** information for the various parts of KPilot.
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


#include "kpilotSettings.h"

class KCmdLineArgs;


class KPilotConfig
{
public:
	typedef enum
	{
		Cancel=0,
		Normal,
		ConfigureKPilot,
		ConfigureConduits,
		ConfigureAndContinue
	} RunMode;

	/**
	 * @return QString of default path for the BackupDB files
	 * are located
	 */
	static QString getDefaultDBPath();


	// Conduit configuration information
	static void addDirtyDatabase(const QString &db);
	static void addAppBlockChangedDatabase(const QString &db);
	static void addFlagsChangedDatabase(const QString &db);

	/**
	* This number can be changed every time a new
	* KPilot version is released that absolutely requires
	* the user to take a look at the configuration of
	* KPilot.
	*/
	static const uint ConfigurationVersion;

	/**
	* Reads the configuration version from a configuration file.
	* TODO: Make this use the *standard* location.
	*/
	static int getConfigVersion();

	/**
	* Write the current configuration version to the standard
	* location. @em Only call this after the KApplication object
	* is created, or crashes will result.
	*/
	static void updateConfigVersion();

	/**
	* Warn the user that the config file is outdated.
	* versionDetails() returns a descriptive string. Pass in the
	*     actual version of the config file. Set @p run to true to add an
	*     admonition to run kpilot in config mode to fix this.
	* sorryVersionOutdated() uses KMessageBox to display it.
	*/
	static QString versionDetails(int fileversion, bool run);
	static void sorryVersionOutdated(int fileversion);
	/**
	* Update the config file as best we can, and inform the user.
	* Returns a suggested run mode if it's ok (ie. update finished, or
	* file was already up-to-date) and Cancel if the user cancels.
	* If the user cancels, it's probably best to _not_ continue with
	* anything, since the config is bogus.
	*
	* The suggested run mode might be anything - usually normal,
	* but might return ConfigureAndContinue as well.
	*
	* This function can call functions to update from different versions
	* to current; these are static in kpilotConfig.cc.
	*/
	static RunMode interactiveUpdate();

	/**
	* Deal with --debug options.
	* @ret resulting debug level
	*/
	static int getDebugLevel(KCmdLineArgs *p);

	/**
	* Returns the user's preference for the system-wide
	* fixed font.
	*/
	static const QFont& fixed() ;

	static void sync();
} ;

#define KPILOT_ABOUT_INIT(about) \
	about.addAuthor(ki18n("Adriaan de Groot"), \
		ki18n("Maintainer"), \
		"groot@kde.org", "http://www.kpilot.org/"); \
	about.addAuthor(ki18n("Jason 'vanRijn' Kasper"), \
		ki18n("Core and conduits Developer, Maintainer"), \
		"vR@movingparts.net", "http://movingparts.net/"); \
	about.addAuthor(ki18n("Reinhold Kainhofer"), \
		    ki18n("Developer"), \
		    "reinhold@kainhofer.com", "http://reinhold.kainhofer.com/Linux/"); \
	about.addAuthor(ki18n("Dan Pilone"), \
		    ki18n("Former Project Leader"), \
		    "pilone@slac.com"); \
	about.addCredit(ki18n("Preston Brown"), ki18n("VCal conduit")); \
	about.addCredit(ki18n("Greg Stern"), ki18n("Abbrowser conduit")); \
	about.addCredit(ki18n("Chris Molnar"), ki18n("Expenses conduit")); \
	about.addCredit(ki18n("Jörn Ahrens"), ki18n("Notepad conduit, Bugfixer")); \
	about.addCredit(ki18n("Heiko Purnhagen"), ki18n("Bugfixer")); \
	about.addCredit(ki18n("Jörg Habenicht"), ki18n("Bugfixer")); \
	about.addCredit(ki18n("Martin Junius"), \
		ki18n("XML GUI"), \
		"mj@m-j-s.net", "http://www.m-j-s.net/kde/"); \
	about.addCredit(ki18n("David Bishop"), \
		ki18n(".ui files")); \
	about.addCredit(ki18n("Aaron J. Seigo"), \
		ki18n("Bugfixer, coolness")); \
	about.addCredit(ki18n("Bertjan Broeksema"), \
		ki18n("VCalconduit state machine, CMake, Base Conduit rewrite")); \
	about.addCredit(ki18n("Montel Laurent"), \
		ki18n("KDE4 port"));  \
	about.setOrganizationDomain("kpilot.kde.org");

#define KPILOT_ABOUT_AUTHORS \
	    ki18n("(c) 1998-2000,2001, Dan Pilone\n(c) 2000-2007, Adriaan de Groot\n(c) 2005-2007, Jason 'vanRijn' Kasper")


#endif
