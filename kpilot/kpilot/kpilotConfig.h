#ifndef _KPILOT_KPILOTCONFIG_H
#define _KPILOT_KPILOTCONFIG_H
/* kpilotConfig.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
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


#include <ksimpleconfig.h>
#include <qdatetime.h>
#include "kpilotSettings.h"

class KCmdLineArgs;
class QLineEdit;
class QComboBox;
class QCheckBox;


/*class KPilotConfigSettings : public KSimpleConfig
{
public:
	KPilotConfigSettings(const QString &filename,bool readonly=false);
	virtual ~KPilotConfigSettings();

	// Conduit configuration information
	void addDirtyDatabase(QString db);
	void addAppBlockChangedDatabase(QString db);
	void addFlagsChangedDatabase(QString db);
	
	QDateTime getLastSyncTime();
	void setLastSyncTime( QDateTime &);
	
} ;*/

class KPilotConfig
{
public:
	/**
	* Returns a (new) reference to the KPilot configuration object.
	* This is used to put all the KPilot configuration --
	* including conduits and such -- into one rc file and
	* not spread out among config files for each conduit.
	*
	* Callers should @em never delete this object.
	* @em Only call this after the KApplication object has been
	* created, or the program will crash (SIGSEGV in KDE 2.1,
	* qFatal() with a sensible message in KDE 2.2).
	*/
//	static KPilotConfigSettings& getConfig();

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
	* versionDetails() returns a descriptive string.
	* sorryVersionOutdated() uses KMessageBox to display it.
	* interactiveUpdate() tries to copy old configs to new.
	*/
	static QString versionDetails(int fileversion, bool run);
	static void sorryVersionOutdated(int fileversion);
	static void interactiveUpdate();

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
	
	static void sync() { KPilotSettings::self()->config()->sync(); }
} ;



#endif
