#ifndef _KPILOT_HOTSYNC_H
#define _KPILOT_HOTSYNC_H
/* hotSync.h                            KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2006 Adriaan de Groot <groot@kde.org>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


class QTimer;

#include "syncAction.h"

class TestLink : public SyncAction
{
Q_OBJECT

public:
	TestLink(KPilotLink *);

protected:
	virtual bool exec();
} ;

class BackupAction : public SyncAction
{
Q_OBJECT

public:
	/** Constructor. Back up all the databases on
	*   the link to a directory on the local disk.
	*   If @p full is @c true, then a full backup,
	*   including applications, is done. Otherwise,
	*   only user data is backed-up.
	*
	* @see setDirectory()
	*/
	BackupAction(KPilotLink *, bool full);

	enum Status { Init,
		Error,
		FastBackup,
		FullBackup,
		BackupIncomplete,
		BackupEnded,
		BackupComplete
		} ;
	virtual QString statusString() const;

	/** By default, a path based on the user name (either
	*   on the handheld or set in KPilot) is used to
	*   determine the backup directory name ( generally
	*   $KDEHOME/share/apps/kpilot/DBBackup/_user_name_ ).
	*   Use setDirectory() to change that and use a given
	*   @p path as target for the backup. Use an empty
	*   @p path to restore the default behavior of using
	*   the username.
	*/
	void setDirectory( const QString &path );

protected:
	virtual bool exec();

private:
	/** Finish the backup and clean up resources. */
	void endBackup();

	/** Copy the database indicated by @p info to the local
	*   disk; returns @c false on failure.
	*/
	bool createLocalDatabase(DBInfo *info);

	/** Make sure that the backup directory @p backupDir
	*   exists and is a directory; returns @c false
	*   if this is not the case. This method will try
	*   to create the directory if it doesn't exist yet.
	*/
	static bool checkBackupDirectory( const QString &backupDir );

private slots:
	/** Implementation detail: databases get backed-up
	*   one at a time because the backup function in
	*   pilot-link isn't threaded.
	*/
	void backupOneDB();

private:
	class Private;
	Private *fP;
} ;

class FileInstallAction : public SyncAction
{
Q_OBJECT
public:
	FileInstallAction(KPilotLink *,
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
	CleanupAction(KPilotLink * p);
	virtual ~CleanupAction();

protected:
	virtual bool exec();
} ;

#endif
