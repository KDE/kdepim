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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
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
#include "timeConduitSettings.h"


// Something to allow us to check what revision
// the modules are that make up a binary distribution.
extern "C"
{
long version_conduit_time = KPILOT_PLUGIN_API ;
const char *id_conduit_time =
	"$Id$";
}



TimeConduit::TimeConduit(KPilotDeviceLink * o,
	const char *n,
	const QStringList & a) :
	ConduitAction(o, n, a)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT<<id_conduit_time<<endl;
#endif
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
	DEBUGCONDUIT<<id_conduit_time<<endl;

	readConfig();

	switch (TimeConduitSettings::direction())
	{
		case TimeConduitSettings::eSetHHfromPC:
			emit logMessage(i18n("Setting the clock on the handheld"));
//			fHandle->addSyncLogEntry(i18n("Setting the clock on the handheld"));
			syncHHfromPC();
			break;
		case TimeConduitSettings::eSetPCfromHH:
			emit logMessage(i18n("Setting the clock on the PC from the time on the handheld"));
//			fHandle->addSyncLogEntry(i18n("Setting the clock on the PC from the time on the handheld"));
			syncPCfromHH();
			break;
		default:
			emit logError(i18n("Unknown setting for time synchronization."));
			kdWarning() << k_funcinfo << ": unknown sync direction "<<TimeConduitSettings::direction()<<endl;
			return false;
	}
	emit syncDone(this);
	return true;
}

void TimeConduit::syncPCfromHH()
{
	FUNCTIONSETUP;
	QDateTime pdaTime=fHandle->getTime();
#ifdef DEBUG
	DEBUGCONDUIT<<fname<<": syncing time "<<pdaTime.toString()<<" to the PC"<<endl;
#endif
	emit logError(i18n("The system clock was not adjusted to %1 (not implemented)").arg(pdaTime.toString()));
	// TODO: Set the system time from this QDateTime
}



void TimeConduit::syncHHfromPC()
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
