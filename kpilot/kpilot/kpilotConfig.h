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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
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
	int get##a(QComboBox *p=0L); \
	void set##a(QComboBox *); \
	void set##a(int); \

#define BoolProperty(a) \
	bool get##a(QCheckBox *p=0L); \
	void set##a(QCheckBox *); \
	void set##a(bool);

#define StringProperty(a) \
	QString get##a(QLineEdit *p=0L); \
	void set##a(QLineEdit *); \
	void set##a(const QString &);

	IntProperty(Version)
	IntProperty(Debug)


	IntProperty(PilotType)
	IntProperty(PilotSpeed)
	StringProperty(PilotDevice)
	StringProperty(User)


	BoolProperty(StartDaemonAtLogin)
	BoolProperty(KillDaemonOnExit)
	BoolProperty(DockDaemon)

	BoolProperty(ShowSecrets)
	BoolProperty(SyncFiles)
	StringProperty(BackupOnly)
	StringProperty(Skip)

	// Address widget stuff goes in a different group
	//
	//
	KPilotConfigSettings &setAddressGroup();

	BoolProperty(UseKeyField)
	IntProperty(AddressDisplayMode)



#undef StringProperty
#undef BoolProperty
#undef IntProperty

	// Conduit configuration information
	//
	//
	KPilotConfigSettings &setConduitGroup();

	QStringList getInstalledConduits();
	void setInstalledConduits(const QStringList &);

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



// $Log$
// Revision 1.8  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.7  2001/09/23 21:44:56  adridg
// Myriad small changes
//
// Revision 1.6  2001/09/23 18:25:50  adridg
// New config architecture
//
// Revision 1.5  2001/09/05 21:53:51  adridg
// Major cleanup and architectural changes. New applications kpilotTest
// and kpilotConfig are not installed by default but can be used to test
// the codebase. Note that nothing else will actually compile right now.
//
// Revision 1.4  2001/04/16 13:48:35  adridg
// --enable-final cleanup and #warning reduction
//
// Revision 1.3  2001/03/27 23:54:43  stern
// Broke baseConduit functionality out into PilotConduitDatabase and added support for local mode in BaseConduit
//
// Revision 1.2  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.1  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
#endif
