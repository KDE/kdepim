#ifndef _KPILOT_JPilotProxy_FACTORY_H
#define _KPILOT_JPilotProxy_FACTORY_H
/* JPilotProxy-factory.h                       KPilot
**
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
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
	static int addPluginPath(QString path);
	static int scanPluginPathes();
	static int loadPlugins();
protected:
	virtual QObject* createObject( QObject* parent = 0,
		const char* name = 0,
		const char* classname = "QObject",
		const QStringList &args = QStringList() );
private:
	KInstance *fInstance;
public:
	static KAboutData *fAbout;
	static PluginList_t *plugins;
	static bool pluginsloaded;
	KLibrary*apilib;
};

extern "C"
{

void *init_libJPilotProxy();

};


#endif
