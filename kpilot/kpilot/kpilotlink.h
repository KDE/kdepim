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

#ifndef _KPILOT_KPILOTLINK_H
#define _KPILOT_KPILOTLINK_H

#ifndef QOBJECT_H
#include <qobject.h>
#endif

class QWidget;
class KStatusBar;
class KSocket;
class KServerSocket;
class KProgress;
class KProcess;

class PilotRecord;
class MessageDialog;

class PilotDatabase;
class PilotSerialDatabase;
class PilotLocalDatabase;

// As long as we have a PilotUser data member
// we'll have to keep including this.
//
// TODO: Change to pointer, lose the include.
//
//
#ifndef _KPILOT_PILOTUSER_H
#include "pilotUser.h"
#endif

#ifndef _KPILOT_STATUSMESSAGES_H
#include "statusMessages.h"
#endif

/*
** The KPilotLink class was originally a kind of C++ wrapper
** for the pilot-link library. It grew and grew and mutated
** until it was finally cleaned up again in 2001. In the meantime
** it had become something that wrapped a lot more than just
** pilot-link. This class currently does:
** 
** * Client and server handling of kpilotlink protocol connections
** * Pilot-link handling
*/


/*
** The struct db is a description class for Pilot databases
** by Kenneth Albanowski. It's not really clear why it's *here*.
** The macro pi_mktag is meant to be given four char (8-bit)
** quantities, which are arranged into an unsigned long; for example
** pi_mktag('l','n','c','h'). This is called the creator tag
** of a database, and db.creator can be compared with such a
** tag. The tag lnch is used by the Pilot's launcher app. Some
** parts of KPilot require such a tag.
*/
struct db 
{
	char name[256];
	int flags;
	unsigned long creator;
	unsigned long type;
	int maxblock;
};

#define pi_mktag(c1,c2,c3,c4) (((c1)<<24)|((c2)<<16)|((c3)<<8)|(c4))




/*
** KPilotLink is a QObject so that it can use the Qt signals / slots
** architecture (although it currently does so only half-heartedly).
*/

class KPilotLink : public QObject
{
	Q_OBJECT

friend class PilotSerialDatabase;

public:

/*
** Constructors and Destructors
*/

	/**
	* The docs here were:
	*     Used for opening local databases only.
	* but I don't really understand that. It is meant,
	* at any rate, for applications other than the
	* pilot-link server, ie. everything but the Pilot daemon.
	*/
	KPilotLink();

	/**
	* Creates a pilot link that can sync to the pilot.
	* @p devicePath is a path to the serial port (or USB
	* port). If @p devicePath is NULL, then /dev/pilot
	* is used instead.
	*
	* TODO: ditch the statusBar. Use signals instead.
	*/
	KPilotLink(QWidget* owner, 
		KStatusBar* statusBar = 0L, 
		const QString &devicePath = QString::null);

	virtual ~KPilotLink();


/*
** Connection and Command methods
*/


	/**
	* This belongs on both client and server end: these are
	* commands sent from KPilot telling the daemon
	* to perform those actions.
	*/
	enum Commands { Backup, 
		Restore, 
		HotSync, 
		FastSync,
		InstallFile, 
		TestConnection 
	};

	/**
	* Static method to get the current link... the KPilotLink
	* is supposed to follow the singleton pattern, but does
	* so rather poorly :(
	*/
	static KPilotLink* getPilotLink() 
		{ return fKPilotLink; }

	/**
	* Begin a HotSync session with a Palm Pilot; this does
	* the actual serial communications; it can also block the
	* UI for a while.
	*/
	void startHotSync();

	/**
	* End a HotSync session with a Palm Pilot.
	*/
	void endHotSync();

	/**
	* Backups all databases to BACKUP_DIR/username.
	*/
	void doFullBackup();  

	/**
	* Restores all databases to the Pilot from BACKUP_DIR/username.
	*/
	bool doFullRestore(); 

	/**
	* Installs all files found in installPath on the Pilot.
	*/
	void installFiles(const QString &installPath);

	/**
	* True if HotSync has been started but not finished yet
	* (ie. the physical Pilot is waiting for sync commands)
	*/
	bool getConnected() const 
		{ return fConnected; }

	/**
	* Returns the user information as set in the KPilot settings dialog.
	* The user information can also be set by the Pilot, and at the
	* end of a HotSync the two user informations are synced as well.
	*/
	KPilotUser& getPilotUser() 
		{ return fPilotUser; }

