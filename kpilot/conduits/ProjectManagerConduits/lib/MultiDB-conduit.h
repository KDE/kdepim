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
	virtual void exec();
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


// $Log$
// Revision 1.1  2002/04/07 12:09:42  kainhofe
// Initial checkin of the conduit. The gui works mostly, but syncing crashes KPilot...
//
// Revision 1.1  2002/04/07 01:03:52  reinhold
// the list of possible actions is now created dynamically
//
// Revision 1.11  2002/04/05 21:17:00  reinhold
// *** empty log message ***
//
// Revision 1.10  2002/04/01 09:22:11  reinhold
// Implemented the syncNextRecord routine
//
// Revision 1.9  2002/03/28 13:47:53  reinhold
// Added the list of synctypes, aboutbox is now directly passed on to the setup dlg (instead of being a static var)
//
// Revision 1.7  2002/03/23 21:46:42  reinhold
// config  dlg works, but the last changes crash the plugin itself
//
// Revision 1.6  2002/03/23 18:21:14  reinhold
// Cleaned up the structure. Works with QTimer instead of loops.
//
// Revision 1.5  2002/03/15 20:43:17  reinhold
// Fixed the crash on loading (member function not defined)...
//
// Revision 1.4  2002/03/13 22:14:40  reinhold
// GUI should work now...
//
// Revision 1.3  2002/03/10 23:58:32  reinhold
// Made the conduit compile...
//
// Revision 1.2  2002/03/10 16:06:43  reinhold
// Cleaned up the class hierarchy, implemented some more features (should be quite finished now...)
//
// Revision 1.1.1.1  2002/03/09 15:38:45  reinhold
// Initial checin of the  project manager / List manager conduit.
//
//
//
#endif
