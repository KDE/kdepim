#ifndef _MAL_CONDUIT_H
#define _MAL_CONDUIT_H
/* MAL-conduit.cc
**
** Copyright (C) 2002 by Reinhold Kainhofer
*/

/* This file is distributed under the Gnu General Public Licence (GPL).
** The GPL should have been included with this file in a file called
** COPYING. 
**
**
** Specific permission is granted for this code to be linked to libmal
** (this is necessary because the libmal license is not GPL-compatible).
*/

/* $Revision$
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/



#include <plugin.h>

#include <kapplication.h>
#include <qdatetime.h>

class MALConduit : public ConduitAction
{
Q_OBJECT
public:
	MALConduit(
		KPilotDeviceLink *o,
		const char *n = 0L,
		const QStringList &a = QStringList() );
	virtual ~MALConduit();
	void printLogMessage(QString msg);

public slots:
	virtual void exec();
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
	 * Check if the last sync was not so long ago that according to eSyncTime we can skip the sync this time
	 */
	bool skip();
private:
	enum eProxyTypeEnum {
		eProxyNone,
		eProxyHTTP,
		eProxySOCKS
	} eProxyType;
	enum eSyncTimeEnum {
		eEverySync,
		eEveryHour,
		eEveryDay,
		eEveryWeek,
		eEveryMonth
	} eSyncTime;
	QString fProxyServer, fProxyUser, fProxyPassword, fMALServer, fMALUser, fMALPassword;
	int fProxyPort, fMALPort;
	QDateTime fLastSync;
} ;




// $Log$
// Revision 1.1  2002/08/15 23:07:37  kainhofe
// First official version of the malconduit
//

#endif
