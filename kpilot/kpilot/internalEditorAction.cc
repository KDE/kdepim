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

#include <pilotRecord.h>
#include <pilotLocalDatabase.h>
#include <pilotDatabase.h>
#include <pilotSerialDatabase.h>
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
		emit logMessage(i18n("Databases with changed records: %1").arg(dirtyDBs.join(", ")));
		dbIter=dirtyDBs.begin();
	}
	else
	{
		dbIter++;
	}
	if (dbIter==dirtyDBs.end())
	{
		KPilotConfig::getConfig().setDirtyDatabases(QStringList());
		KPilotConfig::getConfig().sync();
		QTimer::singleShot(0, this, SLOT(syncFlagsChangedDB()));
		return;
	}
#ifdef DEBUG
	DEBUGKPILOT<<"syncDirtyDB for DB "<<(*dbIter)<<endl;
#endif
	// open the local and the serial database and copy every
	// changed record from the PC to the handheld

	PilotRecord*rec=0L;
	PilotLocalDatabase*localDB=new PilotLocalDatabase(*dbIter);
	PilotSerialDatabase*serialDB=new PilotSerialDatabase(pilotSocket(), *dbIter);
	if (!localDB->isDBOpen() || !serialDB->isDBOpen())
	{
		emit logError(i18n("Unable to open the serial or local database for %1. "
			"Skipping it.").arg(*dbIter));
		goto nextDB;
	}
	while ( (rec=localDB->readNextModifiedRec()) )
	{
		int id=rec->getID();
#ifdef DEBUG
		DEBUGKPILOT<<"ID of modified record is "<<id<<endl;
		DEBUGKPILOT<<endl<<endl;
#endif
		if (id>0)
		{
			PilotRecord*serrec=serialDB->readRecordById(id);
			if (serrec && (serrec->getAttrib() & dlpRecAttrDirty))
			{
				// TODO: Treat the special databases different (i.e. for
				// Addresses, Todos, Events and notes display the contents
				// of the record.
				int res=KMessageBox::questionYesNo(0L, i18n("The record with ID %1 of the database \"%2\" was changed on the handheld and in the internal editor. Which version is the current one?").arg(id).arg(*dbIter), i18n("Conflict in %1").arg(*dbIter), i18n("&Internal Editor"), i18n("&Handheld"));
				if (res==KMessageBox::Ok)
				{
					serialDB->writeRecord(rec);
				}
				else
				{
					localDB->writeRecord(serrec);
				}
			}
			else
			{
				serialDB->writeRecord(rec);
			}
		}
		else
		{
#ifdef DEBUG
			DEBUGKPILOT<<"Generating ID for Record "<<rec->getID()<<" with data "<<endl;
			DEBUGKPILOT<<rec->getData()<<endl;
			DEBUGKPILOT<<"-----------------------------------------"<<endl;
#endif
			int id=serialDB->writeRecord(rec);
			rec->setID(id);
#ifdef DEBUG
			DEBUGKPILOT<<"New ID is "<<id<<endl;
			DEBUGKPILOT<<endl<<endl<<endl;
#endif
			//localDB->writeRecord(rec);
			localDB->writeID(rec);
		}
		KPILOT_DELETE(rec);
	}

nextDB:
	localDB->resetSyncFlags();
	KPILOT_DELETE(localDB);
	KPILOT_DELETE(serialDB);
	QTimer::singleShot(0, this, SLOT(syncDirtyDB()));
}

void InternalEditorAction::syncFlagsChangedDB()
{
	FUNCTIONSETUP;
	if (fInternalEditorSyncStatus!=eSyncFlagsChangedDB)
	{
		fInternalEditorSyncStatus=eSyncFlagsChangedDB;
		dirtyDBs=KPilotConfig::getConfig().getFlagsChangedDatabases();
		emit logMessage(i18n("Databases with changed flags: %1").arg(dirtyDBs.join(", ")));
		dbIter=dirtyDBs.begin();
	}
	else
	{
		dbIter++;
	}
	if (dbIter==dirtyDBs.end())
	{
		KPilotConfig::getConfig().setFlagsChangedDatabases(QStringList());
		KPilotConfig::getConfig().sync();
		QTimer::singleShot(0, this, SLOT(syncAppBlockChangedDB()));
		return;
	}

#ifdef DEBUG
	DEBUGKPILOT<<"syncFlagsChangedDB for DB "<<(*dbIter)<<endl;
#endif
emit logError(i18n("Setting the database flags on the handheld is not yet supported."));
QTimer::singleShot(0, this, SLOT(syncAppBlockChangedDB()));
return;

	PilotLocalDatabase*localDB=new PilotLocalDatabase(*dbIter);
	PilotSerialDatabase*serialDB=new PilotSerialDatabase(pilotSocket(), *dbIter);

	// open the local and the serial database and copy the flags over
	// TODO: Implement the copying
	// TODO: Is there a way to detect if the flags were changed on the handheld?

	KPILOT_DELETE(localDB);
	KPILOT_DELETE(serialDB);
	QTimer::singleShot(0, this, SLOT(syncAppBlockChangedDB()));
}

void InternalEditorAction::syncAppBlockChangedDB()
{
	FUNCTIONSETUP;
	if (fInternalEditorSyncStatus!=eSyncAppBlockChangedDB)
	{
		fInternalEditorSyncStatus=eSyncAppBlockChangedDB;
		dirtyDBs=KPilotConfig::getConfig().getAppBlockChangedDatabases();
		emit logMessage(i18n("Databases with changed AppBlock: %1").arg(dirtyDBs.join(", ")));
		dbIter=dirtyDBs.begin();
	}
	else
	{
		dbIter++;
	}
	if (dbIter==dirtyDBs.end())
	{
		KPilotConfig::getConfig().setAppBlockChangedDatabases(QStringList());
		KPilotConfig::getConfig().sync();
		QTimer::singleShot(0, this, SLOT(cleanup()));
		return;
	}
#ifdef DEBUG
	DEBUGKPILOT<<"syncAppBlockChangedDB for DB "<<(*dbIter)<<endl;
#endif

	PilotLocalDatabase*localDB=new PilotLocalDatabase(*dbIter);
	PilotSerialDatabase*serialDB=new PilotSerialDatabase(pilotSocket(), *dbIter);

	unsigned char*appBlock=new unsigned char[0xFFFF];
	int len=localDB->readAppBlock(appBlock, 0xFFFF);
	// TODO: Check if the app block was changed on the handheld, and if so, do conflict resolution
	serialDB->writeAppBlock(appBlock, len);

	KPILOT_DELETE(localDB);
	KPILOT_DELETE(serialDB);
	QTimer::singleShot(0, this, SLOT(syncAppBlockChangedDB()));
}

void InternalEditorAction::cleanup()
{
	FUNCTIONSETUP;
	fInternalEditorSyncStatus=eSyncFinished;
	emit syncDone(this);
}

#include "internalEditorAction.moc"
