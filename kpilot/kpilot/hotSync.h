#ifndef _KPILOT_HOTSYNC_H
#define _KPILOT_HOTSYNC_H
/* hotSync.h                            KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


class QTimer;

#include "syncAction.h"
#include "syncStack.h"

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
	BackupAction(KPilotDeviceLink *, bool full);

	enum Status { Init,
		Error,
		FastBackup,
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
	bool checkBackupDirectory(QString backupDir);

private slots:
	void backupOneDB();

private:
	QTimer *fTimer;
	int fDBIndex;
	QString fBackupDir, fDatabaseDir;
	bool fFullBackup;
	QStringList fNoBackupDBs;
	QValueList<unsigned long> fNoBackupCreators;
	QStringList mDeviceDBs;
} ;

class FileInstallAction : public SyncAction
{
Q_OBJECT
public:
	FileInstallAction(KPilotDeviceLink *,
		const QString &fileDir);
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

	// TODO: not const because it calls logError(), which is
	// non-const (but might be - can signals be const, anyway?)
	bool resourceOK(const QString &, const QString &) /* const */ ;
} ;

class CleanupAction : public SyncAction
{
public:
	CleanupAction(KPilotDeviceLink * p);
	virtual ~CleanupAction();

protected:
	virtual bool exec();
} ;

#endif
