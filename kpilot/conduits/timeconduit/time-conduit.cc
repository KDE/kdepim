/* time-conduit.cc                           KPilot
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org.
*/

#include "options.h"

#include <time.h>

#include <kconfig.h>
#include <kdebug.h>

#include "time-factory.h"
#include "time-conduit.moc"


// Something to allow us to check what revision
// the modules are that make up a binary distribution.
const char *Time_conduit_id =
	"$Id$";


 
 
TimeConduit::TimeConduit(KPilotDeviceLink * o,
	const char *n, 
	const QStringList & a) :
	ConduitAction(o, n, a),
	fDirection(0)
{
	FUNCTIONSETUP;
	(void) Time_conduit_id;
}



TimeConduit::~TimeConduit()
{
	FUNCTIONSETUP;
}



void TimeConduit::readConfig()
{
	FUNCTIONSETUP;
	KConfigGroupSaver g(fConfig, TimeConduitFactory::group());
	fDirection = fConfig->readNumEntry(TimeConduitFactory::direction(),DIR_PCToPalm);
}


/* virtual */ bool TimeConduit::exec()
{
	FUNCTIONSETUP;
	DEBUGCONDUIT<<Time_conduit_id<<endl;

	if (!fConfig)
	{
		kdWarning() << k_funcinfo << ": No config file was set!" << endl;
		return false;
	}

	readConfig();

	switch (fDirection) 
	{
		case DIR_PCToPalm:
			emit logMessage(i18n("Setting the clock on the handheld"));
//			fHandle->addSyncLogEntry(i18n("Setting the clock on the handheld"));
			syncPCToPalm();
			break;
		case DIR_PalmToPC:
			emit logMessage(i18n("Setting the clock on the PC from the time on the handheld"));
//			fHandle->addSyncLogEntry(i18n("Setting the clock on the PC from the time on the handheld"));
			syncPalmToPC();
			break;
		default:
			emit logError(i18n("Unknown setting for time synchronization."));
			kdWarning() << k_funcinfo << ": unknown sync direction "<<fDirection<<endl;
			return false;
	}
	emit syncDone(this);
	return true;
}

void TimeConduit::syncPalmToPC()
{
	FUNCTIONSETUP;
	QDateTime pdaTime=fHandle->getTime();
#ifdef DEBUG
	DEBUGCONDUIT<<fname<<": syncing time "<<pdaTime.toString()<<" to the PC"<<endl;
#endif
	emit logError(i18n("The system clock was not adjusted to %1 (not implemented)").arg(pdaTime.toString()));
	// TODO: Set the system time from this QDateTime
}



void TimeConduit::syncPCToPalm()
{
	FUNCTIONSETUP;
	time_t ltime;
	time(&ltime);
	QDateTime time=QDateTime::currentDateTime();
	
	long int major=fHandle->majorVersion(), minor=fHandle->minorVersion();
	
	if (major==3 && (minor==25 || minor==30)) 
	{
		emit logMessage(i18n("PalmOS 3.25 and 3.3 do not support setting the system time. Skipping the time conduit..."));
		return;
	}

//	fHandle->setTime(QDateTime::currentDateTime());
	fHandle->setTime(ltime);
#ifdef DEBUG
	time.setTime_t(ltime);
	DEBUGCONDUIT<<fname<<": synced time "<<time.toString()<<" to the handheld"<<endl;
#endif
}
