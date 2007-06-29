#ifndef RECORDCONDUIT_H
#define RECORDCONDUIT_H

/* RecordConduit.h			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema
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

class RecordConduit : public ConduitAction {
// Members
protected:
	QString fDatabaseName;
	IDMapping *fMapping;
	HHDataProxy *fHHDataProxy;
	HHDataProxy *fBackupDataProxy;
	PCDataProxy *fPCDataProxy;

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
	
	bool askConfirmation(const QString & volatilityMessage);

	void copyDatabases();

	void createBackupDatabase();

	/**
	 * This method is called after test and can be used by the implementing class 
	 *to clean things up.
	 */
	virtual void postTest() = 0;

	/**
	 * This operation is called before test and can be used to set things up for 
	 * testing purposes.
	 */
	virtual void preTest() = 0;

	virtual void syncRecords(
		const Record & hh,
		const Record & backup,
		const Record & pc) = 0;

	/**
	 * This method is called when the conduit is run in Test Mode. The implementing
	 * class can do whatever it wants to do for test purposes.
	 */
	virtual void test() = 0;
};
#endif
