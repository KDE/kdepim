#ifndef KPILOT_PLUGIN_H
#define KPILOT_PLUGIN_H
/* KPilot
**
** Copyright (C) 2001 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2002-2004,2006 Adriaan de Groot <groot@kde.org>
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

// KPilot headers
#include "pilotLocalDatabase.h"
#include "syncAction.h"

// Qt headers
#include <QtCore/QVariantList>

/** @file
* This file defines the base class of all KPilot conduit plugins configuration
* dialogs. This is necessary so that we have a fixed API to talk to from
* inside KPilot.
*
* The factories used by KPilot plugins are also documented here.
*/


class KTabWidget;
class KAboutData;

class PilotDatabase;

namespace Pilot
{
	/**
	* As the API for conduits may change in the course of time,
	* identify them and refuse to load incompatible API versions.
	* Bump this number every release to the current YYYYMMDD
	* value.
	*/
	static const unsigned int PLUGIN_API = 20090107;
}

/**
* ConduitConfigBase is for configuration purposes.
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
class KPILOT_EXPORT ConduitConfigBase : public QObject
{
Q_OBJECT
public:
	/**
	* Constructor. Creates a conduit configuration support object
	* with the given parent @p parent and name (optional) @p n.
	*/
	explicit ConduitConfigBase( QWidget *parent = 0L
		, const QVariantList &args = QVariantList() );

	/** Destructor. */
	virtual ~ConduitConfigBase();

	/**
	* This function is called to check whether the configuration
	* of the conduit has changed -- and hence, whether the user
	* needs to be prompted. By default, this just returns
	* fModified, but you can do more complicated things.
	*/
	virtual bool isModified() const
	{
		return fModified;
	} ;

	/** Accessor for the actual widget for the configuration. */
	QWidget *widget() const
	{
		return fWidget;
	} ;

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

	QString conduitName() const { return fConduitName; } ;

	/**
	* This is the function that does the work of adding an about
	* page to a tabwidget. It is made public and static so that
	* it can be used elsewhere wherever tabwidgets appear.
	*
	* The about tab is created using aboutPage(). The new about
	* widget is added to the tab widget @p w with the heading
	* "About".
	*
	* @param w The tab widget to which the about page is added.
	* @param data The KAboutData that is used.
	*
	*/
	static  void addAboutPage(KTabWidget *w,
		KAboutData *data=0L);

	/**
	* This creates the actual about widget. Again, public & static so
	* you can slap in an about widget wherever.
	*
	* An about widget is created that shows the contributors to
	* the application, along with copyright information and the
	* application's icon. This widget can be used pretty much
	* anywhere. Copied from KAboutDialog, mostly.
	*
	* @param parent The widget that holds the about widget.
	* @param data The KAboutData that is used to populate the widget.
	*/
	static QWidget *aboutPage(QWidget *parent, KAboutData *data=0L);

protected:
	/**
	* This function provides the string for the prompt used
	* in maybeSave(). Override it to change the text.
	*/
	virtual QString maybeSaveText() const;

	void unmodified() { fModified=false; } ;

	bool fModified;
	QWidget *fWidget;
	QString fConduitName;


protected Q_SLOTS:
	void modified();
Q_SIGNALS:
	void changed(bool);

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

class KPILOT_EXPORT ConduitAction : public SyncAction
{
Q_OBJECT

public:
	ConduitAction(KPilotLink *,
		const char *name,
		const QVariantList &args = QVariantList());
	virtual ~ConduitAction();

	QString conduitName() const { return fConduitName; } ;

	/** Retrieve the sync mode set for this action. */
	const SyncMode &syncMode() const { return fSyncDirection; };

	/**
	* A full sync happens for eFullSync, eCopyPCToHH and eCopyHHToPC. It
	* completely ignores all modified flags and walks through all records
	* in the database.
	*/
	bool isFullSync() const
	{
		return fFirstSync || fSyncDirection.isFullSync() ;
	}

	/**
	* A first sync (i.e. database newly fetched from the handheld )
	* does not check for deleted records, but understands them as
	* added on the other side. The flag is set by the conduits
	* when opening the local database, or the calendar/addressbook
	* (if it is empty). This also implies a full sync.
	*/
	bool isFirstSync() const
	{
		return fFirstSync || fSyncDirection.isFirstSync() ;
	}

protected:
	/** Retrieve the conflict resolution setting for this action. */
	ConflictResolution getConflictResolution() const
		{ return fConflictResolution; };

	/** Try to change the sync mode from what it is now to the mode @p m.
	* This may fail (ie. changing a backup to a restore is not kosher) and
	* changeSync() will return false then.
	*/
	bool changeSync(SyncMode::Mode m);

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

	void setFirstSync(bool first) { fFirstSync=first; } ;

	PilotDatabase *fDatabase;
	PilotLocalDatabase *fLocalDatabase; // Guaranteed to be a PilotLocalDatabase

	/**
	* Open both the local copy of database @p dbName
	* and the version on the Pilot. Return true only
	* if both opens succeed. If the local copy of the database
	* does not exist, it is retrieved from the handheld. In this
	* case, retrieved is set to true, otherwise it is left alone
	* (i.e. retains its value and is not explicitly set to false).
	*
	* @param dbName database name to open.
	* @param retrieved indicator whether the database had to be loaded
	*        from the handheld.
	*/
	bool openDatabases(const QString &dbName, bool*retrieved=0L);

	/**
	* Name of the conduit; might be changed by subclasses. Should
	* normally be set in the constructor.
	*/
	QString fConduitName;

private:
	SyncMode fSyncDirection;
	ConflictResolution fConflictResolution;

	bool fFirstSync;
} ;

/**
* The ConduitProxy action delays loading the plugin for a conduit until the conduit
* actually executes; the proxy then loads the file, creates a SyncAction for the conduit
* and runs that. Once the conduit has finished, the proxy unloads everything
* and emits syncDone().
*/
class ConduitProxy : public ConduitAction
{
Q_OBJECT

public:
	ConduitProxy(KPilotLink *,
		const QString &desktopName,
		const SyncAction::SyncMode &m);

protected:
	virtual bool exec();
protected Q_SLOTS:
	void execDone(SyncAction *);

protected:
	QString fDesktopName;
	QString fLibraryName;
	ConduitAction *fConduit;
} ;


#endif
