// Time-conduit.cc
//
// Copyright (C) 2002 by Reinhold Kainhofer
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
//
 



#include "options.h"

// Only include what we really need:
// First UNIX system stuff, then std C++,
// then Qt, then KDE, then local includes.
//
//
//#include <unistd.h>
#include <time.h>

//#include <qtimer.h>

//#include <kglobal.h>
//#include <kmessagebox.h>
#include <kconfig.h>
#include <kdebug.h>

//#include <pilotUser.h>

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


/* virtual */ void TimeConduit::exec()
{
	FUNCTIONSETUP;

	if (!fConfig)
	{
		kdWarning() << k_funcinfo << ": No config file was set!" << endl;
		emit syncDone(this);
		return;
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
			emit syncDone(this);
			return;
	}
	emit syncDone(this);
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

// $Log$
// Revision 1.6  2002/08/12 09:48:11  kainhofe
// better log message
//
// Revision 1.5  2002/07/31 06:43:41  kainhofe
// typographical errors
//
// Revision 1.4  2002/07/31 06:40:30  kainhofe
// skip conduit for PalmOS 3.25 and 3.3, which don't support setting the time
//
// Revision 1.2  2002/07/25 21:58:11  kainhofe
// compile error
//
