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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
**
**
** Specific permission is granted for this code to be linked to libmal
** (this is necessary because the libmal license is not GPL-compatible).
*/
 
/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
 



#include "options.h"

#include <qregexp.h>
#include <kconfig.h>
#include <kdebug.h>

#include "mal-factory.h"
#include "mal-conduit.moc"
#include <libmal.h>
#include "malconduitSettings.h"


// Something to allow us to check what revision
// the modules are that make up a binary distribution.
const char *MAL_conduit_id =
	"$Id$";


static MALConduit *conduitInstance=0L;

int malconduit_logf(const char *, ...) __attribute__ ((format (printf, 1, 2)));

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
 
#ifndef LIBMAL20
int32 cbTask (void * /*out*/,
        int32 * /*returnErrorCode*/,
        char *currentTask,
        AGBool /*bufferable*/)
{
    if (currentTask) {
        malconduit_logf ("%s\n", currentTask);
    }

    return AGCLIENT_CONTINUE;
}

static int32 cbItem (void */*out*/,
        int32 * /*returnErrorCode*/,
        int32   /*currentItemNumber*/,
        int32   /*totalItemCount*/,
        char *  /*currentItem*/)
{
//	The log widget only supports writing out whole lines. You just can't add a single character 
//	to the last line. Thus I completely remove the pseudo-percentbar.
/*    malconduit_logf (".");

    if (currentItemNumber == totalItemCount) {
        malconduit_logf ("\n");
    }
*/
    return AGCLIENT_CONTINUE;
}
#endif

 
MALConduit::MALConduit(KPilotDeviceLink * o,
	const char *n, 
	const QStringList & a) :
	ConduitAction(o, n, a)
{
	FUNCTIONSETUP;
#ifdef LIBMAL20
	register_printStatusHook(malconduit_logf);
	register_printErrorHook(malconduit_logf);
#endif
	conduitInstance=this;
#ifdef DEBUG
	DEBUGCONDUIT<<MAL_conduit_id<<endl;
#endif
	fConduitName=i18n("MAL");
}



MALConduit::~MALConduit()
{
	FUNCTIONSETUP;
}



void MALConduit::readConfig()
{
	FUNCTIONSETUP;
	MALConduitSettings::self()->readConfig();
#ifdef DEBUG
	DEBUGCONDUIT<<"Last sync was "<<MALConduitSettings::lastMALSync().toString()<<endl;
#endif
}



void MALConduit::saveConfig()
{
	FUNCTIONSETUP;
	MALConduitSettings::setLastMALSync( QDateTime::currentDateTime() );
	MALConduitSettings::self()->writeConfig();
}



bool MALConduit::skip() 
{
	QDateTime now=QDateTime::currentDateTime();
	QDateTime lastSync=MALConduitSettings::lastMALSync();
	
	if (!lastSync.isValid() || !now.isValid()) return false;
	
	switch ( MALConduitSettings::syncFrequency() ) 
	{
		case MALConduitSettings::eEveryHour:
			if ( (lastSync.secsTo(now)<=3600) && (lastSync.time().hour()==now.time().hour()) ) return true;
			else return false;
		case MALConduitSettings::eEveryDay:
			if ( lastSync.date() == now.date() ) return true;
			else return false;
		case MALConduitSettings::eEveryWeek:
			if ( (lastSync.daysTo(now)<=7)  && ( lastSync.date().dayOfWeek()<=now.date().dayOfWeek()) ) return true;
			else return false;
		case MALConduitSettings::eEveryMonth:
			if ( (lastSync.daysTo(now)<=31) && (lastSync.date().month()==now.date().month()) ) return true;
			else return false;
		case MALConduitSettings::eEverySync:
		default:
			return false;
	}
	return false;
}



