/*
** MAL-conduit.cc
**
** Copyright (C) 2002 by Reinhold Kainhofer
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
**
**
** Specific permission is granted for this code to be linked to libmal
** (this is necessary because the libmal license is not GPL-compatible).
*/
 



#include "options.h"
//#include <unistd.h>
//#include <time.h>

//#include <qtimer.h>

//#include <kglobal.h>
//#include <kmessagebox.h>
#include <kconfig.h>
#include <kdebug.h>

#include "mal-factory.h"
#include "mal-conduit.moc"
//#include "malsync/libmal.h"
#include <libmal.h>


// Something to allow us to check what revision
// the modules are that make up a binary distribution.
const char *MAL_conduit_id =
	"$Id$";


static MALConduit *conduitInstance=0L;

int malconduit_logf(const char *format, ...)
{
	FUNCTIONSETUP;
	va_list val;
	int rval;
	va_start(val, format);
#define WRITE_MAX_BUF	4096
	char msg[WRITE_MAX_BUF];
	msg[0]='\0';
	rval=vsnprintf(&msg[0], sizeof(msg), format, val);
	va_end(val);
	if (rval == -1) {
		msg[WRITE_MAX_BUF-1] = '\0';
		rval=WRITE_MAX_BUF-1;
	}
	if (conduitInstance)
	{
		conduitInstance->printLogMessage(msg);
	}
	else
	{
		// write out to stderr
		kdWarning()<<msg<<endl;
	}
	return rval;
}
 
 
MALConduit::MALConduit(KPilotDeviceLink * o,
	const char *n, 
	const QStringList & a) :
	ConduitAction(o, n, a)
{
	FUNCTIONSETUP;
	register_printStatusHook(malconduit_logf);
	register_printErrorHook(malconduit_logf);
	conduitInstance=this;
	(void) MAL_conduit_id;
}



MALConduit::~MALConduit()
{
	FUNCTIONSETUP;
}



void MALConduit::readConfig()
{
	FUNCTIONSETUP;
	QDateTime dt;
	KConfigGroupSaver g(fConfig, MALConduitFactory::group());
	fLastSync = fConfig->readDateTimeEntry(MALConduitFactory::lastSync(), &dt);
#ifdef DEBUG
	DEBUGCONDUIT<<"Last sync was "<<fLastSync.toString()<<endl;
#endif
	
	eSyncTime=fConfig->readNumEntry(MALConduitFactory::syncTime(), 0);
	
	// Proxy settings
	eProxyType=fConfig->readNumEntry(MALConduitFactory::proxyType(), 0);
	fProxyServer=fConfig->readEntry(MALConduitFactory::proxyServer(), "");
	
	fProxyPort=fConfig->readNumEntry(MALConduitFactory::proxyPort(), 0);
	fProxyUser=fConfig->readEntry(MALConduitFactory::proxyUser(), "");
	fProxyPassword=fConfig->readEntry(MALConduitFactory::proxyPassword(), "");

	// MAL Server settings (not yet possible!!!)
	fMALServer=fConfig->readEntry(MALConduitFactory::malServer(), "sync.avantgo.com");
	fMALPort=fConfig->readNumEntry(MALConduitFactory::malPort(), 0);
	
	fMALUser=fConfig->readEntry(MALConduitFactory::malUser(), "");
	fMALPassword=fConfig->readEntry(MALConduitFactory::malPassword(), "");
}



void MALConduit::saveConfig()
{
	FUNCTIONSETUP;
	KConfigGroupSaver g(fConfig, MALConduitFactory::group());
	
	fConfig->writeEntry(MALConduitFactory::lastSync(), QDateTime::currentDateTime());
}



bool MALConduit::skip() 
{
	QDateTime now=QDateTime::currentDateTime();
	if (!fLastSync.isValid() || !now.isValid()) return false;
	
	switch (eSyncTime) 
	{
		case eEveryHour:
			if ( (fLastSync.secsTo(now)<=3600) && (fLastSync.time().hour()==now.time().hour()) ) return true;
			else return false;
		case eEveryDay:
			if ( fLastSync.date() == now.date() ) return true;
			else return false;
		case eEveryWeek:
			if ( (fLastSync.daysTo(now)<=7)  && ( fLastSync.date().dayOfWeek()<=now.date().dayOfWeek()) ) return true;
			else return false;
		case eEveryMonth:
			if ( (fLastSync.daysTo(now)<=31) && (fLastSync.date().month()==now.date().month()) ) return true;
			else return false;
		case eEverySync:
		default:
			return false;
	}
	return false;
}



/* virtual */ bool MALConduit::exec()
{
	FUNCTIONSETUP;
	DEBUGCONDUIT<<MAL_conduit_id<<endl;

	if (!fConfig)
	{
		kdWarning() << k_funcinfo << ": No config file was set!" << endl;
		return false;
	}

	readConfig();
	
	// TODO: set the log/error message hooks of libmal here!!!

	if (skip()) 
	{
		emit logMessage(i18n("Skipping MAL sync, because last synchronization was not long enough ago."));
		emit syncDone(this);
		return true;
	}
	
	// Set all proxy settings
	switch (eProxyType) 
	{
		case eProxyHTTP:
			if (fProxyServer.isEmpty()) break;
#ifdef DEBUG
			DEBUGCONDUIT<<" Using HTTP proxy server \""<<fProxyServer<<"\", Port "<<fProxyPort<<", User "<<fProxyUser<<", Password "<<( (fProxyPassword.isEmpty())?QString("not "):QString(""))<<"set"<<endl;
#endif
			setHttpProxy(fProxyServer.latin1());
			if (fProxyPort>0 && fProxyPort<65536) setHttpProxyPort( fProxyPort );
			else setHttpProxyPort(80);
			
			if (!fProxyUser.isEmpty()) 
			{
				setProxyUsername( fProxyUser.latin1() );
				if (!fProxyPassword.isEmpty()) setProxyPassword( fProxyPassword.latin1() );
			}
			break;
		case eProxySOCKS:
#ifdef DEBUG
			DEBUGCONDUIT<<" Using SOCKS proxy server \""<<fProxyServer<<"\",  Port "<<fProxyPort<<", User "<<fProxyUser<<", Password "<<( (fProxyPassword.isEmpty())?QString("not "):QString("") )<<"set"<<endl;
#endif
			setSocksProxy( fProxyServer.latin1() );
			if (fProxyPort>0 && fProxyPort<65536) setSocksProxyPort( fProxyPort );
			else setSocksProxyPort(1080);
			break; 
		default:
			break;
	}


	// Now initiate the sync.
	PalmSyncInfo* pInfo=syncInfoNew();
	if (!pInfo) {
		kdWarning() << k_funcinfo << ": Could not allocate SyncInfo!" << endl;
		emit logError(i18n("MAL synchronization failed (no SyncInfo)."));
		return false;
	}
	malsync( pilotSocket(), pInfo);
	syncInfoFree(pInfo);

	saveConfig();
	emit syncDone(this);
	return true;
}

void MALConduit::printLogMessage(QString msg)
{
	FUNCTIONSETUP;
	emit logMessage(msg);
}

