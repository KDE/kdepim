#ifndef _KPILOT_PLUGIN_H
#define _KPILOT_PLUGIN_H
/* plugin.h                             KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the base class of all KPilot conduit plugins configuration
** dialogs. This is necessary so that we have a fixed API to talk to from
** inside KPilot.
**
** The factories used by KPilot plugins are also documented here.
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

#include <qstringlist.h>

#include "uiDialog.h"
#include "syncAction.h"

class KConfig;

/**
* Dialogs that are created by the factory take a stringlist of
* arguments. They interpret at least an argument of "modal",
* and possible others (depending on the conduit).
*
* The config file to be read is set after creationg, but before
* show()ing the dialog.
*/
class ConduitConfig : public UIDialog
{
Q_OBJECT

public:
	ConduitConfig(QWidget *parent=0L,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~ConduitConfig();

	void setConfig(KConfig *c) { fConfig=c; } ;

	virtual void readSettings() = 0 ;
	/* and commit changes, too! */

protected:
	KConfig *fConfig;
} ;

/**
* The SyncActions created by the factory should obey at least
* the argument test, indicating a dry run.
*/

class ConduitAction : public SyncAction
{
Q_OBJECT
public:
	ConduitAction(KPilotDeviceLink *,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~ConduitAction();

protected:
	bool isTest() const { return fTest; } ;

private:
	bool fTest;
} ;

class PluginUtility
{
public:
	static int findHandle(const QStringList &);
	static bool isModal(const QStringList &a);
} ;

/**
* All KPilot conduits should subclass KLibfActory like this.
*
* Boilerplate for inheritance:
*
* <pre>
* class KPilotPlugin : public KLibFactory
* {
* Q_OBJECT
* 
* public:
* 	KPilotPlugin(QObject * = 0L,const char * = 0L) ;
* 	virtual ~KPilotPlugin();
* </pre>
*
* You don't @em have to provide about information for the plugin,
* but it's useful, particularly for the about box in a conduit.
*
* 
* <pre>
* 	static KAboutData *about() { return fAbout; } ;
* </pre>
*
* 
* This is what it's all about: creating objects for the plugin.
* One classname that @em must be supported is ConduitConfig,
* which is defined above. The other is SyncAction.
*
*
* <pre>
* protected:
* 	virtual QObject* createObject( QObject* parent = 0, 
* 		const char* name = 0, 
* 		const char* classname = "QObject", 
* 		const QStringList &args = QStringList() );
* </pre>
* 
* More boilerplate, and support for an instance and about data, used
* by about() above.
*
* <pre>
* 	KInstance *fInstance;
* 	static KAboutData *fAbout;
* } ;
* </pre>
*/

// $Log$

#endif
