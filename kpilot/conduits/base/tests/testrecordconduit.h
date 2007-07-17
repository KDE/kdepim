#ifndef TESTRECORDCONDUIT_H
#define TESTRECORDCONDUIT_H
/* keyringconduit.h			KPilot
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

#include "recordconduit.h"

class KPILOT_EXPORT TestRecordConduit : public RecordConduit {
public:
	TestRecordConduit( const QStringList &args, bool createRecords = false );
	
	~TestRecordConduit();
	
	virtual void loadSettings();
	
	virtual void initDataProxies();
	
	virtual void test();
	
	virtual bool createBackupDatabase() { return true; }
	
	/** Methods below are added for testpurposes **/
	
	bool syncFieldsTest( Record *from, Record *to );
	
	void solveConflictTest( Record *pcRecord, Record *hhRecord );
	
	void hotSyncTest();
	
	DataProxy *pcDataProxy() { return fPCDataProxy; }
	
	DataProxy *hhDataProxy() { return fHHDataProxy; }
	
	DataProxy *backupDataProxy() { return fBackupDataProxy; }
	
	IDMapping *mapping() { return fMapping; }

private:
	bool fCreateRecords;
};

#endif
