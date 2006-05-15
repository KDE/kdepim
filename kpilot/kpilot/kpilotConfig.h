#ifndef _KPILOT_KPILOTCONFIG_H
#define _KPILOT_KPILOTCONFIG_H
/* kpilotConfig.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
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
		ConfigureAndContinue,
		WizardAndContinue
	} RunMode;

	/**
	 * @return QString of default path for the BackupDB files
	 * are located
	 */
	static QString getDefaultDBPath();


	// Conduit configuration information
	static void addDirtyDatabase(QString db);
	static void addAppBlockChangedDatabase(QString db);
	static void addFlagsChangedDatabase(QString db);

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



#endif
