#ifndef _MAL_CONDUIT_H
#define _MAL_CONDUIT_H
/* mal-conduit.h                           KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
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
**
**
** Specific permission is granted for this code to be linked to libmal
** (this is necessary because the libmal license is not GPL-compatible).
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/



#include <plugin.h>

class MALConduit : public ConduitAction
{
Q_OBJECT
public:
	MALConduit(
		KPilotLink *o,
		const char *n = 0L,
		const QStringList &a = QStringList() );
	virtual ~MALConduit();
	void printLogMessage(QString msg);
	virtual bool exec();

protected:
	/**
	 * Read in the config from the KPilot config files and fill the member variables accordingly
	 */
	void readConfig();
	/**
	 * Store the sync time in the KPilot configuration
	 */
	void saveConfig();
	/**
	 * Check if the last sync was not so long ago that according to MALConduitSettings::syncFrequency() we can skip the sync this time
	 */
	bool skip();
} ;


#endif
