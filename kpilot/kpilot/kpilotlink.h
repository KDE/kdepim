// kpilotLink.h
//
// Copyright (C) 1998,1999 Dan Pilone
// Copyright (C) 2000 Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// This is the version of kpilotLink.h for KDE2 / KPilot 4.
//
//


#ifndef __KPILOT_LINK
#define __KPILOT_LINK

#if (QT_VERSION > 199)
#include <qobject.h>

class QWidget;
class KStatusBar;
class KConfig;
class KSocket;
class KServerSocket;
class KProgress;
class KProcess;

class PilotRecord;
class MessageDialog;

#include "pilotUser.h"
// #include "pi-file.h"
#include "pilotDatabase.h"
#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"

#else
#include <qobject.h>
#include <qlist.h>
#include <kurl.h>
#include <kstatusbar.h>
#include <kprogress.h>
#include <kapp.h>
#include <kprocess.h>
#include <ksock.h>
#include "pilotUser.h"
#include "pi-file.h"
#include "pilotDatabase.h"
#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"
#include "messageDialog.h"
#endif

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
#ifdef KDE2
		const QString &devicePath = QString::null);
#else
		const QString &devicePath = QString());
#endif
  ~KPilotLink();
  
  /**
   * Commands sent from KPilot requesting the daemon
   * to perform those actions.
   */
  enum Commands { Backup, Restore, HotSync, InstallFile, TestConnection };
  
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
  void changePilotPath(const char* devicePath) { initPilotSocket(devicePath); }

  /**
   * This is merely a flag for any widget that should run.  Will be set by KPilot when doing
   * a full backup, then reset after that.  Mostly obsolete since conduits are being used.
   */
  void setSlowSyncRequired(bool yesno) { fSlowSyncRequired = yesno; }

  /** 
   * Returns the whether the widget should do a full or partial backup.  Mostly obsolete now
   * that conduits are being used.
   */
  bool slowSyncRequired() { return fSlowSyncRequired; }
  
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
  
	/**
	* Returns a (new) pointer to the KPilot configuration object.
	* This is used to put all the KPilot configuration --
	* including conduits and such -- into one rc file and
	* not spread out among config files for each conduit.
	*
	* Callers should delete this object when no longer needed.
	*/
#ifdef KDE2
	static KConfig *getConfig(const QString &group=QString::null);
#else
	static KConfig *getConfig(const QString &group=QString());
#endif

private:
  PilotRecord* readRecord(KSocket*);
  void writeRecord(KSocket*, PilotRecord*);

  /**
   * Starts syncing the next (could be first) database, either by firing up a conduit, or
   * by using syncDatabase() to back it up into the BACKUP_DIR.  Uses
   * fNextDBIndex as the next DB to sync, and increments it.
   */
	int findNextDB(struct DBInfo *);
  void syncNextDB();
  void doConduitBackup();

public slots:
  void slotConduitRead(KSocket*);
  void slotConduitClosed(KSocket*);
  void slotConduitConnected(KSocket*);
 /* END CONDUIT SYNCING SUPPORT */

public:
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
  void initPilotSocket(const char* devicePath);

  int         fPilotMasterSocket;   // This is the master one created with the class
  int         fCurrentPilotSocket;  // This changes with each connect()/disconnect()
  bool        fSlowSyncRequired;
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
signals:
  void databaseSyncComplete();
  void syncingDatabase(char*);
//   void recordModified(PilotRecord*);
//   void recordDeleted(PilotRecord*);
};

#endif
