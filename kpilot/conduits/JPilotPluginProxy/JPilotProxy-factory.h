#ifndef _KPILOT_JPilotProxy_FACTORY_H
#define _KPILOT_JPilotProxy_FACTORY_H
/* JPilotProxy-factory.h                       KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the JPilotProxy-conduit plugin.
** It also defines the class for the behavior of the setup dialog.
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <klibloader.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include "kpilotlink.h"
#include "plugin.h"
#include "JPilotProxy-setup.h"
#include "JPilotProxy-conduit.h"
#include "jplugin.h"

#ifdef KDE2
#include <qlist.h>
#define PluginList_t QList<JPlugin>
#define PluginIterator_t QListIterator<JPlugin>
#else
#include <qptrlist.h>
#define PluginList_t QList<JPlugin>
#define PluginIterator_t QPtrListIterator<JPlugin>
#endif


class JPilotProxyConduitFactory : public KLibFactory {
Q_OBJECT
public:
	JPilotProxyConduitFactory(QObject * = 0L,const char * = 0L);
	virtual ~JPilotProxyConduitFactory();

	static KAboutData *about() { return fAbout; } ;

	virtual QObject*createSetupWidget(QWidget*w, const char*n, const QStringList &l) {return new JPilotProxyWidgetSetup(w,n,l);};
	virtual QObject*createConduit(KPilotDeviceLink*w, const char*n=0L, const QStringList &l=QStringList()) { return new JPilotProxyConduit(w,n,l);};
	static void readSettings();
	static JPlugin*addPlugin(QString path, bool on);
	static int removePlugin(QString path);
	static int addPluginPath(QString path, KConfig*fC=NULL);
	static int scanPluginPathes();
	static int loadPlugins(KConfig*fC);
protected:
	virtual QObject* createObject( QObject* parent = 0,
		const char* name = 0,
		const char* classname = "QObject",
		const QStringList &args = QStringList() );
private:
	KInstance *fInstance;
public:
	KConfig *fConfig;
	static KAboutData *fAbout;
	static PluginList_t *plugins;
	static QString settingsGroup;
	static QString PluginPathes;
	static QString LoadedPlugins;
	static bool pluginsloaded;
	KLibrary*apilib;
};

extern "C"
{

void *init_libJPilotProxy();

};


// $Log$
// Revision 1.1  2002/04/07 11:17:54  kainhofe
// First Version of the JPilotPlugin Proxy conduit. it can be activated, but loading a plugin or syncing a plugin crashes the palm (if no plugin is explicitely enabled, this conduit can be enabled and it won't crash KPIlot). A lot of work needs to be done, see the TODO
//
// Revision 1.4  2002/04/07 00:10:49  reinhold
// Settings are now saved
//
// Revision 1.3  2002/04/06 19:08:02  reinhold
// the plugin compiles now and the plugins can be loaded (except that they crash if they access jp_init or jpilog_printf etc.)
//
// Revision 1.2  2002/03/20 01:27:29  reinhold
// The plugin's setup dialog now runs without crashes. loading JPilot plugins still fails because of inresolved dependencies like jp_init or gtk_... functions.
//
// Revision 1.1  2002/03/18 23:15:55  reinhold
// Plugin compiles now
//
// Revision 1.3  2002/03/15 20:43:17  reinhold
// Fixed the crash on loading (member function not defined)...
//
// Revision 1.2  2002/03/10 23:58:32  reinhold
// Made the conduit compile...
//
// Revision 1.1.1.1  2002/03/09 15:38:45  reinhold
// Initial checin of the generic project manager / List manager conduit.
//
//


#endif
