/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __KPILOT_LINK
#define __KPILOT_LINK

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
  KPilotLink(QWidget* owner, KStatusBar* statusBar = 0L, char* devicePath = 0L);
  ~KPilotLink();
  
  /**
   * Commands sent from KPilot requesting the daemon
   * to perform those actions.
   */
  enum Commands { Backup, Restore, HotSync, InstallFile };
  
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
  void installFiles(QString installPath);
  
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
  QString registeredConduit(QString dbName);

  /**
   * Uses BACKUP_DIR/username as local source, and compares records versus those on the pilot
   * in databaseName.  When finished, BACKUP_DIR/username/databaseName.pdb (or prc) 
   * will contain the same data as the pilot.
   */
  bool syncDatabase(DBInfo* database);
  
private:
  PilotRecord* readRecord(KSocket*);
  void writeRecord(KSocket*, PilotRecord*);

  /**
   * Starts syncing the next (could be first) database, either by firing up a conduit, or
   * by using syncDatabase() to back it up into the BACKUP_DIR.  Uses
   * fNextDBIndex as the next DB to sync, and increments it.
   */
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
  PilotDatabase* openDatabase(char* database) { return new PilotSerialDatabase(this, database); }

  /**
   * Opens database "database" locally and returns the handle to it:
   */
  PilotDatabase* openLocalDatabase(char* database) { return new PilotLocalDatabase(kapp->localkdedir() +  BACKUP_DIR + getPilotUser().getUserName() + "/", database); }

  /**
   * Closes database 'database'.
   */
  void closeDatabase(PilotDatabase* database) { delete database; }
  
  /**
   * Write a log entry to the pilot
   */
  void addSyncLogEntry(char* entry) { dlp_AddSyncLogEntry(getCurrentPilotSocket(), entry); }
  
  void createNewProgressBar(QString title, QString text, int min, int max, int value);
  void updateProgressBar(int value) const;
  void destroyProgressBar();
  
  /**
   * Displays a dialog showing error message.
   */
  void showMessage(QString message) const;
  
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
