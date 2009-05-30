#ifndef TESTRECORDCONDUIT_H
#define TESTRECORDCONDUIT_H
/* testrecordconduit.h			KPilot
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

#include "recordconduit.h"
#include "testdataproxy.h"
#include "testhhdataproxy.h"

class KPILOT_EXPORT TestRecordConduit : public RecordConduit {
public:
	TestRecordConduit( const QVariantList &args, bool createRecords = false );
	
	virtual void loadSettings();
	
	virtual bool initDataProxies();
	
	virtual void test();
	
	virtual bool createBackupDatabase() { return true; }
	
	virtual Record* createPCRecord( const HHRecord* record );
	
	virtual HHRecord* createHHRecord( const Record* record );
	
	virtual bool equal( const Record *rec, const HHRecord *hhRec ) const;
	
	virtual HHRecord* newHHRecord( Record *pcRecord );
	
	virtual Record* newPCRecord( HHRecord *hhRecord );
	
	virtual void copy( const Record *from, HHRecord *to );

	virtual void copy( const HHRecord *from, Record *to  );
	
	virtual void setCategory( Record* r, const QString& c ) { Q_UNUSED( r ); Q_UNUSED( c ); }

	virtual void addCategory( Record* r, const QString& c ) { Q_UNUSED( r ); Q_UNUSED( c ); }
	
	/** Methods below are added for testpurposes **/
	
	void solveConflictTest( Record *pcRecord, HHRecord *hhRecord );
	
	void hotSyncTest();
	
	void firstSyncTest();
	
	void copyHHToPCTest();
	
	void copyPCToHHTest();
	
	TestDataProxy *pcDataProxy()
	{
		return static_cast<TestDataProxy*>( fPCDataProxy );
	}
	
	TestHHDataProxy *hhDataProxy()
	{
		return static_cast<TestHHDataProxy*>( fHHDataProxy );
	}
	
	TestHHDataProxy *backupDataProxy()
	{
		return static_cast<TestHHDataProxy*>( fBackupDataProxy );
	}
	
	IDMapping& mapping() { return fMapping; }

	void addHHRecord( HHRecord *rec );
	
	void addBackupRecord( HHRecord *rec );
	
	void addPCRecord( Record *rec );

protected:
	virtual void _copy( const Record *from, HHRecord *to );

	virtual void _copy( const HHRecord *from, Record *to  );

private:
	bool fCreateRecords;
};

#endif
