/***************************************************************************
						  jplugin.cpp  -  description
							 -------------------
	begin				: Sat Mar 16 2002
	copyright			: (C) 2002 by reinhold
	email				: reinhold@albert
 ***************************************************************************/

/***************************************************************************
 *																		 *
 * JPilotPlugin Proxy Copyright (C) 2002 by Reinhold Kainhofer			 *
 * JPilot Plugin API Copyright (C) 1999 by Judd Montgomery				 *
 * KPilot Conduit API Copyright by Dan Pilone, Adriaan de Groot			*
 *																		 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or	 *
 *   (at your option) any later version.								   *
 *																		 *
 ***************************************************************************/

#include <stdlib.h>
//#include <dlfcn.h>
#include <string.h>
#include <klibloader.h>
#include "jplugin.h"
#include "options.h"

extern "C" {
	#include "libplugin.h"
}

int jpilot_logf(int level, char *format, ...){
	FUNCTIONSETUP;
	return 0;
}


JPlugin::JPlugin(){
	FUNCTIONSETUP;
	init_info_null(&info);
	loaded=false;
}
JPlugin::JPlugin(QString fn) {
	FUNCTIONSETUP;
	init_info_null(&info);
	loaded=load(fn);
}
JPlugin::~JPlugin() {
	FUNCTIONSETUP;
	unload();
}

bool JPlugin::unload() {
	FUNCTIONSETUP;
	exit_info(&info);
	return true;
}

bool JPlugin::load(QString path) {
	FUNCTIONSETUP;
	if (loaded) unload();
	loaded=get_plugin_info(&info, path)>0;
	if (!loaded) {
		#ifdef DEBUG
		DEBUGCONDUIT<<"Error loading the plugin "<<path<<endl;
		#endif
		exit_info(&info);
	}
	return loaded;
}



void JPlugin::init_info_null(struct plugin_s *p) {
	FUNCTIONSETUP;
	loaded=false;
	lib = NULL;
	p->fullpath = QString::null;
	p->sync_on = 1;
	p->name = QString::null;
	p->db_name = QString::null;
	p->number = 0;
}

void JPlugin::exit_info(struct plugin_s *p) {
	FUNCTIONSETUP;
	if (loaded) exit_cleanup();
	if (loaded && lib) KLibLoader::self()->unloadLibrary(info.fullpath);
	init_info_null(p);
	loaded=false;
}


bool JPlugin::get_plugin_info(struct plugin_s *p, QString path) {
	FUNCTIONSETUP;
	void *h;
	const char *err;
	// use a string of length 100 to prevent a buffer overflow, although JPilot allows only 50 chars
	char name[100];
	int version, major_version, minor_version;
	T_versionM plugin_versionM;
	KLibLoader*ll=KLibLoader::self();


	init_info_null(p);
	// TODO: Remove the .so so that dependent libs are loaded, too!!!
	QString libname=path;
	if (path.right(3)==".so") libname=path.left(path.length()-3);
	lib = ll->library(libname);
//	h = dlopen(libname, RTLD_NOW);
	if (!lib) {
		#ifdef DEBUG
		DEBUGCONDUIT<<"open failed on plugin ["<<libname<<"]\n error ["<<ll->lastErrorMessage()<<"]"<<endl;
		#endif
		return false;
	}
	p->fullpath=lib->fileName();
	#ifdef DEBUG
	DEBUGCONDUIT<<"opened plugin ["<<libname<<"]"<<endl;
	#endif

	/* plugin_versionM */
	plugin_versionM = (T_versionM)(lib->symbol("plugin_version"));
	if (plugin_versionM==NULL) {
		err = ll->lastErrorMessage();
		#ifdef DEBUG
		DEBUGCONDUIT<<"plugin_version: ["<<err<<"],  plugin is invalid: ["<<libname<<"]"<<endl;
		#endif
		ll->unloadLibrary(p->fullpath);
		lib=NULL;
		return false;
	}
	plugin_versionM(&major_version, &minor_version);
	version=major_version*1000+minor_version;
	if ((major_version <= 0) && (minor_version < 99)) {
		#ifdef DEBUG
		DEBUGCONDUIT<<"Plugin:["<<libname<<"]: plugin version ("<<major_version<<"."<< minor_version<<" too old..."<<endl;
		#endif
		ll->unloadLibrary(libname);
		lib=NULL;
		return false;
	}
	#ifdef DEBUG
	DEBUGCONDUIT <<"This plugin is version ("<<major_version<<"."<<minor_version<<")"<<endl;
	#endif

	/* plugin_get_name */
	T_get_name plugin_get_name = (T_get_name)(lib->symbol("plugin_get_name"));
	if (plugin_get_name==NULL) {
		err = ll->lastErrorMessage();
		#ifdef DEBUG
		DEBUGCONDUIT<<"plugin_get_name: ["<<err<<"],  plugin is invalid: ["<<libname<<"]"<<endl;
		#endif
		ll->unloadLibrary(libname);
		lib=NULL;
		return false;
	} else {
		plugin_get_name(name, 50);
		name[50]='\0';
		p->name = name;
	}


	#ifdef DEBUG
	DEBUGCONDUIT <<"Before loading menu name"<<endl;
	#endif
	/* plugin_get_menu_name */
	T_get_menu_name plugin_get_menu_name = (T_get_menu_name)(lib->symbol("plugin_get_menu_name"));
	if (plugin_get_menu_name!=NULL) {
		plugin_get_menu_name(name, 50);
		#ifdef DEBUG
		DEBUGCONDUIT <<"Menu name function successfully loaded: "<<name<<endl;
		#endif
		name[50]='\0';
		p->menu_name = name;
	}

	#ifdef DEBUG
	DEBUGCONDUIT <<"Before loading help name"<<endl;
	#endif
	/* plugin_get_help_name */
	T_get_help_name plugin_get_help_name = (T_get_help_name)(lib->symbol("plugin_get_help_name"));
	if (plugin_get_help_name!=NULL) {
		plugin_get_help_name(name, 50);
		#ifdef DEBUG
		DEBUGCONDUIT <<"Help name function successfully loaded: "<<name<<endl;
		#endif
		name[50]='\0';
		p->help_name = name;
	}

		#ifdef DEBUG
		DEBUGCONDUIT <<"Before loading db name"<<endl;
		#endif
	/* plugin_get_db_name */
	name[0]='\0';
	T_get_db_name plugin_get_db_name = (T_get_db_name)(lib->symbol("plugin_get_db_name"));
	if (plugin_get_db_name!=NULL) {
		plugin_get_db_name(name, 50);
		name[50]='\0';
		#ifdef DEBUG
		DEBUGCONDUIT <<"DB name function successfully loaded: "<<name<<endl;
		#endif
	}
	p->db_name = name;
	
	#ifdef DEBUG
	DEBUGCONDUIT<<"Finished loading symbols from JPilot plugin ("<<libname<<")"<<endl;
	#endif
	return true;
}

