#ifndef _TIME_FACTORY_H
#define _TIME_FACTORY_H
/* MAL-factory.h                       KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the mal-conduit plugin.
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

#include <klibloader.h>

class KInstance;
class KAboutData;

class MALConduitFactory : public KLibFactory
{
Q_OBJECT

public:
	MALConduitFactory(QObject * = 0L,const char * = 0L);
	virtual ~MALConduitFactory();

	static KAboutData *about() { return fAbout; } ;
	static const char *group() { return fGroup; } ;
	static const char *lastSync() { return fLastSync; };
	static const char *syncTime() {return fSyncTime;};
	static const char *proxyType() {return fProxyType;};
	static const char *proxyServer() {return fProxyServer;};
	static const char *proxyPort() {return fProxyPort;};
	static const char *proxyUser() {return fProxyUser;};
	static const char *proxyPassword() {return fProxyPassword;};
	static const char *malServer() {return fMALServer;};
	static const char *malPort() {return fMALPort;};
	static const char *malUser() {return fMALUser;};
	static const char *malPassword() {return fMALPassword;};
	
protected:
	virtual QObject* createObject( QObject* parent = 0,
		const char* name = 0,
		const char* classname = "QObject",
		const QStringList &args = QStringList() );
private:
	KInstance *fInstance;
	static KAboutData *fAbout;
	// KConfig entry keys.
	static const char *fGroup;
	static const char *fLastSync, *fSyncTime, 
		*fProxyType, *fProxyServer, *fProxyPort, *fProxyUser, *fProxyPassword, 
		*fMALServer, *fMALPort, *fMALUser, *fMALPassword;
} ;

extern "C"
{

void *init_libtimeconduit();

} ;

#endif
