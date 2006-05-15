#ifndef _TIME_FACTORY_H
#define _TIME_FACTORY_H
/* MAL-factory.h                       KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the mal-conduit plugin.
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
        *fProxyType, *fProxyServer, *fProxyPort, *fProxyUser, *fProxyPassword;
} ;

extern "C" {
  void *init_libmalconduit();
} ;

#endif
