/* KPilot
**
** Copyright (C) 2002-2003 by Reinhold Kainhofer
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
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org.
*/

#include "options.h"

#include <time.h>

#include <pilotSysInfo.h>

#include <kconfig.h>
#include <kdebug.h>

#include "time-factory.h"
#include "time-conduit.h"
#include "timeConduitSettings.h"


// Something to allow us to check what revision
// the modules are that make up a binary distribution.
extern "C"
{
unsigned long version_conduit_time = Pilot::PLUGIN_API ;
}



TimeConduit::TimeConduit(KPilotLink * o,
	const char *n,
	const QStringList & a) :
	ConduitAction(o, n, a)
{
	FUNCTIONSETUP;
	fConduitName=i18n("Time");
}



TimeConduit::~TimeConduit()
{
	FUNCTIONSETUP;
}



void TimeConduit::readConfig()
{
	FUNCTIONSETUP;
	TimeConduitSettings::self()->readConfig();
}


/* virtual */ bool TimeConduit::exec()
{
	FUNCTIONSETUP;

	readConfig();

	if (syncMode().isLocal())
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Would have set time to "
			<< QDateTime::currentDateTime().toString() << endl;
#endif
		return delayDone();
	}

	emit logMessage(i18n("Setting the clock on the handheld"));
	syncHHfromPC();
	return delayDone();
}


void TimeConduit::syncHHfromPC()
{
	FUNCTIONSETUP;
	time_t ltime;
	time(&ltime);

	long int major=fHandle->getSysInfo().getMajorVersion(), 
		 minor=fHandle->getSysInfo().getMinorVersion();

	if (major==3 && (minor==25 || minor==30))
	{
		emit logMessage(i18n("PalmOS 3.25 and 3.3 do not support setting the system time. Skipping the time conduit..."));
		return;
	}

	int sd = pilotSocket();
	if ( sd > 0 )
	{
		dlp_SetSysDateTime( sd, ltime );
	}
	else
	{
		WARNINGKPILOT << "Link is not a real device." << endl;
	}
}
