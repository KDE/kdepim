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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#ifndef _KPILOT_KPILOTCONFIG_H
#define _KPILOT_KPILOTCONFIG_H

// Normally I wouldn't include this since you can use "class KConfig"
// instead, but there's really no point in including this file at all
// without KConfig as well.
//
//
#ifndef _KCONFIG_H
#include <kconfig.h>
#endif

class KPilotConfig
{
public:
	/**
	* Returns a (new) pointer to the KPilot configuration object.
	* This is used to put all the KPilot configuration --
	* including conduits and such -- into one rc file and
	* not spread out among config files for each conduit.
	*
	* Callers should delete this object when no longer needed.
	*/
	static KConfig& getConfig(const QString &group=QString::null);

        /**
	 * @return QString of default path for the BackupDB files
	 * are located
	 */
        static QString getDefaultDBPath();

  
	/**
	* This number can be changed every time a new
	* KPilot version is released that absolutely requires
	* the user to take a look at the configuration of
	* KPilot.
	*/
	static const int ConfigurationVersion;

	/**
	* Reads the configuration version from a standard location.
	*/
	static int getConfigVersion(KConfig *);
	static int getConfigVersion(KConfig&);

	/**
	* We might have an additional Debug= line in their
	* config which may be read and ORed with the user-specified
	* debug level. This function does that. 
	*
	* @ret resulting debug level
	*/
	static int getDebugLevel(KConfig&,const QString& group=QString::null);


	/**
	* Returns the user's preference for the system-wide
	* fixed font.
	*/
	static const QFont& fixed() ;
} ;

#else
#warning "File doubly included"
#endif


// $Log$
// Revision 1.2  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.1  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
