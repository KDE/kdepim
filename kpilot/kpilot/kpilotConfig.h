#ifndef _KPILOT_KPILOTCONFIG_H
#define _KPILOT_KPILOTCONFIG_H
/* kpilotConfig.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This class concentrates all the configuration
** information for the various parts of KPilot.
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


#include <ksimpleconfig.h>

class KCmdLineArgs;
class QLineEdit;
class QComboBox;
class QCheckBox;

class KPilotConfigSettings : public KSimpleConfig
{
public:
	KPilotConfigSettings(const QString &filename,bool readonly=false);
	virtual ~KPilotConfigSettings();

	// All the functions in this class follow one of the patterns
	//
	// getXXX() read the config file and return the value.
	//        If there are parameters, set the value of those parameters
	//        to the value read.
	// setXXX() write the value to the config file. The value might
	//        be taken from a variety of widgets of data types.
	//
	// For example, the following would be typical of a string that
	// can be entered in a line edit:
	//
	// QString getPilotDevice(QLineEdit *p=0L);
	// void setPilotDevice(QLineEdit *);
	// void setPilotDevice(const QString &);
	//
	// Since the boilerplate is straightfoward, I use macros to
	// define String, Bool and Int properties.
	//
	//

#define IntProperty(a) \
	int get##a() const; \
	void set##a(int); \

#define BoolProperty(a) \
	bool get##a() const; \
	void set##a(bool);

#define StringProperty(a) \
	QString get##a() const; \
	void set##a(const QString &);

	IntProperty(Version)
	IntProperty(Debug)

	/* General tab in the config dialog */
	StringProperty(PilotDevice)
	IntProperty(PilotSpeed)
	StringProperty(Encoding)
	StringProperty(User)
	BoolProperty(StartDaemonAtLogin)
	BoolProperty(DockDaemon)
	BoolProperty(KillDaemonOnExit)

	/* Sync tab */
	IntProperty(SyncType)
	BoolProperty(FullSyncOnPCChange)
	IntProperty(ConflictResolution)
	BoolProperty(SyncFiles)
	BoolProperty(SyncWithKMail)

	/* Viewers tab */
	BoolProperty(InternalEditors)
	BoolProperty(ShowSecrets)
	// Address widget stuff goes in a different group
	KPilotConfigSettings &setAddressGroup();
	IntProperty(AddressDisplayMode)
	BoolProperty(UseKeyField)

	/* Backup tab */
	StringProperty(BackupOnly)
	StringProperty(Skip)




#undef StringProperty
#undef BoolProperty
#undef IntProperty

	// Conduit configuration information
	//
	//
	KPilotConfigSettings &setConduitGroup();

	QStringList getInstalledConduits();
	void setInstalledConduits(const QStringList &);

	QStringList getDirtyDatabases();
	void setDirtyDatabases(const QStringList &);
	void addDirtyDatabase(QString db);

	QStringList getAppBlockChangedDatabases();
	void setAppBlockChangedDatabases(const QStringList &);
	void addAppBlockChangedDatabase(QString db);

	QStringList getFlagsChangedDatabases();
	void setFlagsChangedDatabases(const QStringList &);
	void addFlagsChangedDatabase(QString db);

	KPilotConfigSettings &setDatabaseGroup();
	void setDatabaseConduit(const QString &database,const QString &conduit);

public:
	// Re-export some useful functions from KConfig
	//
	//
	void sync() { KSimpleConfig::sync(); } ;
	KPilotConfigSettings & resetGroup()
		{ setGroup(QString::null); return *this; } ;

} ;

class KPilotConfig
{
public:
	/**
	* Returns a (new) reference to the KPilot configuration object.
	* This is used to put all the KPilot configuration --
	* including conduits and such -- into one rc file and
	* not spread out among config files for each conduit.
	*
	* Callers should @em never delete this object.
	* @em Only call this after the KApplication object has been
	* created, or the program will crash (SIGSEGV in KDE 2.1,
	* qFatal() with a sensible message in KDE 2.2).
	*/
	static KPilotConfigSettings& getConfig();

        /**
	 * @return QString of default path for the BackupDB files
	 * are located
	 */
        static QString getDefaultDBPath();


	/**
	* This number can be changed every time a new
	* KPilot version is released that absolutely requires
	* the user to take a look at the configuration of
	* KPilot.
	*/
	static const int ConfigurationVersion;

	/**
	* Reads the configuration version from a configuration file.
	* TODO: Make this use the *standard* location.
	*/
	static int getConfigVersion(KConfig *);
	static int getConfigVersion(KConfig&);

	/**
	* Write the current configuration version to the standard
	* location. @em Only call this after the KApplication object
	* is created, or crashes will result.
	*/
	static void updateConfigVersion();

	/**
	* We might have an additional Debug= line in their
	* config which may be read and ORed with the user-specified
	* debug level. This function does that.
	*
	* Note that this will SIGSEGV if there is no KApplication
	* instance (yet) since it uses functions from there. @em Only
	* call this after the KApplication object has been created.
	*
	* @ret resulting debug level
	*/
	static int getDebugLevel(bool useDebugId=true);


	/**
	* Returns the user's preference for the system-wide
	* fixed font.
	*/
	static const QFont& fixed() ;

protected:
	static int getDebugLevel(KPilotConfigSettings &);
} ;



#endif