	/**
	* Changes the path (port) that the link should work with.
	*/
	void changePilotPath(const QString& devicePath) 
		{ initPilotSocket(devicePath); }

/*
** Sync Mode methods
*/

	/*
	** KPilotLink currently uses two boolean variables to indicate
	** what kind of a sync is running. This is utterly moot since
	** the internal conduits no longer sync any DBs themselves.
	** We want to move to a situation where the kind of HotSync is:
	** 
	** * FullBackup - copy all DBs to disk
	** * HotSync    - copy all DBs to disk and run conduits
	** * FastSync   - copy DBs with conduits to disk and run conduits
	*/

	/**
	* This is merely a flag for any widget that should run.  
	* Will be set by KPilot when doing a full backup, then 
	* reset after that.  Mostly obsolete since conduits are 
	* being used.
	*/
	void setSlowSyncRequired(bool yesno) 
		{ fSlowSyncRequired = yesno; }

	/** 
	* Returns the whether the widget should do a full or partial backup.  
	* Mostly obsolete now that conduits are being used.
	*/
	bool slowSyncRequired() const 
		{ return fSlowSyncRequired; }

	/**
	* Here we have another boolean variable with set and get
	* methods, meant to indicate a FastSync like Heiko Purnhagen
	* wanted. I have no idea, really, whether it's implemented.
	*/
	void setFastSyncRequired(bool yesno) 
		{ fFastSyncRequired = yesno; } 
	bool fastSyncRequired() const 
		{ return fFastSyncRequired; } 


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
	QString registeredConduit(const QString &dbName) const;

	/**
	* Uses BACKUP_DIR/username/databaseName as local source, and 
	* compares records versus those on the Pilot in databaseName.  
	* When finished, BACKUP_DIR/username/databaseName.pdb (or prc) 
	* will contain the same data as the pilot.
	*/
	bool syncDatabase(DBInfo* database);


/* START Tickle Support */

public:
	/**
	* These are methods and slots intended to keep the Pilot awake while 
	* something lengthy -- like starting an application -- is going on. 
	*
	* Passing a timeout of 0 to setTickleTimeout() indicates no timeout.
	* Use this with care, as it will keep the Pilot awake forever
	* without finishing the sync unless stopTickle() is called.
	*/
	void setTickleTimeout(unsigned seconds = 30) { fTickleTimeout=2*seconds; } ;

public slots:
	/**
	* Start and stop tickle are used to tickle the Pilot every-so-often
	* while you're doing other work. Note that this uses QTimer, so you
	* will need an event loop, ie. during for(unsigned i=1; i; i++) foo();
	* this will not help. Ideal for keeping the Pilot awake during user
	* interaction, though.
	*/
	void startTickle();
	void stopTickle();
	void tickle();

signals:
	void timeout();

private:
	void initTickle();

	// The tickletimeout and ticklecount here count in half-seconds,
	// which is also why it says "2*seconds" above.
	//
	//
	QTimer *fTimer;
	unsigned fTickleTimeout, fTickleCount;

/* END   Tickle Support */

private:
	/**
	* For initializing the link: read the configuration
	* file and save vital information from there.
	*/
	void readConfig();

	/**
	* These two private functions read or write an entire
	* Pilot record -- which may be of varying length --
	* from the given socket following the kpilotlink
	* protocol. A new record is returned by readRecord(),
	* the caller should dispose of it when done.
	*/
	PilotRecord* readRecord(KSocket*);
	void writeRecord(KSocket*, PilotRecord*);

	/**
	* Write a single-word response message to the command
	* connection. This is typically used to respond to 
	* requests from conduits.
	*
	* @return the number of bytes written, should be sizeof(int)
	*/
	int writeResponse(KSocket *, CStatusMessages::LinkMessages m);

	/**
	* Starts syncing the next (could be first) database, either 
	* by firing up a conduit, or by using syncDatabase() to back 
	* it up into the BACKUP_DIR.  Uses fNextDBIndex as the next DB 
	* to sync, and increments it.
	*/
	void syncNextDB();

	/**
	* Asks the Pilot for the next database to sync, storing
	* the DB info in @p p. 
	*
	* @return 0 if there is no more database to sync
	*/
	int findNextDB(struct DBInfo *p);

	/**
	* Does a syncNextDB but without using syncDatabase for
	* the databases with no conduit -- this is for FastSync.
	*/
	void doConduitBackup();

