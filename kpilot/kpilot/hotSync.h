#ifndef _KPILOT_HOTSYNC_H
#define _KPILOT_HOTSYNC_H
/* hotSync.h                            KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines a specialization of KPilotDeviceLink
** that can actually handle some HotSync tasks, like backup
** and restore. It does NOT do conduit stuff.
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
** Bug reports and questions can be sent to groot@kde.org
*/


class QTimer;

#include <qstring.h>

#include "kpilotlink.h"

class TestLink : public SyncAction
{
Q_OBJECT

public:
	TestLink(KPilotDeviceLink *);

public slots:
	virtual void exec();
} ;

class BackupAction : public SyncAction
{
Q_OBJECT

public:
	BackupAction(KPilotDeviceLink *);

	enum Status { Init,
		Error,
		FullBackup,
		BackupIncomplete,
		BackupEnded,
		BackupComplete 
		} ;
	virtual QString statusString() const;

public slots:
	virtual void exec();

private:
	/**
	* All manner of support functions for full backup.
	*/
	void endBackup();
	bool createLocalDatabase(DBInfo *);

private slots:
	void backupOneDB();

private:
	QTimer *fTimer;
	int fDBIndex;
	QString fDatabaseDir;
} ;

class RestoreAction : public SyncAction
{
Q_OBJECT
public:
	RestoreAction(KPilotDeviceLink *);

	typedef enum { InstallingFiles, GettingFileInfo,Done } Status;
	virtual QString statusString() const;

public slots:
	virtual void exec();

protected slots:
	void getNextFileInfo();
	void installNextFile();

private:
	// Use a private-d pointer for once (well, in KPilot
	// parlance it'd be fd, which is confusing, so it's
	// become a private fP) since we need QList or QPtrList.
	//
	//
	class RestoreActionPrivate;
	RestoreActionPrivate *fP;
} ;

class FileInstallAction : public SyncAction
{
Q_OBJECT
public:
	FileInstallAction(KPilotDeviceLink *,const QStringList &);
	virtual ~FileInstallAction();

	virtual QString statusString() const;

public slots:
	virtual void exec();

protected slots:
	void installNextFile();

private:
	const QStringList &fList;
	int fDBIndex;
	QTimer *fTimer;
} ;

