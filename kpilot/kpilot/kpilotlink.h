/* kpilotlink.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is a horrible class that implements three different
** functionalities and that should be split up as soon as 2.1 
** is released. See the .cc file for more.
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
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#ifndef __KPILOT_LINK
#define __KPILOT_LINK


#include <qobject.h>

class QWidget;
class KStatusBar;
class KSocket;
class KServerSocket;
class KProgress;
class KProcess;

class PilotRecord;
class MessageDialog;

class PilotSerialDatabase;
class PilotLocalDatabase;

#include "pilotUser.h"
#include "pilotDatabase.h"
#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"

/**
  * This class is an attempt to provide some wrapper around the pilot-link
  * library.
  */

// Struct by Kenneth Albanowski
struct db 
    {
    char name[256];
    int flags;
    unsigned long creator;
    unsigned long type;
    int maxblock;
    };

#define pi_mktag(c1,c2,c3,c4) (((c1)<<24)|((c2)<<16)|((c3)<<8)|(c4))

class KPilotLink : public QObject
{
  Q_OBJECT
  
  friend class PilotSerialDatabase;

public:
  /**
   * Used for opening local databases only.
   */
  KPilotLink(); // Used only for local databases

  /**
   * Creates a pilot link that can sync to the pilot.
   * devicePath = path to serial port.  Defaults to /dev/pilot
   */
  KPilotLink(QWidget* owner, KStatusBar* statusBar = 0L, 
		const QString &devicePath = QString::null);
  ~KPilotLink();
  
  /**
   * Commands sent from KPilot requesting the daemon
   * to perform those actions.
   */
	enum Commands { Backup, Restore, 
		HotSync, FastSync,
		InstallFile, TestConnection 
		};
  
  /**
   * Static method to get the current link..
   */
  static KPilotLink* getPilotLink() { return fKPilotLink; }
  
  /**
   * Begin a hot-sync session with a palm pilot
   */
  void startHotSync();

  /**
   * End a hot-sync session with a palm pilot
   */
  void endHotSync();
  
  /**
   * True if hot-sync has been started
   */
  bool getConnected() const { return fConnected; }

  /**
   * Returns the user information as set in the KPilot settings dialog.
   */
  KPilotUser& getPilotUser() { return fPilotUser; }

  /**
   * Changes the path (port) that the link should work with.
   */
  void changePilotPath(const QString& devicePath) 
	{ initPilotSocket(devicePath); }

  /**
   * This is merely a flag for any widget that should run.  Will be set by KPilot when doing
   * a full backup, then reset after that.  Mostly obsolete since conduits are being used.
   */
  void setSlowSyncRequired(bool yesno) { fSlowSyncRequired = yesno; }

  /** 
   * Returns the whether the widget should do a full or partial backup.  Mostly obsolete now
   * that conduits are being used.
   */
  bool slowSyncRequired() const { return fSlowSyncRequired; }

  void setFastSyncRequired(bool yesno) { fFastSyncRequired = yesno; } 
  bool fastSyncRequired() const { return fFastSyncRequired; } 
  
  /**
   * Backups all databases to BACKUP_DIR/username
   */
  void doFullBackup();  

  /**
   * Restores all databases to the pilot from BACKUP_DIR/username
   */
  bool doFullRestore(); 
  
  /**
   * Installs all files found in installPath on the pilot
   */
  void installFiles(const QString &installPath);
  
  /* CONDUIT SYNCING SUPPORT */

  /**
   * Begins HotSyncing modified data.  Emits databaseSyncComplete() when
   * finished.
   */
  void quickHotSync();

  /**
   * Checks to see if there is a conduit registered for database dbName.
   * returns the name if yes, NULL if no.
   */
  QString registeredConduit(const QString &dbName);

  /**
   * Uses BACKUP_DIR/username as local source, and compares records versus those on the pilot
   * in databaseName.  When finished, BACKUP_DIR/username/databaseName.pdb (or prc) 
   * will contain the same data as the pilot.
   */
  bool syncDatabase(DBInfo* database);
  

private:
	/**
	* For initializing the link: read the configuration
	* file and save vital information from there.
	*/
	void readConfig();

  PilotRecord* readRecord(KSocket*);
  void writeRecord(KSocket*, PilotRecord*);

	/**
	* Write a single-word response message to the command
	* connection. This is typically used to respond to 
	* requests from conduits. Pass one of the CStatusMessages.
	*/
	int writeResponse(KSocket *, int m);

  /**
   * Starts syncing the next (could be first) database, either by firing up a conduit, or
   * by using syncDatabase() to back it up into the BACKUP_DIR.  Uses
   * fNextDBIndex as the next DB to sync, and increments it.
   */
	int findNextDB(struct DBInfo *);
  void syncNextDB();
  void doConduitBackup();
	/**
	* This function is called by slotConduitCLosed and
	* some other places to resume processing of databases
	* for a backup or sync.
	*/
	void resumeDB();

