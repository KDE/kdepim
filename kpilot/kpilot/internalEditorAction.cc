/* internalEditorAction.cc                KPilot
**
** Copyright (C) 2003 by Dan Pilone
** Written 2003 by Reinhold Kainhofer
**
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

#include <options.h>

#include <qtimer.h>
#include <kmessagebox.h>

#include "kpilotConfig.h"
#include "internalEditorAction.h"

InternalEditorAction::InternalEditorAction(KPilotDeviceLink * p, int mode) :
	SyncAction(p, "internalSync")
{
	FUNCTIONSETUP;
}

bool InternalEditorAction::exec()
{
	FUNCTIONSETUP;
	emit logMessage(i18n("Syncronizing changes by the internal editors of KPilot:"));
	fInternalEditorSyncStatus=eSyncStarted;
	QTimer::singleShot(0, this, SLOT(syncDirtyDB()));
	return true;
}

void InternalEditorAction::syncDirtyDB()
{
	FUNCTIONSETUP;

	if (fInternalEditorSyncStatus!=eSyncDirtyDB)
	{
		fInternalEditorSyncStatus=eSyncDirtyDB;
		dirtyDBs=KPilotConfig::getConfig().getDirtyDatabases();
		emit logMessage(i18n("Databases with changed records: %s").arg(dirtyDBs.join(", ")));
		dbIter=dirtyDBs.begin();
	}
	if (dbIter==dirtyDBs.end())
	{
		QTimer::singleShot(0, this, SLOT(syncFlagsChangedDB()));
		return;
	}
#ifdef DEBUG
	DEBUGKPILOT<<"syncDirtyDB for DB "<<(*dbIter)<<endl;
#endif
	// open the local and the serial database and copy every
	// changed record from the PC to the handheld

	// TODO:

	QTimer::singleShot(0, this, SLOT(syncDirtyDB()));
}
void InternalEditorAction::syncFlagsChangedDB()
{
	FUNCTIONSETUP;
	if (fInternalEditorSyncStatus!=eSyncFlagsChangedDB)
	{
		fInternalEditorSyncStatus=eSyncFlagsChangedDB;
		dirtyDBs=KPilotConfig::getConfig().getFlagsChangedDatabases();
		emit logMessage(i18n("Databases with changed flags: %s").arg(dirtyDBs.join(", ")));
		dbIter=dirtyDBs.begin();
	}
	if (dbIter==dirtyDBs.end())
	{
		QTimer::singleShot(0, this, SLOT(syncAppBlockChangedDB()));
		return;
	}
#ifdef DEBUG
	DEBUGKPILOT<<"syncFlagsChangedDB for DB "<<(*dbIter)<<endl;
#endif
	// open the local and the serial database and copy the flags over
	// TODO
	// TODO: Is there a way to detect if the flags were changed on the handheld?

	QTimer::singleShot(0, this, SLOT(syncAppBlockChangedDB()));
}

void InternalEditorAction::syncAppBlockChangedDB()
{
	FUNCTIONSETUP;
	if (fInternalEditorSyncStatus!=eSyncAppBlockChangedDB)
	{
		fInternalEditorSyncStatus=eSyncAppBlockChangedDB;
		dirtyDBs=KPilotConfig::getConfig().getAppBlockChangedDatabases();
		emit logMessage(i18n("Databases with changed AppBlock: %s").arg(dirtyDBs.join(", ")));
		dbIter=dirtyDBs.begin();
	}
	if (dbIter==dirtyDBs.end())
	{
		QTimer::singleShot(0, this, SLOT(cleanup()));
		return;
	}
#ifdef DEBUG
	DEBUGKPILOT<<"syncAppBlockChangedDB for DB "<<(*dbIter)<<endl;
#endif
	// open the local and the serial database and copy the app block over
	// TODO
	// TODO: Is there a way to detect if the app block was changed on the handheld?

	QTimer::singleShot(0, this, SLOT(syncAppBlockChangedDB()));
}

void InternalEditorAction::cleanup()
{
	FUNCTIONSETUP;
	fInternalEditorSyncStatus=eSyncFinished;
	emit syncDone(this);
}

#include "internalEditorAction.moc"