bool JPlugin::hasGui() {
	FUNCTIONSETUP;
	void *func = lib->symbol("plugin_gui");
	return (func!=NULL);
}

/***************************************************************************
 * These functions just lookup the callback functions from the plugin
 * and execute it if available. If not, -1 is returned, but no crash
 * or exception *should* occur...
 ***************************************************************************/

int JPlugin::startup(jp_startup_info*si) {
	FUNCTIONSETUP;
	void *func = lib->symbol("plugin_startup");
	if (func) return ((T_startup)func)(si);
	else {
		#ifdef DEBUG
		DEBUGCONDUIT<<"Callback "<<fname<<" not found in plugin "<<info.name<<endl;
		#endif
		return -1;
	}
}

int JPlugin::gui(GtkWidget*vbox, GtkWidget*hbox, unsigned int uID) {
	FUNCTIONSETUP;
	void *func = lib->symbol("plugin_gui");
	if (func) return ((T_gui)func)(vbox, hbox, uID);
	else {
		#ifdef DEBUG
		DEBUGCONDUIT<<"Callback "<<fname<<" not found in plugin "<<info.name<<endl;
		#endif
		return -1;
	}
}

int JPlugin::gui_cleanup() {
	FUNCTIONSETUP;
	void *func = lib->symbol("plugin_gui_cleanup");
	if (func) return ((T_gui_cleanup)func)();
	else {
		#ifdef DEBUG
		DEBUGCONDUIT<<"Callback "<<fname<<" not found in plugin "<<info.name<<endl;
		#endif
		return -1;
	}
}

int JPlugin::help(char** text, int*width, int*height) {
	FUNCTIONSETUP;
	void *func = lib->symbol("plugin_help");
	if (func) return ((T_help)func)(text, width, height);
	else {
		#ifdef DEBUG
		DEBUGCONDUIT<<"Callback "<<fname<<" not found in plugin "<<info.name<<endl;
		#endif
		return -1;
	}
}

int JPlugin::pre_sync() {
	FUNCTIONSETUP;
	void *func = lib->symbol("plugin_pre_sync");
	if (func) return ((T_pre_sync)func)();
	else {
		#ifdef DEBUG
		DEBUGCONDUIT<<"Callback "<<fname<<" not found in plugin "<<info.name<<endl;
		#endif
		return -1;
	}
}

int JPlugin::sync(int sd) {
	FUNCTIONSETUP;
	void *func = lib->symbol("plugin_sync");
	if (func) return ((T_sync)func)(sd);
	else {
		#ifdef DEBUG
		DEBUGCONDUIT<<"Callback "<<fname<<" not found in plugin "<<info.name<<endl;
		#endif
		return -1;
	}
}

int JPlugin::post_sync() {
	FUNCTIONSETUP;
	void *func = lib->symbol("plugin_post_sync");
	if (func) return ((T_post_sync)func)();
	else {
		#ifdef DEBUG
		DEBUGCONDUIT<<"Callback "<<fname<<" not found in plugin "<<info.name<<endl;
		#endif
		return -1;
	}
}

int JPlugin::exit_cleanup() {
	FUNCTIONSETUP;
	void *func = lib->symbol("plugin_exit_cleanup");
	if (func) return ((T_exit_cleanup)func)();
	else {
		#ifdef DEBUG
		DEBUGCONDUIT<<"Callback "<<fname<<" not found in plugin "<<info.name<<endl;
		#endif
		return -1;
	}
}