// 
// /* CONDUIT SYNCING SUPPORT */
// 
// 	/**
// 	* Begins HotSyncing modified data.  Emits databaseSyncComplete() when
// 	* finished.
// 	*/
// 	void quickHotSync();
// 
// 	/**
// 	* Checks to see if there is a conduit registered for database dbName.
// 	* returns the name if yes, NULL if no.
// 	*/
// 	QString registeredConduit(const QString &dbName) const;
// 
// 	/**
// 	* Uses BACKUP_DIR/username/databaseName as local source, and 
// 	* compares records versus those on the Pilot in databaseName.  
// 	* When finished, BACKUP_DIR/username/databaseName.pdb (or prc) 
// 	* will contain the same data as the pilot.
// 	*/
// 	bool syncDatabase(DBInfo* database);
// 
// 
// /* START Tickle Support */
// 
// public:
// 	/**
// 	* These are methods and slots intended to keep the Pilot awake while 
// 	* something lengthy -- like starting an application -- is going on. 
// 	*
// 	* Passing a timeout of 0 to setTickleTimeout() indicates no timeout.
// 	* Use this with care, as it will keep the Pilot awake forever
// 	* without finishing the sync unless stopTickle() is called.
// 	*/
// 	void setTickleTimeout(unsigned seconds = 30) { fTickleTimeout=2*seconds; } ;
// 
// public slots:
// 	/**
// 	* Start and stop tickle are used to tickle the Pilot every-so-often
// 	* while you're doing other work. Note that this uses QTimer, so you
// 	* will need an event loop, ie. during for(unsigned i=1; i; i++) foo();
// 	* this will not help. Ideal for keeping the Pilot awake during user
// 	* interaction, though.
// 	*/
// 	void startTickle();
// 	void stopTickle();
// 	void tickle();
// 
// signals:
// 	void timeout();
// 
// private:
// 	void initTickle();
// 
// 	// The tickletimeout and ticklecount here count in half-seconds,
// 	// which is also why it says "2*seconds" above.
// 	//
// 	//
// 	QTimer *fTimer;
// 	unsigned fTickleTimeout, fTickleCount;
// 
// /* END   Tickle Support */
// 
// private:
// 
// 	/**
// 	* These two private functions read or write an entire
// 	* Pilot record -- which may be of varying length --
// 	* from the given socket following the kpilotlink
// 	* protocol. A new record is returned by readRecord(),
// 	* the caller should dispose of it when done.
// 	*/
// 	PilotRecord* readRecord(KSocket*);
// 	void writeRecord(KSocket*, PilotRecord*);
// 
// 	/**
// 	* Write a single-word response message to the command
// 	* connection. This is typically used to respond to 
// 	* requests from conduits.
// 	*
// 	* @return the number of bytes written, should be sizeof(int)
// 	*/
// 	int writeResponse(KSocket *, CStatusMessages::LinkMessages m);
// 
// 	/**
// 	* Starts syncing the next (could be first) database, either 
// 	* by firing up a conduit, or by using syncDatabase() to back 
// 	* it up into the BACKUP_DIR.  Uses fNextDBIndex as the next DB 
// 	* to sync, and increments it.
// 	*/
// 	void syncNextDB();
// 
// 	/**
// 	* Asks the Pilot for the next database to sync, storing
// 	* the DB info in @p p. 
// 	*
// 	* @return 0 if there is no more database to sync
// 	*/
// 	int findNextDB(struct DBInfo *p);
// 
// 	/**
// 	* Does a syncNextDB but without using syncDatabase for
// 	* the databases with no conduit -- this is for FastSync.
// 	*/
// 	void doConduitBackup();
// 
// 	/**
// 	* This function is called by slotConduitClosed and
// 	* some other places to resume processing of databases
// 	* for a backup or sync.
// 	*/
// 	void resumeDB();
// 
// 	/**
// 	* When the sync is done (or the backup, or whatever ...)
// 	* call this function to cleanup and emit done signals.
// 	*/
// 	void finishDatabaseSync();
// 
// 
// protected slots:
// 	/**
// 	* This is for handling the connection, communication,
// 	* and disconnection of conduits via the socket that
// 	* runs the kpilotlink protocol.
// 	*/
// 	void slotConduitRead(KSocket*);
// 	void slotConduitClosed(KSocket*);
// 	void slotConduitConnected(KSocket*);
// 	void slotConduitDone(KProcess *);
// 
// 
// private:
// 	/**
// 	* This is en enum listing the states that a running conduit
// 	* may be in. This is used to detect conduits that crash or
// 	* otherwise misbehave.
// 	*/
// 	enum { None = 0 , Running, Connected, Done } fConduitRunStatus ;
// 
// 	/**
// 	* For sanity checking, we try to restrict ourselves to one 
// 	* conduit and one socket at a time.
// 	*/
// 	KSocket *fCurrentConduitSocket;
// 
// public:
// 
// 
// /*
// ** Misguided GUI methods
// */
// 	/**
// 	* Displays a dialog showing error message.
// 	*/
// 	void showMessage(const QString &message);
// 
// 	/** 
// 	* Compares the user name in getPilotUser() to the one saved by KPilot
// 	* If they don't match, check with the user to see if we should
// 	* make them match.
// 	* When this routine returns all the user info in getPilotUser will be 
// 	* as correct as the user wants it.
// 	*/
// 	void checkPilotUser();
// 
// protected:
// 	void setConnected(bool connected) { fConnected = connected; }
// 
// 	int  getPilotMasterSocket() const { return fPilotMasterSocket; }
// 	int  getCurrentPilotSocket() const { return fCurrentPilotSocket; }
// 
// 
// 	bool       fConnected;
// 
// private:
// 	// static const QString BACKUP_DIR; // The directory backedup databases go..
// 
// 	/**
// 	* This should be called to finish up a HotSync. It sets
// 	* some flags on the Pilot and stores the last HotSync time.
// 	*/
// 	void syncFlags();
// 
// 	/**
// 	* Creates a local database in BACKUP_DIR for the given database.
// 	*/
// 	bool createLocalDatabase(DBInfo* info);
// 
// 	/**
// 	* findDisposition is used to see if the database 
// 	* d is named in the (comma separated) list s.
// 	* This is currently based only on the creator of
// 	* the database.
// 	*/
// 	int findDisposition(const QString &s,const struct DBInfo *d);
// 
// 	/**
// 	* Compares two database structs in a strange order.
// 	* @return -1,0,1 depending.
// 	*/
// 	int compare(struct db* d1, struct db* d2);
// 
// 	/**
// 	* Setup the socket that waits for incoming connections from
// 	* conduits.
// 	*/
// 	void initConduitSocket();
// 
// 	/**
// 	* Setup socket that connects to the physical device.
// 	* @p inetconnection is currently ignored.
// 	*/
// 	void initPilotSocket(const QString& devicePath, 
// 		bool inetconnection=false);
// 
// private:
// 	bool        fSlowSyncRequired;
// 	bool fFastSyncRequired;
// 	KServerSocket* fConduitSocket;    // Socket conduits connect on
// 	PilotSerialDatabase* fCurrentDB;  // Currently Open Database (for conduit)
// 	DBInfo      fCurrentDBInfo;          // Currently open Database information
// 	int         fNextDBIndex;         // The next DB to sync
// 	KProcess*    fConduitProcess;
// 
// 	QString fDatabaseDir;
// 
// signals:
// 	void databaseSyncComplete();
// 	void syncingDatabase(char*);
// };
// #endif

// $Log$
// Revision 1.1  2001/09/16 13:43:18  adridg
// Subclasses for hotSyncing
//
#endif
