/***************************************************************************
                          jplugin.h  -  description
                             -------------------
    begin                : Sat Mar 16 2002
    copyright            : (C) 2002 by reinhold
    email                : reinhold@albert
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JPLUGIN_H
#define JPLUGIN_H

#include <qstring.h>
#include <stdlib.h>
#include <stdio.h>
#include <klibloader.h>

extern "C" {
#undef signals /* GTK uses "signals" as variable name :( */
#include "libplugin.h"
//#include <gtk/gtk.h>
#define signals protected /* GTK is done messing up ;) */

//typedef int GtkWidget;


/**
  *@author reinhold
  */

  // type definitions of the callback functions in the JPIlot plugins
typedef int (*T_get_name)(char *name, int len);
typedef int (*T_get_menu_name)(char *name, int len);
typedef int (*T_get_help_name)(char *name, int len);
typedef int (*T_get_db_name)(char *db_name, int len);
typedef int (*T_startup)(jp_startup_info *info);
typedef int (*T_gui)(GtkWidget *vbox, GtkWidget *hbox, unsigned int unique_id);
typedef int (*T_help)(char **text, int *width, int *height);
typedef int (*T_print)();
typedef int (*T_gui_cleanup)(void);
typedef int (*T_pre_sync)(void);
typedef int (*T_sync)(int sd);
typedef int (*T_search)(char *search_string, int case_sense, struct search_result **sr);
typedef int (*T_post_sync)(void);
typedef int (*T_exit_cleanup)(void);
typedef void (*T_versionM)(int *major_version, int *minor_version);

}


/* this structure holds *ALL* information about the dynamically linked library (aka. plugin)
   the references to the callback functions are retrieved when they are really needed */
struct plugin_s {
	bool sync_on;
	unsigned char user_only;
	QString fullpath;
	QString name;
	QString menu_name;
	QString help_name;
	QString db_name;
	int number;
};


class JPlugin {
public:
	JPlugin();
	JPlugin(QString filename);
	~JPlugin();
	bool unload();
	bool load(QString fn);
	void init_info_null(struct plugin_s *p);
	void exit_info(struct plugin_s *);
	bool get_plugin_info(struct plugin_s *p, QString path);
	
	int startup(jp_startup_info*si);
	int gui(GtkWidget*vbox, GtkWidget*hbox, unsigned int uID);
	int gui_cleanup();
	int help(char** text, int*width, int*height);
	int pre_sync();
	int sync(int sd);
	int post_sync();
	int exit_cleanup();
	bool hasGui();
	
	bool loaded;
	plugin_s info;
	KLibrary *lib;
};




#endif