	/**
	* This function is called by slotConduitClosed and
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

signals:
	/**
	* Whenever a conduit adds a Sync log entry (actually,
	* KPilotLink itself adds some log entries itself),
	* this signal is emitted.
	*/
	void logEntry(const char *);

protected slots:
	/**
	* This is for handling the connection, communication,
	* and disconnection of conduits via the socket that
	* runs the kpilotlink protocol.
	*/
	void slotConduitRead(KSocket*);
	void slotConduitClosed(KSocket*);
	void slotConduitConnected(KSocket*);
	void slotConduitDone(KProcess *);


private:
	/**
	* This is en enum listing the states that a running conduit
	* may be in. This is used to detect conduits that crash or
	* otherwise misbehave.
	*/
	enum { None = 0 , Running, Connected, Done } fConduitRunStatus ;

	/**
	* For sanity checking, we try to restrict ourselves to one 
	* conduit and one socket at a time.
	*/
	KSocket *fCurrentConduitSocket;

public:
	/**
	* Write a log entry to the pilot. Note that the library
	* function takes a char *, not const char * (which is
	* highly dubious). Causes signal logEntry(const char *)
	* to be emitted.
	*/
	void addSyncLogEntry(const char* entry);


/*
** Misguided GUI methods
*/
	/**
	* These are some seriously misguided methods for showing th
	* status bar. Really they belong in KPilot itself or the daemon.
	*/
	void createNewProgressBar(const QString &title, 
		const QString &text, 
		int min, int max, int value);
	void updateProgressBar(int value) const;
	void destroyProgressBar();

	/**
	* Displays a dialog showing error message.
	*/
	void showMessage(const QString &message) const;

	/** 
	* Compares the user name in getPilotUser() to the one saved by KPilot
	* If they don't match, check with the user to see if we should
	* make them match.
	* When this routine returns all the user info in getPilotUser will be 
	* as correct as the user wants it.
	*/
	void checkPilotUser();

protected:
	void setConnected(bool connected) { fConnected = connected; }

	int  getPilotMasterSocket() const { return fPilotMasterSocket; }
	int  getCurrentPilotSocket() const { return fCurrentPilotSocket; }


	QWidget* getOwningWidget() { return fOwningWidget; }

	QString    fPilotPath;  // defaults to /dev/pilot
	bool       fConnected;

private:
	static KPilotLink* fKPilotLink; // Static variable to ourself.
	// static const QString BACKUP_DIR; // The directory backedup databases go..

	/**
	* This should be called to finish up a HotSync. It sets
	* some flags on the Pilot and stores the last HotSync time.
	*/
	void syncFlags();

	/**
	* Creates a local database in BACKUP_DIR for the given database.
	*/
	bool createLocalDatabase(DBInfo* info);

	/**
	* findDisposition is used to see if the database 
	* d is named in the (comma separated) list s.
	* This is currently based only on the creator of
	* the database.
	*/
	int findDisposition(const QString &s,const struct DBInfo *d);

	/**
	* Compares two database structs in a strange order.
	* @return -1,0,1 depending.
	*/
	int compare(struct db* d1, struct db* d2);

	/**
	* Setup the socket that waits for incoming connections from
	* conduits.
	*/
	void initConduitSocket();

	/**
	* Setup socket that connects to the physical device.
	* @p inetconnection is currently ignored.
	*/
	void initPilotSocket(const QString& devicePath, 
		bool inetconnection=false);

private:
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
	PilotSerialDatabase* fCurrentDB;  // Currently Open Database (for conduit)
	DBInfo      fCurrentDBInfo;          // Currently open Database information
	int         fNextDBIndex;         // The next DB to sync
	KProcess*    fConduitProcess;
	MessageDialog* fMessageDialog;    // The dialog showing current status

	Status fStatus;
	QString fDatabaseDir;

signals:
	void databaseSyncComplete();
	void syncingDatabase(char*);
};

#else
#ifdef DEBUG
#warning "File doubly included"
#endif
#endif

// $Log$
// Revision 1.26  2001/05/24 10:36:56  adridg
// Tickle support
//
// Revision 1.25  2001/04/26 21:59:00  adridg
// CVS_SILENT B0rkage with previous commit
//
// Revision 1.24  2001/04/26 19:25:24  adridg
// Real change in addSyncLogEntry; muchos reformatting
//
// Revision 1.23  2001/04/16 13:48:35  adridg
// --enable-final cleanup and #warning reduction
//
// Revision 1.22  2001/03/27 23:54:43  stern
// Broke baseConduit functionality out into PilotConduitDatabase and added support for local mode in BaseConduit
//
// Revision 1.21  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.20  2001/03/08 16:18:40  adridg
// Cruft removal
//
// Revision 1.19  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
