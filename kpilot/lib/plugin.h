#ifndef _KPILOT_PLUGIN_H
#define _KPILOT_PLUGIN_H
/* plugin.h                             KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2002-2004 by Adriaan de Groot
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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

class PilotDatabase;
class KLibrary;

#define KPILOT_PLUGIN_API	(20041002)

/**
* The first classe here: ConduitConfigBase is for configuration purposes.
*
* ConduitConfigBase: this is an object (with a widget!) that is embedded
* in a dialog. This is the currently preferred form for configuration,
* and it's what is used in the KPilot conduit configuration dialog.
* The factory is asked for a "ConduitConfigBase" object.
*
* NB. The reason that this is a QObject which needs to create a
* separate widget - instead of a QWidget subclass - has to do with
* layouting. If you make the widget with designer then the easiest
* thing to do is to use a grid layout there. Making ConduitConfigBase
* a QWidget subclass would require an additional layout here, which
* seems a little foolish.
*
*/

class KDE_EXPORT ConduitConfigBase : public QObject
{
Q_OBJECT
public:
	ConduitConfigBase(QWidget *parent=0L, const char *n=0L);
	virtual ~ConduitConfigBase();

	/**
	* This function is called to check whether the configuration
	* of the conduit has changed -- and hence, whether the user
	* needs to be prompted. By default, this just returns
	* fModified, but you can do more complicated things.
	*/
	virtual bool isModified() const { return fModified; } ;
	QWidget *widget() const { return fWidget; } ;

public:
	/**
	* Load or save the config widget's settings in the given
	* KConfig object; leave the group unchanged. load() and
	* commit() should both call unmodified() to indicate that
	* the current settings match the on-disk ones.
	*/
	virtual void commit() = 0;
	virtual void load() = 0;
	/**
	* Called when the object is to be hidden again and might
	* need to save changed settings. Should prompt the user
	* and call commit() if needed. Override this function only
	* if you need a very different kind of prompt window.
	*
	* Returns false if the change is to be canceled. Returns
	* true otherwise, whether or not the changes were saved.
	*/
	virtual bool maybeSave();
protected:
	/**
	* This function provides the string for the prompt used
	* in maybeSave(). Override it to change the text.
	*/
	virtual QString maybeSaveText() const;

public:
	QString conduitName() const { return fConduitName; } ;

protected slots:
	void modified();
signals:
	void changed(bool);

protected:
	bool fModified;
	QWidget *fWidget;
	QString fConduitName;

	void unmodified() { fModified=false; } ;
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

class KDE_EXPORT ConduitAction : public SyncAction
{
Q_OBJECT
public:
	ConduitAction(KPilotDeviceLink *,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~ConduitAction();

	QString conduitName() const { return fConduitName; } ;

protected:
	bool isTest() const { return fTest; } ;
	bool isBackup() const { return fBackup; } ;
	bool isLocal() const { return fLocal; } ;

	int getSyncDirection() const { return fSyncDirection; };
	ConflictResolution getConflictResolution() const
		{ return fConflictResolution; };

	// Set the conflict resolution, except if the resolution
	// form is UseGlobalSetting, in which case nothing changes
	// (assumes then that the resolution form is already set
	// according to that global setting).
	//
	void setConflictResolution(ConflictResolution res)
	{
		if (SyncAction::eUseGlobalSetting != res)
			fConflictResolution=res;
	}
	void setSyncDirection(SyncMode dir)
		{ fSyncDirection=dir; }

	/**
	* A full sync happens for eFullSync, eCopyPCToHH and eCopyHHToPC. It
	* completely ignores all modified flags and walks through all records
	* in the database.
	*/
	bool isFullSync() const
	{
		return fFirstSync ||
			( (fSyncDirection==SyncAction::eFullSync  ) ||
			(fSyncDirection==SyncAction::eCopyPCToHH) ||
			(fSyncDirection==SyncAction::eCopyHHToPC) ) ;
	}

	/**
	* A first sync (i.e. database newly fetched from the handheld )
	* does not check for deleted records, but understands them as
	* added on the other side. The flag is set by the conduits
	* when opening the local database, or the calendar/addressbook
	* (if it is empty). This also implies a full sync.
	*/
	bool isFirstSync() const {
		return fFirstSync ||
		(fSyncDirection==SyncAction::eCopyHHToPC) ||
		(fSyncDirection==SyncAction::eCopyPCToHH); };

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
	SyncMode fSyncDirection; // Stores fast, full, PCToHH or HHToPC
	ConflictResolution fConflictResolution;

	// Make these only protected so the conduit can change the variable
protected:
	bool fFirstSync;
	QString fConduitName;

private:
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

class KDE_EXPORT PluginUtility
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

	/**
	* Check a given library for its version, returning 0 if no
	* version symbol is found.
	*/
	static long pluginVersion(const KLibrary *);
	static QString pluginVersionString(const KLibrary *);
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
*
*
*
* The implementation of a conduit needs an init_conduit_name() function,
* just like any KLibLoader library that uses factories.
*
* The createObject() function needs to support at least two creation
* calls: "ConduitConfigBase" and "SyncAction".
* "ConduitConfigBase" should return a subclass of ConduitConfigBase,
* "SyncAction" a subclass of SyncAction.
*
* Finally, a conduit should have a symbol version_conduit_name,
* that returns a long; much like the init_conduit_name() function. This
* should return the version of the plugin API (KPILOT_PLUGIN_VERSION)
* the conduit was compiled against. Additionally, a plugin may have a
* id_conduit_name, which should be a const char *.
*
*/

#endif
