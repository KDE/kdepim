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
class Record;

/**
 * This is the base class for all record based conduits. Each of them should 
 * implement this class. All references in the form of (see x.x) refer to a 
 * section in the document "Use Case - Conduit Syncing.odt" which can be found
 * in the design directory.
 */
class RecordConduit : public ConduitAction {
// Members
protected:
	QString fDatabaseName;
	IDMapping *fMapping;
	DataProxy *fHHDataProxy;
	DataProxy *fBackupDataProxy;
	DataProxy *fPCDataProxy;

// Methods
public:
	RecordConduit( KPilotLink *o, const QStringList &a = QStringList()
		, const QString &databaseName = QString()
		, const QString &conduitName = QString() );
	
	virtual ~RecordConduit();

protected:
	virtual bool exec();
	
	virtual void loadSettings() = 0;
	
	/**
	 * Initialize the data proxies data are needed during sync. The following 
	 * members should be initialized after the call:
	 * - fLocalDatase
	 * - fDatastore
	 * - fBackupdb
	 */
	virtual void initDataProxies() = 0;

	/**
	 * This method is called when the conduit is run in Test Mode. The 
	 * implementing class can do whatever it wants to do for test purposes.
	 */
	virtual void test() = 0;

private:
	/**
	 * Executes the HotSync flow (see 4.1)
	 */
	void hotSync();
	
	bool checkVolatility();
	
	/**
	 * Synchronizes the three records. If one of the parameters is 0L we asume 
	 * that either the record does not exist (and needs to be created), or it is
	 * deleted and should be deleted on the other side.
	 */
	void syncRecords( Record *pcRecord, Record *backupRecord, Record *hhRecord );
	
	/**
	 * Changes the fields in @p to so that they are in sync with the fields from
	 * @p from. Returns false if one of the fields of @p from is not in @p to or
	 * if one of the values is not accepted by @p to.
	 */
	bool syncFields( Record *from, Record *to );
	
	void solveConflict( Record *pcRecord, Record *hhRecord );

	virtual bool createBackupDatabase() = 0;
};
#endif
