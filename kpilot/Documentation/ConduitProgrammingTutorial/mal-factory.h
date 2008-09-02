/* mal-factory.h                       KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer <reinhold@kainhofer.com>
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

#ifndef COUNDUITPROGRAMMINGTUTORIAL_MAL_FACTORY_H
#define COUNDUITPROGRAMMINGTUTORIAL_MAL_FACTORY_H

#include <klibloader.h>

class KComponentData;
class KAboutData;

class MALConduitFactory : public KLibFactory
{
Q_OBJECT

public:
    explicit MALConduitFactory(QObject * = 0L,const char * = 0L);
    virtual ~MALConduitFactory();

    static KAboutData *about() { return fAbout; } ;
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
    KComponentData fInstance;
    static KAboutData *fAbout;
    // KConfig entry keys.
    static const char *fGroup;
    static const char *fLastSync, *fSyncTime, 
        *fProxyType, *fProxyServer, *fProxyPort, *fProxyUser, *fProxyPassword;
} ;

extern "C" {
  void *init_libmalconduit();
} ;

#endif // COUNDUITPROGRAMMINGTUTORIAL_MAL_FACTORY_H
