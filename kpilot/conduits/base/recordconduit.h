#ifndef RECORDCONDUIT_H
#define RECORDCONDUIT_H
/* RecordConduit.h			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>
** Copyright (C) 2007 by Jason "vanRijn" Kasper <vr@movingparts.net>
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

#include "idmapping.h"
#include "plugin.h"

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
	IDMapping fMapping;
	HHDataProxy *fHHDataProxy;
	HHDataProxy *fBackupDataProxy;
	DataProxy *fPCDataProxy;
	QSet<QString> fSyncedPcRecords;

// Methods
public:
	explicit RecordConduit( KPilotLink *o, const QVariantList &a = QVariantList()
		, const QString &conduitName = QString( "Conduit name not set." )
		, const QString &databaseName = QString() );

	virtual ~RecordConduit();

protected:
	virtual bool exec();

	virtual void loadSettings() = 0;

	/**
	 * Initialize the data proxies that are needed during sync and makes sure that
	 * all records are loaded for each data proxy. The following
	 * members should be initialized after the call:
	 * - fLocalDatabase
	 * - fDatastore
	 * - fBackupdb
	 *
	 * Returns false if one of the dataproxies could not be initialized.
	 */
	virtual bool initDataProxies() = 0;

	/**
	 * Compares @p pcRecord with @p hhRec and returns true if they are considered
	 * equal.
	 */
	virtual bool equal( const Record *pcRec, const HHRecord *hhRec ) const  = 0;

	/**
	 * Creates a new Record object with the same data as @p hhRec.
	 */
	virtual Record* createPCRecord( const HHRecord *hhRec ) = 0;

	/**
	 * Creates a new HHRecord object with the same data as @p pcRec.
	 */
	virtual HHRecord* createHHRecord( const Record *pcRec ) = 0;

	/**
	 * Copies the categories of @p from to @p to. Delegates the rest of the
	 * copying to the implementing classes. @see
	 * _copy( const Record *from, HHRecord *to ).
	 *
	 * RecordConduit::equal( pcRecord, hhRecord ) must return true.
	 */
	virtual void copy( const Record *from, HHRecord *to );

	/**
	 * Copies the category of @p from to @p to. Delegates the rest of the
	 * copying to the implementing classes. @see
	 * _copy( const HHRecord *from, Record *to ).
	 *
	 * RecordConduit::equal( pcRecord, hhRecord ) must return true.
	 */
	virtual void copy( const HHRecord *from, Record *to  );

		/**
	 * Copies the field values of @p from to @p to. The method should only touch
	 * data that can be synced between the two records and leave the rest of the
	 * records data unchanged. After calling this method
	 *
	 * RecordConduit::equal( pcRecord, hhRecord ) must return true.
	 */
	virtual void _copy( const Record *from, HHRecord *to ) = 0;

	/**
	 * Copies the field values of @p from to @p to. The method should only touch
	 * data that can be synced between the two records and leave the rest of the
	 * records data unchanged. After calling this method
	 *
	 * RecordConduit::equal( pcRecord, hhRecord ) must return true.
	 */
	virtual void _copy( const HHRecord *from, Record *to  ) = 0;

	/**
	 * Give implementing conduits the change to clean up after a successful sync.
	 */
	virtual void syncFinished() {};

	/**
	 * This method is called when the conduit is run in Test Mode. The
	 * implementing class can do whatever it wants to do for test purposes.
	 */
	virtual void test() = 0;

	void updateBackupDatabase();

	/**
	 * Executes the HotSync or the FullSync flow (see 4.1 and 5.2). What actualy
	 * is executed depends on syncMode().mode().
	 */
	void hotOrFullSync();

	/**
	 * Executes the FirstSync flow (see 5.1)
	 */
	void firstSync();

	/**
	 * Executes the copyHHToPC flow (see 5.3)
	 */
	void copyHHToPC();

	/**
	 * Executes the copyPCToHH flow (see 5.4)
	 */
	void copyPCToHH();

	/**
	 * Checks the number of changes (Creates, updates, deletes) as wel as the
	 * number of changes. When one of these exceeds the configured values it will
	 * warn the user and return false if the user doesn't want to commit the
	 * changes to the data stores.
	 */
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

private: //Functions
	/**
	 * Copies the category from the pc record to the handheld record. It also does
	 * some checks because handheld records have always only one category while pc
	 * records may have more than one category.
	 */
	void copyCategory( const Record *from, HHRecord *to );

	/**
	 * Copies the category from the handheld record to the pc record. It also does
	 * some checks because handheld records have always only one category while pc
	 * records may have more than one category.
	 */
	void copyCategory( const HHRecord *from, Record *to  );
};
#endif
