/* JPilotProxy-factory.cc					  KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the JPilotProxy-conduit plugin.
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

#include "options.h"

#include <qdir.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include "kpilotlink.h"
#include "kconfig.h"
#include "JPilotProxy-factory.moc"


extern "C"
{

void *init_libJPilotProxy() {
	FUNCTIONSETUP;
//	K_EXPORT_COMPONENT_FACTORY( libkspread, KSpreadFactory )
	return new JPilotProxyConduitFactory;
}

};


bool JPilotProxyConduitFactory::pluginsloaded=false;
QString JPilotProxyConduitFactory::settingsGroup="JPilotPluginProxy";
QString JPilotProxyConduitFactory::PluginPathes="PluginPathes";
QString JPilotProxyConduitFactory::LoadedPlugins="LoadedPlugins";

KAboutData *JPilotProxyConduitFactory::fAbout = 0L;
PluginList_t *JPilotProxyConduitFactory::plugins=0L;

JPilotProxyConduitFactory::JPilotProxyConduitFactory(QObject *p, const char *n) :
		KLibFactory(p,n)  {
	FUNCTIONSETUP;
	fConfig=0L;
	plugins=new PluginList_t();

	fInstance = new KInstance(n);
	fAbout = new KAboutData(n,
		I18N_NOOP("JPilotProxy Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the JPilotProxy Conduit for KPilot"),
		KAboutData::License_GPL,
		"(C) 2002, Reinhold F. Kainhofer");
	fAbout->addAuthor("Dan Pilone", I18N_NOOP("Original Author of KPilot"));
	fAbout->addAuthor("Adriaan de Groot", I18N_NOOP("Maintainer or KPilot"), "groot@kde.org", "http://www.cs.kun.nl/~adridg/kpilot");
	fAbout->addAuthor("Reinhold Kainhofer", I18N_NOOP("Original author and maintainer of this conduit"), "reinhold@kainhofer.com", "http://reinhold.kainhofer.com");
}

JPilotProxyConduitFactory::~JPilotProxyConduitFactory() {
	FUNCTIONSETUP;

	PluginIterator_t it(*plugins); // iterator for plugin list
	for ( ; it.current(); ++it ) {
		it.current()->exit_cleanup();
	}

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *JPilotProxyConduitFactory::createObject( QObject *p,
	const char *n, const char *c, const QStringList &a) {
	FUNCTIONSETUP;

		#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Creating object of class "	<< c << endl;
		#endif

	if (qstrcmp(c,"ConduitConfig")==0) {
		QWidget *w = dynamic_cast<QWidget *>(p);

		if (w) {
			return createSetupWidget(w,n,a);
		} else {
				#ifdef DEBUG
			DEBUGCONDUIT << fname << ": Couldn't cast parent to widget." << endl;
				#endif
			return 0L;
		}
	}

	if (qstrcmp(c,"SyncAction")==0) {
		KPilotDeviceLink *d = dynamic_cast<KPilotDeviceLink *>(p);

		if (d) {
			return createConduit(d,n,a);
		} else {
			kdError() << k_funcinfo
				<< ": Couldn't cast to KPilotDeviceLink."
				<< endl;
		}
	}

	return 0L;
}

JPlugin*JPilotProxyConduitFactory::addPlugin( QString path, bool on) {
	FUNCTIONSETUP;
	// TODO: search the plugin list if the plugin was already loaded
	JPlugin*newplugin=new JPlugin( path );
	#ifdef DEBUG
	DEBUGCONDUIT<<"successfully created a JPlugin instance for "<<path<<endl;
	#endif
	if (newplugin->loaded) {
		newplugin->info.sync_on=on;
		#ifdef DEBUG
		DEBUGCONDUIT<<"loading "<<path<<" was successful"<<endl;
		#endif
		// if the plugin was loaded successfully, insert it into the list of plugins
		plugins->append(newplugin);
		jp_startup_info si;
		si.base_dir="/usr/local";
		newplugin->startup(&si);
		return newplugin;
	} else delete newplugin;
	return 0;
}

// This is not yet optimal, but should work for now...
int JPilotProxyConduitFactory::removePlugin( QString path) {
	FUNCTIONSETUP;
	
	JPlugin*plugintodel=NULL;

	PluginIterator_t it(*plugins); // iterator for plugin list
	for ( ; it.current(); ++it ) {
		JPlugin *plug = it.current();
		if (plug->lib && strcmp(plug->info.fullpath, path)) plugintodel=plug;
	}

	if (plugintodel) {
		plugins->take(plugins->find(plugintodel));
		plugintodel->exit_cleanup();
		delete plugintodel;
	}
}

int JPilotProxyConduitFactory::addPluginPath(QString path, KConfig*fC) {
	FUNCTIONSETUP;
	// find the list of possible plugins in the directory given by path
	QDir dir(path);
	QStringList plugs=dir.entryList("*.so");

	for (QStringList::Iterator it = plugs.begin(); it != plugs.end(); ++it ) {
		#ifdef DEBUG
		DEBUGCONDUIT<<"Load plugin "<<(*it)<<endl;
		#endif
		bool on=false;
		if (fC) {
			KConfigGroupSaver cfgs(fC, settingsGroup);
			on=fC->readBoolEntry(*it);
		}
		addPlugin(dir.absFilePath(*it), on);
	}
}

int JPilotProxyConduitFactory::loadPlugins(KConfig*fC) {
	FUNCTIONSETUP;
	if (!fC) return -1;
		
	KConfigGroupSaver cfgs(fC, settingsGroup);
	
	QStringList pathes=fC->readListEntry(PluginPathes);
	for (QStringList::Iterator it = pathes.begin(); it != pathes.end(); ++it ) {
		addPluginPath(*it, fC);
	}
	// now load the individual plugins...
	QStringList plugs=fC->readListEntry(LoadedPlugins);
	for (QStringList::Iterator it = plugs.begin(); it != plugs.end(); ++it ) {
		addPlugin(*it, fC->readBoolEntry(*it));
	}
	pluginsloaded=true;
	
	QStringList loadedplugs;
	// TODO: Write out the plugin list to the config file.
/*XXX	PluginIterator_t it(*plugins); // iterator for plugin list
	for ( ; it.current(); ++it ) {
		loadedplugs.append(it.current()->info.fullpath);
		// TODO:...
//		QStringList pluginfo();
//		fConfig->
	}*/
	
}

// $Log$
// Revision 1.6  2002/04/07 11:11:18  reinhold
// If the plugin is removed, this conduit does no longer crash
//
// Revision 1.5  2002/04/07 00:10:49  reinhold
// Settings are now saved
//
// Revision 1.4  2002/04/06 19:08:02  reinhold
// the plugin compiles now and the plugins can be loaded (except that they crash if they access jp_init or jpilog_printf etc.)
//
// Revision 1.3  2002/04/01 14:37:33  reinhold
// Use DirListIterator to find the plugins in a directorz
// User KLibLoader to load the JPilot plugins
//
// Revision 1.2  2002/03/20 01:27:29  reinhold
// The plugin's setup dialog now runs without crashes. loading JPilot plugins still fails because of inresolved dependencies like jp_init or gtk_... functions.
//
// Revision 1.1  2002/03/18 23:16:11  reinhold
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
