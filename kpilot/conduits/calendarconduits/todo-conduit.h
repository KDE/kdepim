#ifndef _KPILOT_TODO_CONDUIT_H
#define _KPILOT_TODO_CONDUIT_H
/* todo-conduit.h                       KPilot
**
** Copyright (C) 2002-2003 Reinhold Kainhofer
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown
** Copyright (C) 1998 Herwin-Jan Steehouwer
**
** This file is part of the todo conduit, a conduit for KPilot that
** synchronises the Pilot's todo application with the outside world,
** which currently means KOrganizer.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <libkcal/todo.h>
#include <pilotTodoEntry.h>
#include "vcal-conduitbase.h"

class PilotRecord;
class PilotSerialDatabase;
class PilotLocalDatabase;
class PilotTodoEntry;

class TodoConduit : public VCalConduitBase
{
Q_OBJECT
public:
	TodoConduit( KPilotDeviceLink *, const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~TodoConduit();
	
	class TodoData : public VCalConduitBase::VCalDataBase
	{
	public:
		TodoData( RecordConduit *conduit, VCalConduitSettings *cfg );
		virtual ~TodoData();
		virtual bool initData();
		virtual QString description() const { return i18n("todo list"); }
	}; 
	
	
protected:
	virtual QString dbName() const { return CSL1("ToDoDB"); };

	virtual PCData*initializePCData();
	unsigned char *doPackAppInfo( int *appLen );
	bool doUnpackAppInfo( unsigned char *buffer, int appLen );
	virtual const QString getTitle(PilotAppCategory*de);
	virtual VCalConduitSettings *config();
	virtual void readConfig();
	virtual PilotAppCategory *createPalmEntry( PilotRecord *rec )  {
		FUNCTIONSETUP; 
		if ( rec ) return new PilotTodoEntry( fTodoAppInfo, rec ); 
		else return new PilotTodoEntry( fTodoAppInfo );
	};
	virtual void doPostSync();
	virtual QString category( int n ) const;

	enum {
		eqFlagsDesc=0x04,
		eqFlagsDue=0x08,
		eqFlagsCompleted=0x10,
		eqFlagsPriority=0x20,
		eqFlagsSecrecy=0x40,
	};
	virtual bool _equal( const PilotAppCategory *palmEntry, const PCEntry *pcEntry, 
				int flags = eqFlagsAlmostAll ) const;
	virtual bool _copy( PilotAppCategory *toPalmEntry, const PCEntry *fromPCEntry );
	virtual bool _copy( PCEntry *toPCEntry, const PilotAppCategory *fromPalmEntry );
// 	virtual bool smartMergeEntry( RecordConduit::PCEntry *pcEntry, PilotAppCategory *backupEntry,
// 		PilotAppCategory *palmEntry );
	
	struct ToDoAppInfo fTodoAppInfo;
	bool categoriesSynced;
} ;

#endif
