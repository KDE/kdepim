#ifndef _KPILOT_HOTSYNC_H
#define _KPILOT_HOTSYNC_H
/* hotSync.h                            KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines SyncActions, which are used to perform some specific
** task during a HotSync. Conduits are not included here, nor are 
** sync actions requiring user interaction. Those can be found in the
** conduits subdirectory or interactiveSync.h.
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/


class QTimer;

#include <qstring.h>
#include <qstringlist.h>

#include "syncAction.h"


class TestLink : public SyncAction
{
Q_OBJECT

public:
	TestLink(KPilotDeviceLink *);

protected:
	virtual bool exec();
} ;

class BackupAction : public SyncAction
{
Q_OBJECT

public:
	BackupAction(KPilotDeviceLink *);

	enum Status { Init,
		Error,
		FullBackup,
		BackupIncomplete,
		BackupEnded,
		BackupComplete
		} ;
	virtual QString statusString() const;

protected:
	virtual bool exec();

private:
	/**
	* All manner of support functions for full backup.
	*/
	void endBackup();
	bool createLocalDatabase(DBInfo *);

private slots:
	void backupOneDB();

private:
	QTimer *fTimer;
	int fDBIndex;
	QString fDatabaseDir;
} ;

class FileInstallAction : public SyncAction
{
Q_OBJECT
public:
	FileInstallAction(KPilotDeviceLink *,
		const QString &fileDir,
		const QStringList &fileNames);
	virtual ~FileInstallAction();

	virtual QString statusString() const;

protected:
	virtual bool exec();

protected slots:
	void installNextFile();

private:
	int fDBIndex;
	QTimer *fTimer;
	QString fDir;
	QStringList fList;
} ;

class CleanupAction : public SyncAction
{
public:
	CleanupAction(KPilotDeviceLink * p);
	virtual ~CleanupAction();
	
protected:
	virtual bool exec();
} ;

// $Log$
// Revision 1.8  2002/05/14 22:57:40  adridg
// Merge from _BRANCH
//
// Revision 1.7.2.1  2002/04/04 20:28:28  adridg
// Fixing undefined-symbol crash in vcal. Fixed FD leak. Compile fixes
// when using PILOT_VERSION. kpilotTest defaults to list, like the options
// promise. Always do old-style USB sync (also works with serial devices)
// and runs conduits only for HotSync. KPilot now as it should have been
// for the 3.0 release.
//
// Revision 1.7  2002/01/25 21:43:12  adridg
// ToolTips->WhatsThis where appropriate; vcal conduit discombobulated - it doesn't eat the .ics file anymore, but sync is limited; abstracted away more pilot-link
//
// Revision 1.6  2001/12/29 15:45:02  adridg
// Lots of little changes for the syncstack
//
// Revision 1.5  2001/10/08 22:20:18  adridg
// Changeover to libkpilot, prepare for lib-based conduits
//
// Revision 1.4  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.3  2001/09/24 22:17:41  adridg
// () Removed lots of commented out code from previous incarnations.
// () Added a cleanup action.
// () Removed a heap-corruption bug caused by using QStringList & and
//    then deleting what it points to in FileInstallAction.
// () Removed deadlock when last file to install couldn't be read.
// () Moved RestoreAction to interactiveSync.{h,cc}, since I feel it
//    needs to ask "Are you sure?" at the very least.
//
// Revision 1.2  2001/09/23 18:24:59  adridg
// New syncing architecture
//
// Revision 1.1  2001/09/16 13:43:18  adridg
// Subclasses for hotSyncing
//
#endif
