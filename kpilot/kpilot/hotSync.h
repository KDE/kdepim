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
** Bug reports and questions can be sent to groot@kde.org
*/


class QTimer;

#include <qstring.h>
#include <qstringlist.h>

#include "kpilotlink.h"

class TestLink : public SyncAction
{
Q_OBJECT

public:
	TestLink(KPilotDeviceLink *);

public slots:
	virtual void exec();
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

public slots:
	virtual void exec();

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

public slots:
	virtual void exec();

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
	CleanupAction(KPilotDeviceLink *p) : SyncAction(p) { } ;

	virtual void exec();
} ;

// $Log$
// Revision 1.2  2001/09/23 18:24:59  adridg
// New syncing architecture
//
// Revision 1.1  2001/09/16 13:43:18  adridg
// Subclasses for hotSyncing
//
#endif
