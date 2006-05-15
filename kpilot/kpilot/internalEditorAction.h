#ifndef _INTERNALEDITORACTION_H_
#define _INTERNALEDITORACTION_H_
/* internalEditorAction.h                            KPilot
**
** Copyright (C) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
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



#include "syncAction.h"
#include "syncStack.h"
class QStringList;

class InternalEditorAction : public SyncAction
{
Q_OBJECT

public:
	InternalEditorAction(KPilotDeviceLink *);
	~InternalEditorAction() {}

protected:
	virtual bool exec();

private:
private slots:
	void syncDirtyDB();
	void syncFlagsChangedDB();
	void syncAppBlockChangedDB();
	void cleanup();

private:
	bool queryUseKPilotChanges(QString dbName, recordid_t id,
		PilotRecord*localrec, PilotRecord*serialrec, PilotDatabase*db);
	QStringList dirtyDBs;
	QStringList::Iterator dbIter;
	enum eInternalEditorSyncStatus {
		eSyncStarted,
		eSyncDirtyDB,
		eSyncFlagsChangedDB,
		eSyncAppBlockChangedDB,
		eSyncFinished
	} fInternalEditorSyncStatus;
} ;

#endif