/* virtual */ bool MALConduit::exec()
{
	FUNCTIONSETUP;
	DEBUGCONDUIT<<MAL_conduit_id<<endl;

	readConfig();
	
	// TODO: set the log/error message hooks of libmal here!!!

	if (skip()) 
	{
		emit logMessage(i18n("Skipping MAL sync, because last synchronization was not long enough ago."));
		emit syncDone(this);
		return true;
	}
	
	// Now initiate the sync.
	PalmSyncInfo* pInfo=syncInfoNew();
	if (!pInfo) {
		kdWarning() << k_funcinfo << ": Could not allocate SyncInfo!" << endl;
		emit logError(i18n("MAL synchronization failed (no SyncInfo)."));
		return false;
	}

#ifndef LIBMAL20
	pInfo->lowres = 1;
#endif

	QString proxyServer( MALConduitSettings::proxyServer() );
	int proxyPort( MALConduitSettings::proxyPort() );
	QString syncMessage;
	bool canContinue = true;
	// Set all proxy settings
	switch (MALConduitSettings::proxyType()) 
	{
		case MALConduitSettings::eProxyHTTP:
			if (proxyServer.isEmpty()) 
			{
				canContinue = false;
				syncMessage = i18n("No proxy server is set.");
				break;
			}
			syncMessage = i18n("Using proxy server: %1").arg(proxyServer);

#ifdef DEBUG
			DEBUGCONDUIT<<" Using HTTP proxy server \""<<proxyServer<<
				"\", Port "<<proxyPort<<", User "<<MALConduitSettings::proxyUser()<<
				", Password "<<( (MALConduitSettings::proxyPassword().isEmpty())?QString("not "):QString())<<"set"
				<<endl;
#endif
#ifdef LIBMAL20
			setHttpProxy(const_cast<char *>(proxyServer.latin1()));
			if (proxyPort>0 && proxyPort<65536) setHttpProxyPort( proxyPort );
			else setHttpProxyPort(80);
#else
			pInfo->httpProxy = new char[ proxyServer.length() + 1 ];
			strlcpy( pInfo->httpProxy, proxyServer.latin1(), proxyServer.length() + 1);
			if (proxyPort>0 && proxyPort<65536) pInfo->httpProxyPort = proxyPort;
			else pInfo->httpProxyPort = 80;
#endif
			
			if (!MALConduitSettings::proxyUser().isEmpty()) 
			{
#ifdef LIBMAL20
				setProxyUsername( const_cast<char *>(MALConduitSettings::proxyUser().latin1()) );
				if (!MALConduitSettings::proxyPassword().isEmpty()) setProxyPassword( const_cast<char *>(MALConduitSettings::proxyPassword().latin1()) );
#else
				pInfo->proxyUsername = new char[ MALConduitSettings::proxyUser().length() + 1 ];
				strlcpy( pInfo->proxyUsername, MALConduitSettings::proxyUser().latin1(), MALConduitSettings::proxyUser().length() + 1);
				if (!MALConduitSettings::proxyPassword().isEmpty()) {
//						pInfo->proxyPassword = MALConduitSettings::proxyPassword().latin1();
					pInfo->proxyPassword = new char[ MALConduitSettings::proxyPassword().length() + 1 ];
					strlcpy( pInfo->proxyPassword, MALConduitSettings::proxyPassword().latin1(), MALConduitSettings::proxyPassword().length() + 1);
				}
#endif
			}
			break;
		case MALConduitSettings::eProxySOCKS:
			if (proxyServer.isEmpty()) 
			{
				canContinue = false;
				syncMessage = i18n("No SOCKS proxy is set.");
				break;
			}
			syncMessage = i18n("Using SOCKS proxy: %1").arg(proxyServer);
#ifdef DEBUG
			DEBUGCONDUIT<<" Using SOCKS proxy server \""<<proxyServer<<"\",  Port "<<proxyPort<<", User "<<MALConduitSettings::proxyUser()<<", Password "<<( (MALConduitSettings::proxyPassword().isEmpty())?QString("not "):QString() )<<"set"<<endl;
#endif
#ifdef LIBMAL20
			setSocksProxy( const_cast<char *>(proxyServer.latin1()) );
			if (proxyPort>0 && proxyPort<65536) setSocksProxyPort( proxyPort );
			else setSocksProxyPort(1080);
#else
//			pInfo->socksProxy = proxyServer.latin1();
			pInfo->socksProxy = new char[ proxyServer.length() + 1 ];
			strlcpy( pInfo->socksProxy, proxyServer.latin1(), proxyServer.length() + 1);
			if (proxyPort>0 && proxyPort<65536) pInfo->socksProxyPort = proxyPort;
			else pInfo->socksProxyPort = 1080;
#endif
			break; 
		default:
			break;
	}

	logMessage(syncMessage);

	if (!canContinue)
	{
		return false;
	}

#ifdef LIBMAL20
	malsync( pilotSocket(), pInfo);
#else
	pInfo->sd = pilotSocket();
	pInfo->taskFunc = cbTask;
	pInfo->itemFunc = cbItem;
	malsync( pInfo );
	delete[] pInfo->httpProxy;
	delete[] pInfo->proxyUsername;
	delete[] pInfo->proxyPassword;
	delete[] pInfo->socksProxy;
	syncInfoFree(pInfo);
#endif

	saveConfig();
	return delayDone();
}

void MALConduit::printLogMessage(QString msg)
{
	FUNCTIONSETUP;
	// Remove the pseudo-progressbar:
	QString newmsg(msg);
	newmsg.replace( QRegExp("^\\s*\\.*\\s*"), "");
	newmsg.replace( QRegExp("\\s*\\.*\\s*$"), "");
	if (newmsg.length()>0)
	{
		emit logMessage(newmsg);
	}
}

