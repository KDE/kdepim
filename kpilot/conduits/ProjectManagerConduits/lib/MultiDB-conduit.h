#ifndef _MultiDBCONDUIT_H
#define _MultiDBCONDUIT_H

/* MultiDB-conduit.h			KPilot
**
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown
** Copyright (C) 1998 Herwin-Jan Steehouwer
** Copyright (C) 1998 Reinhold Kainhofer
**
** This file is part of the MultiDB conduit, a conduit for KPilot that
** synchronises the Pilot's MultiDB application with the outside world,
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to groot@kde.org
*/

#include "klistview.h"
#include "options.h"

#include <plugin.h>
#include <todo.h>
#include <pilotSerialDatabase.h>
#include <pilotLocalDatabase.h>
#include "MultiDB-factory.h"
#include "calendar.h"

using namespace KCal;


class MultiDBConduit : public ConduitAction {
	Q_OBJECT
public:
	MultiDBConduit(KPilotDeviceLink *, const char *n=0L, const QStringList &l=QStringList(), SyncTypeList_t *tps=NULL);
	virtual ~MultiDBConduit() {} ;

	virtual unsigned long flags() const { return 0; }
	virtual bool exec();
	virtual bool isCorrectDBTypeCreator(DBInfo dbinfo) { return dbinfo.type==dbtype() && dbinfo.creator==dbcreator(); } ;

	/* pure virtual functions, need to be overloaded in child classes */
	virtual const QString conduitName()=0;
	virtual const QString conduitSettingsGroup()=0;
	virtual const QString settingsFileList()=0;
	virtual const unsigned long dbtype()=0;
	virtual const unsigned long dbcreator()=0;
	virtual bool GetSyncType(DBInfo dbinfo, DBSyncInfo*syncinfo);
	virtual const QString getSyncTypeEntry()=0;
	virtual const QString getSyncFileEntry()=0;
	virtual void updateLocalEntry(PilotRecord *rec, bool force=false)=0;
	virtual bool preSyncAction(DBSyncInfo*dbinfo);

protected slots:
	virtual void syncNextDB();
	virtual void finishedDB();
	virtual void syncNextRecord();
	virtual void cleanup();
	virtual void cleanupDB();

protected:
	DBSyncInfo syncinfo;
	QStringList dbases;
	int dbnr;
	bool fFullSync, conflictResolution, archive;
	PilotSerialDatabase *fCurrentDatabase;
	PilotLocalDatabase *fBackupDatabase;
	SyncTypeList_t *synctypes;
};


#endif
