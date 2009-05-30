#ifndef TESTDATAPROXY_H
#define TESTDATAPROXY_H
/* testdataproxy.h			KPilot
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

#include "dataproxy.h"

class TestRecord;

class KPILOT_EXPORT TestDataProxy : public DataProxy {
private:
	int fCreateCount;
	int fUpdateCount;
	int fDeleteCount;
	int fId;
	QMap<QString, TestRecord*> fUpdatedRecord;
	QMap<QString, TestRecord*> fDeletedRecord;
	
public:
	TestDataProxy();
	
	/**
	 * Creates a dataproxy with @p count records in it. The ids are prefixed with
	 * @p idPref.
	 */
	TestDataProxy( int count, const QString &idPref );

	/** Added for test purposes **/
	
	int createCount() { return fCreateCount; }
	
	int updateCount() { return fUpdateCount; }
	
	int deleteCount() { return fDeleteCount; }
	
	QMap<QString, bool> created() { return fCreated; }

	QMap<QString, Record*>* records() { return &fRecords; }

	QMultiMap<QString, Record*>* recordsByDescription() { return &fRecordsByDescription; }
	
	QMap<QString, TestRecord*>* updatedRecords() { return &fUpdatedRecord; }
	
	QMap<QString, TestRecord*>* deletedRecords() { return &fDeletedRecord; }
	
	virtual void printRecords();

	/**
	 * Generates a unique id for a new record.
	 */
	 virtual QString generateUniqueId() { return QString::number( --fId );  }

	/*
	 * Implement virtual methods to be able to instantiate this. The testclass 
	 * will only test the non-virtual methods of DataProxy end Record.
	 */
	virtual bool isOpen() const;
	
	virtual void loadAllRecords();
	
	virtual bool commitCreate( Record *rec );
	
	virtual bool commitDelete( Record *rec );
	
	virtual bool commitUpdate( Record *rec );
	
	virtual bool createDataStore() { return true; }
	
	virtual void setCategory( Record* r, const QString& c );

	virtual void addCategory( Record* r, const QString& c );
};
#endif
