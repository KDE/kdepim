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
#include <kdialog.h>
#include <qlayout.h>
#include <qlabel.h>
#include <ktextedit.h>

#include <pilotRecord.h>
#include <pilotLocalDatabase.h>
#include <pilotDatabase.h>
#include <pilotSerialDatabase.h>
#include "kpilotConfig.h"
#include "internalEditorAction.h"

#include <pilotAddress.h>
#include <pilotMemo.h>
#include <pilotDateEntry.h>
#include <pilotTodoEntry.h>

#ifdef USE_KHEXEDIT
#include "khexedit/byteseditinterface.h"
using namespace KHE;
#endif

InternalEditorAction::InternalEditorAction(KPilotDeviceLink * p) :
	SyncAction(p, "internalSync")
{
	FUNCTIONSETUP;
}

bool InternalEditorAction::exec()
{
	FUNCTIONSETUP;
	emit logMessage(i18n("Synchronizing changes by the internal editors of KPilot:"));
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
		dirtyDBs=KPilotSettings::dirtyDatabases();
		emit logMessage(i18n("Databases with changed records: %1").arg(dirtyDBs.join(", ")));
		dbIter=dirtyDBs.begin();
	}
	else
	{
		dbIter++;
	}
	if (dbIter==dirtyDBs.end())
	{
		KPilotSettings::setDirtyDatabases(QStringList());
		KPilotConfig::sync();
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
		int id=rec->id();
#ifdef DEBUG
		DEBUGKPILOT<<"ID of modified record is "<<id<<endl;
		DEBUGKPILOT<<endl<<endl;
#endif
		if (id>0)
		{
			PilotRecord*serrec=serialDB->readRecordById(id);
			if (serrec && (serrec->getAttrib() & dlpRecAttrDirty))
			{
				bool kpilotOverrides=queryUseKPilotChanges(*dbIter, id, rec, serrec, localDB);
				if (kpilotOverrides)
					serialDB->writeRecord(rec);
				else
					localDB->writeRecord(serrec);
			}
			else
				serialDB->writeRecord(rec);
		}
		else
		{
#ifdef DEBUG
			DEBUGKPILOT<<"Generating ID for Record "<<rec->id()<<" with data "<<endl;
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

bool InternalEditorAction::queryUseKPilotChanges(QString dbName, recordid_t id, PilotRecord*localrec, PilotRecord*serialrec, PilotDatabase*db)
{
	FUNCTIONSETUP;
	bool knownDB=true;
	QString localEntry, serialEntry, recType(i18n("record"));

	if (dbName=="AddressDB" && db)
	{
		struct AddressAppInfo fAppInfo;
		unsigned char *buffer = new unsigned char[PilotRecord::APP_BUFFER_SIZE];
		int appLen = db->readAppBlock(buffer, PilotRecord::APP_BUFFER_SIZE);
		unpack_AddressAppInfo(&fAppInfo, buffer, appLen);
		delete[] buffer;

		PilotAddress localAddr(fAppInfo, localrec);
		PilotAddress serialAddr(fAppInfo, serialrec);
		localEntry=localAddr.getTextRepresentation(true);
		serialEntry=serialAddr.getTextRepresentation(true);
		recType=i18n("address");
	}
	else
	if (dbName=="ToDoDB" && db)
	{
		struct ToDoAppInfo fAppInfo;
		unsigned char *buffer = new unsigned char[PilotRecord::APP_BUFFER_SIZE];
		int appLen = db->readAppBlock(buffer, PilotRecord::APP_BUFFER_SIZE);
		unpack_ToDoAppInfo(&fAppInfo, buffer, appLen);
		delete[] buffer;

		PilotTodoEntry localTodo(fAppInfo, localrec);
		PilotTodoEntry serialTodo(fAppInfo, serialrec);
		localEntry=localTodo.getTextRepresentation(true);
		serialEntry=serialTodo.getTextRepresentation(true);
		recType=i18n("todo entry");
	}
	else
	if (dbName=="MemoDB")
	{
		PilotMemo localMemo(localrec);
		PilotMemo serialMemo(serialrec);
		localEntry=localMemo.getTextRepresentation(true);
		serialEntry=serialMemo.getTextRepresentation(true);
		recType=i18n("memo");
	}
	else
	if (dbName=="DatebookDB")
	{
		PilotDateEntry localEvent(localrec);
		PilotDateEntry serialEvent(serialrec);
		localEntry=localEvent.getTextRepresentation(true);
		serialEntry=serialEvent.getTextRepresentation(true);
		recType=i18n("calendar entry");
	}
	else
		knownDB=false;

	QString dialogText(i18n("The %1 with ID %2 of the database \"%3\" was changed "
		"on the handheld and in the internal editor. Shall the changes in KPilot be copied to the handheld, and so override the changes there?").
		arg(recType).arg(id).arg(dbName));

	KDialogBase*resdlg=new KDialogBase(0L, "internalresolutiondialog", true,
		i18n("Conflict in database  %1").arg(*dbIter),
		KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true,
		i18n("Use KPilot"), i18n("Use Handheld") );
	resdlg->setButtonText(KDialogBase::Ok,  i18n("Use &KPilot"));
	resdlg->setButtonText(KDialogBase::Cancel, i18n("Use &Handheld"));

	QWidget*page=new QWidget(resdlg);
	resdlg->setMainWidget(page);
	QGridLayout*layout = new QGridLayout( page, 1, 1);

	QLabel *label=new QLabel(dialogText, page);
	label->setAlignment( QLabel::WordBreak );
	layout->addMultiCellWidget( label,  0,0, 0,1 );

 	layout->addItem( new QSpacerItem( 20, 10, QSizePolicy::Minimum,
		QSizePolicy::Fixed ), 1, 0 );

	if (knownDB)
	{
		label=new QLabel(i18n("Entry in KPilot"), page);
		layout->addWidget( label, 2,0);

		KTextEdit*textBrowser = new KTextEdit("<qt>"+localEntry+"</qt>", QString::null, page);
		textBrowser->setReadOnly(true);
		layout->addWidget( textBrowser, 3,0);

		label=new QLabel(i18n("Entry on Handheld"), page);
		layout->addWidget( label, 2,1);

		textBrowser = new KTextEdit("<qt>"+serialEntry+"</qt>", QString::null, page);
		textBrowser->setReadOnly(true);
		layout->addWidget( textBrowser, 3,1);
	}
	else
	{
#ifdef USE_KHEXEDIT
		label=new QLabel(i18n("Entry in KPilot"), page);
		layout->addMultiCellWidget( label, 2,2,0,1);

		// directly display the record's data:
		QWidget *hexEdit = KHE::createBytesEditWidget( page, "LocalBufferEdit" );
		if( hexEdit )
		{
			KHE::BytesEditInterface* hexEditIf = KHE::bytesEditInterface( hexEdit );
			Q_ASSERT( hexEditIf ); // This should not fail!
			if( hexEditIf )
			{
				hexEditIf->setData( localrec->getData(), localrec->getLen() );
// 					Do we need the following call at all???
//				hexEditIf->setMaxDataSize( localrec->getLen() );
				hexEditIf->setReadOnly( true );
			}
		}
		else
		{
			QLabel*tmpW = new QLabel( i18n("To view and edit the record data, please install a hex editor (e.g. khexedit from kdeutils)."), page );
			tmpW->setBackgroundMode( Qt::PaletteMid );
			tmpW->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter | Qt::WordBreak);
			tmpW->setFrameShape( QFrame::Panel );
			tmpW->setFrameShadow( QFrame::Sunken );
			hexEdit = tmpW;
		}
		layout->addMultiCellWidget( hexEdit, 3,3,0,1);

		label=new QLabel(i18n("Entry on Handheld"), page);
		layout->addMultiCellWidget( label, 4,4,0,1);

		// directly display the record's data:
		hexEdit = KHE::createBytesEditWidget( page, "SerialBufferEdit" );
		if( hexEdit )
		{
			KHE::BytesEditInterface* hexEditIf = KHE::bytesEditInterface( hexEdit );
			Q_ASSERT( hexEditIf ); // This should not fail!
			if( hexEditIf )
			{
				hexEditIf->setData( serialrec->getData(), serialrec->getLen() );
// 					Do we need the following call at all???
//				hexEditIf->setMaxDataSize( serialrec->getLen() );
				hexEditIf->setReadOnly( true );
			}
		}
		else
		{
			QLabel*tmpW = new QLabel( i18n("To view and edit the record data, please install a hex editor (e.g. khexedit from kdeutils)."), page );
			tmpW->setBackgroundMode( Qt::PaletteMid );
			tmpW->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter | Qt::WordBreak);
			tmpW->setFrameShape( QFrame::Panel );
			tmpW->setFrameShadow( QFrame::Sunken );
			hexEdit = tmpW;
		}
		layout->addMultiCellWidget( hexEdit, 5,5,0,1);
#endif
	}

	int res=resdlg->exec();
	KPILOT_DELETE(resdlg);

	return res==KDialogBase::Accepted;
}


void InternalEditorAction::syncFlagsChangedDB()
{
	FUNCTIONSETUP;
	if (fInternalEditorSyncStatus!=eSyncFlagsChangedDB)
	{
		fInternalEditorSyncStatus=eSyncFlagsChangedDB;
		dirtyDBs=KPilotSettings::flagsChangedDatabases();
		emit logMessage(i18n("Databases with changed flags: %1").arg(dirtyDBs.join(", ")));
		dbIter=dirtyDBs.begin();
	}
	else
	{
		dbIter++;
	}
	if (dbIter==dirtyDBs.end())
	{
		KPilotSettings::setFlagsChangedDatabases(QStringList());
		KPilotConfig::sync();
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
		dirtyDBs=KPilotSettings::appBlockChangedDatabases();
		emit logMessage(i18n("Databases with changed AppBlock: %1").arg(dirtyDBs.join(", ")));
		dbIter=dirtyDBs.begin();
	}
	else
	{
		dbIter++;
	}
	if (dbIter==dirtyDBs.end())
	{
		KPilotSettings::setAppBlockChangedDatabases(QStringList());
		KPilotConfig::sync();
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