	/**
	* When the sync is done (or the backup, or whatever ...)
	* call this function to cleanup and emit done signals.
	*/
	void finishDatabaseSync();


public:
	typedef enum { Normal, PilotLinkError } Status ;

	Status status() const { return fStatus; } ;

public slots:
  void slotConduitRead(KSocket*);
  void slotConduitClosed(KSocket*);
  void slotConduitConnected(KSocket*);
	void slotConduitDone(KProcess *);
 /* END CONDUIT SYNCING SUPPORT */

private:
	/**
	* This is en enum listing the states that a running conduit
	* may be in. This is used to detect conduits that crash or
	* otherwise misbehave.
	*/
	enum { None = 0 , Running, Connected, Done } fConduitRunStatus ;

public:
#if 0
  /**
   * Opens database "database" on Pilot and returns the id to reference it:
   */
  PilotSerialDatabase* openDatabase(char* database) { return new PilotSerialDatabase(this, database); }

  /**
   * Opens database "database" locally and returns the handle to it:
   */
  PilotLocalDatabase* openLocalDatabase(const QString &database);
	// { 
	//	return new PilotLocalDatabase(kapp->localkdedir() +  
	//		BACKUP_DIR + 
	//		getPilotUser().getUserName() + 
	//		"/", database); 
	// }

  /**
   * Closes database 'database'.
   */
  void closeDatabase(PilotDatabase* database) { delete database; }
#endif

  /**
   * Write a log entry to the pilot. Note that the library
   * function takes a char *, not const char * (which is
   * highly dubious).
   */
  void addSyncLogEntry(const char* entry) 
	{ dlp_AddSyncLogEntry(getCurrentPilotSocket(), (char *)entry); }
  
  void createNewProgressBar(QString title, QString text, int min, int max, int value);
  void updateProgressBar(int value) const;
  void destroyProgressBar();
  
  /**
   * Displays a dialog showing error message.
   */
  void showMessage(const QString &message) const;
  
protected:
  void setConnected(bool connected) { fConnected = connected; }
  
  int  getPilotMasterSocket() const { return fPilotMasterSocket; }
  int  getCurrentPilotSocket() const { return fCurrentPilotSocket; }

  /** 
   * Compares the user name in getPilotUser() to the one saved by the KPilot
   * GUI.  If they don't match, check with the user to see if we should
   * make them match.
   * When this routine returns all the user info in getPilotUser will be 
   * as correct as the user wants it.
   */
  void checkPilotUser();

  QWidget* getOwningWidget() { return fOwningWidget; }
  
  QString    fPilotPath;  // defaults to /dev/pilot
  bool       fConnected;
  
private:
  static KPilotLink* fKPilotLink; // Static variable to ourself.
  static const QString BACKUP_DIR; // The directory backedup databases go..
  
  void syncFlags();          // Sets last hotsync time on pilot
  bool createLocalDatabase(DBInfo* info); // Creates a new local DB in BACKUP_DIR
  
	/**
	* findDisposition is used to see if the database 
	* d is named in the (comma separated) list s.
	* This is currently based only on the creator of
	* the database.
	*/
	int findDisposition(const QString &s,const struct DBInfo *d);

  int compare(struct db* d1, struct db* d2); // Compares two database infos..
  void initConduitSocket();         // Sets up fConduitSocket
  void initPilotSocket(const QString& devicePath, bool inetconnection=false);

  int         fPilotMasterSocket;   // This is the master one created with the class
  int         fCurrentPilotSocket;  // This changes with each connect()/disconnect()
  bool        fSlowSyncRequired;
	bool fFastSyncRequired;
  QWidget*    fOwningWidget;
  KStatusBar* fStatusBar;           // Optional
  QDialog*    fProgressDialog;
  KProgress*  fProgressMeter;
  KPilotUser  fPilotUser;           // Pilot User Info
  KServerSocket* fConduitSocket;    // Socket conduits connect on
  PilotDatabase* fCurrentDB;           // Currently Open Database (for conduit)
  DBInfo      fCurrentDBInfo;          // Currently open Database information
  int         fNextDBIndex;         // The next DB to sync
  KProcess*    fConduitProcess;
  MessageDialog* fMessageDialog;    // The dialog showing current status

	Status fStatus;
	QString fDatabaseDir;

signals:
  void databaseSyncComplete();
  void syncingDatabase(char*);
//   void recordModified(PilotRecord*);
//   void recordDeleted(PilotRecord*);
};

#endif

#undef REALLY_KPILOTLINK

// $Log: $
