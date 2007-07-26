#ifndef RECORDCONDUIT_H
#define RECORDCONDUIT_H
/* RecordConduit.h			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema
** Copyright (C) 2007 by Jason "vanRijn" Kasper
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

#include "plugin.h"

class IDMapping;
class HHDataProxy;
class PCDataProxy;
class DataProxy;
class HHRecord;
class Record;

/**
 * This is the base class for all record based conduits. Each of them should 
 * implement this class. All references in the form of (see x.x) refer to a 
 * section in the document "Use Case - Conduit Syncing.odt" which can be found
 * in the design directory.
 */
class KPILOT_EXPORT RecordConduit : public ConduitAction {
// Members
protected:
	QString fDatabaseName;
	IDMapping *fMapping;
	HHDataProxy *fHHDataProxy;
	HHDataProxy *fBackupDataProxy;
	DataProxy *fPCDataProxy;

// Methods
public:
	explicit RecordConduit( KPilotLink *o, const QStringList &a = QStringList()
		, const QString &databaseName = QString()
		, const QString &conduitName = QString() );
	
	virtual ~RecordConduit();

protected:
	virtual bool exec();
	
	virtual void loadSettings() = 0;
	
	virtual Record* createPCRecord( const HHRecord *hhRec ) = 0;
	
	virtual HHRecord* createHHRecord( const Record *pcRec ) = 0;
	
	/**
	 * Initialize the data proxies data are needed during sync. The following 
	 * members should be initialized after the call:
	 * - fLocalDatase
	 * - fDatastore
	 * - fBackupdb
	 */
	virtual void initDataProxies() = 0;

	/**
	 * Compares @p pcRecord with @p hhRec and returns true if they are equal.
	 */
	virtual bool equal( Record *pcRec, HHRecord *hhRec ) = 0;
	
	/**
	 * Synchronizes the field values of @p hhRecord to @p pcRecord. If @p fromHH
	 * is false the fields are synchronized from the pcRecord to the hhRecord.
	 * After calling this method, RecordConduit::equal( pcRecord, hhRecord ) must
	 * return true.
	 */
	virtual void syncFields( Record *pcRecord, HHRecord *hhRecord
		, bool fromHH = true ) = 0;
	
	/**
	 * This method is called when the conduit is run in Test Mode. The 
	 * implementing class can do whatever it wants to do for test purposes.
	 */
	virtual void test() = 0;
	
	/**
	 * Returns a HHRecord that is a copy of @p pcRecord.
	 */
	virtual HHRecord* newHHRecord( Record *pcRecord ) = 0;
	
	/**
	 * Returns a Record that is a copy of @p hhRecord.
	 */
	virtual Record* newPCRecord( HHRecord *hhRecord ) = 0;

	virtual bool createBackupDatabase() = 0;

	/**
	 * Executes the HotSync flow (see 4.1)
	 */
	void hotSync();
	
	/**
	 * Executes the FirstSync flow (see 5.1)
	 */
	void firstSync();
	
	/**
	 * Executes the copyHHToPC flow (see 5.3)
	 */
	void copyHHToPC();
	
	bool checkVolatility();
	
	/**
	 * Iterates over the records from the pc data proxy and tries to find a
	 * matching record for @p rec. If no matching record is found 0L is returned.
	 * The method makes use of the matchFields() method of Record.
	 */
	Record* findMatch( HHRecord *rec );
	
	/**
	 * Deletes the mapping for those records and removes them from the proxies.
	 */
	void deleteRecords( Record *pcRecord, HHRecord *hhRecord );
	
	/**
	 * Synchronizes the three records. If one of the parameters is 0L we assume 
	 * that either the record does not exist (and needs to be created), or it is
	 * deleted and should be deleted on the other side.
	 */
	void syncRecords( Record *pcRecord, HHRecord *backupRecord, HHRecord *hhRecord );
	
	void solveConflict( Record *pcRecord, HHRecord *hhRecord );
	
	/**
	 * Synchronizes the two conflicted records and lets one of the two overide.
	 */
	void syncConflictedRecords( Record *pcRecord, HHRecord *hhRecord
		, bool pcOverides );
};
#endif
