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
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/
 
/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <qstringlist.h>

#include "uiDialog.h"
#include "syncAction.h"

class KConfig;
class PilotDatabase;

class ConduitConfigBase : public QObject
{
Q_OBJECT
public:
	ConduitConfigBase(QWidget *parent=0L, const char *n=0L);
	virtual ~ConduitConfigBase();

	bool isModified() const { return fModified; } ;
	QWidget *widget() const { return fWidget; } ;

public:
	/** 
	* Load or save the config widget's settings in the given 
	* KConfig object; leave the group unchanged.
	*/
	virtual void commit(KConfig *) = 0L;
	virtual void load(KConfig *) = 0L;
	/**
	* Called when the object is to be hidden again and might
	* need to save changed settings. Should prompt the user
	* and call commit() if needed. Override this function only
	* if you need a very different kind of prompt window.
	*
	* Returns false if the change is to be canceled. Returns
	* true otherwise, whether or not the changes were saved.
	*/
	virtual bool maybeSave(KConfig *);
	/**
	* This function provides the string for the prompt used
	* in maybeSave(). Override it to change the text.
	*/
	virtual QString maybeSaveText() const;

	virtual QString conduitName() const;

protected slots:
	void modified();

protected:
	bool fModified;
	QWidget *fWidget;
} ;

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

	// User-readable name of the conduit. Should match
	// the other conduitName() methods in other classes
	// in this file.
	virtual QString conduitName() const;
protected:
	KConfig *fConfig;
} ;

/**
* The SyncActions created by the factory should obey at least
* the argument test, indicating a dry run. The device link is
* the link where the sync should run -- don't get the pilotPort()
* until the sync runs!
*
* setConfig() will be called before the sync starts so that the
* conduit can read/write metadata and local settings.
*/

class ConduitAction : public SyncAction
{
Q_OBJECT
public:
	ConduitAction(KPilotDeviceLink *,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~ConduitAction();

	void setConfig(KConfig *c) { fConfig=c; } ;

	virtual QString conduitName() const;
protected:
	bool isTest() const { return fTest; } ;
	bool isBackup() const { return fBackup; } ;
	bool isLocal() const { return fLocal; } ;

	KConfig *fConfig;
	PilotDatabase *fDatabase,*fLocalDatabase;

	/**
	* See openDatabases_ for info on the @p retrieved
	* parameter. In --local mode, retrieved is left
	* unchanged.
	*/
	bool openDatabases(const QString &dbName, bool*retrieved=0L);

private:
	bool fTest;	// Do some kind of test run on the pilot
	bool fBackup;	// Do a backup of the database
	bool fLocal;	// Local test without a Pilot

	/**
	* Open both the local copy of database @p dbName
	* and the version on the Pilot. Return true only
	* if both opens succeed. If the local copy of the database
	* does not exist, it is retrieved from the handheld. In this
	* case, retrieved is set to true, otherwise it is left alone
	* (i.e. retains its value and is not explicitly set to false).
	*/
	bool openDatabases_(const QString &dbName, bool*retrieved=0L);

	/**
	* Open both databases, but get the fDatabase not from
	* the Pilot, but from a local database in an alternate
	* directory. For testing only.
	*
	* If @p localPath is QString::null, don't even try to open
	* fDatabase. Just open the local one.
	*/
	bool openDatabases_(const QString &dbName,const QString &localPath);
} ;

class PluginUtility
{
public:
	static int findHandle(const QStringList &);
	static bool isModal(const QStringList &a);

	/**
	* This function attempts to detect whether or not the given
	* application is running. If it is, true is returned, otherwise
	* false.
	*
	* The current approach is to ask the DCOP server if the application
	* has registered.
	*/
	static bool isRunning(const QCString &appName);
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

#endif
