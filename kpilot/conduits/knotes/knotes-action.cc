/* knotes-action.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the SyncAction for the knotes-conduit plugin.
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

#include "options.h"

#include <qmap.h>
#include <qtimer.h>

#include <kapplication.h>

#include <kconfig.h>
#include <dcopclient.h>

#include <time.h>  // required by pilot-link includes

#include <pi-memo.h>

#include "pilotMemo.h"
#include "pilotSerialDatabase.h"

#include "KNotesIface_stub.h"

#include "knotes-factory.h"

#include "knotes-action.moc"


class NoteAndMemo
{
public:
	NoteAndMemo() : noteId(-1),memoId(-1) { } ;
	NoteAndMemo(int noteid,int memoid) : noteId(noteid),memoId(memoid) { } ;

	int memo() const { return memoId; } ;
	int note() const { return noteId; } ;
	bool valid() const { return (noteId>0) && (memoId>0); } ;


	static NoteAndMemo findNote(const QValueList<NoteAndMemo> &,int note);
	static NoteAndMemo findMemo(const QValueList<NoteAndMemo> &,int memo);

protected:
	int noteId;
	int memoId;
} ;

NoteAndMemo NoteAndMemo::findNote(const QValueList<NoteAndMemo> &l ,int note)
{
	FUNCTIONSETUP;

	for (QValueList<NoteAndMemo>::ConstIterator it =l.begin();
		it != l.end();
		++it)
	{
		if ((*it).note()==note) return *it;
	}

	return NoteAndMemo();
}

NoteAndMemo NoteAndMemo::findMemo(const QValueList<NoteAndMemo> &l ,int memo)
{
	FUNCTIONSETUP;

	for (QValueList<NoteAndMemo>::ConstIterator it =l.begin();
		it != l.end();
		++it)
	{
		if ((*it).memo()==memo) return *it;
	}

	return NoteAndMemo();
}

class KNotesAction::KNotesActionPrivate
{
public:
	KNotesActionPrivate() :
		fDCOP(0L),
		fKNotes(0L),
		fTimer(0L),
		fDatabase(0L),
		fCounter(0)
	{ } ;

	// These are  the notes that we got from KNotes
	QMap <int,QString> fNotes;
	// This iterates through that list; it's in here because
	// we use slots to process one item at a time and need
	// to keep track of where we are between slot calls.
	QMap <int,QString>::ConstIterator fIndex;
	// The DCOP client for this application, and the KNotes stub.
	DCOPClient *fDCOP;
	KNotesIface_stub *fKNotes;
	// The timer for invoking process() to do some more work.
	QTimer *fTimer;
	// The database we're working with (MemoDB)
	PilotSerialDatabase *fDatabase;
	// Some counter that needs to be preserved between calls to
	// process(). Typically used to note hom much work is done.
	int fCounter;

	// We need to translate between the ids that KNotes uses and
	// Pilot id's, so we make a list of pairs.
	//
	QValueList<NoteAndMemo> fIdList;
} ;


/* static */ const char * const KNotesAction::noteIdsKey="NoteIds";
/* static */ const char * const KNotesAction::memoIdsKey="MemoIds";


KNotesAction::KNotesAction(KPilotDeviceLink *o,
	const char *n, const QStringList &a) :
	ConduitAction(o,n,a),
	fP(new KNotesActionPrivate)
{
	FUNCTIONSETUP;


	fP->fDCOP = KApplication::kApplication()->dcopClient();

	if (!fP->fDCOP)
	{
		kdWarning() << k_funcinfo
			<< ": Can't get DCOP client."
			<< endl;
	}
}

/* virtual */ KNotesAction::~KNotesAction()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fP->fTimer);
	KPILOT_DELETE(fP->fKNotes);
	KPILOT_DELETE(fP->fDatabase);
	KPILOT_DELETE(fP);
}

/* virtual */ void KNotesAction::exec()
{
	FUNCTIONSETUP;

	if (!fP->fDCOP) return;
	if (!PluginUtility::isRunning("knotes")) return;
	if (!fConfig) return;

	fP->fKNotes = new KNotesIface_stub("knotes","KNotesIface");

	fP->fNotes = fP->fKNotes->notes();
	fP->fDatabase = new PilotSerialDatabase(pilotSocket(),"MemoDB",this,"MemoDB");

	if (isTest())
	{
		listNotes();
	}
	else
	{
		fP->fTimer = new QTimer(this);
		fStatus = Init;
		resetIndexes();

		connect(fP->fTimer,SIGNAL(timeout()),SLOT(process()));

		fP->fTimer->start(0,false);
	}
}

void KNotesAction::resetIndexes()
{
	FUNCTIONSETUP;

	fP->fCounter = 0;
	fP->fIndex = fP->fNotes.begin();
}

void KNotesAction::listNotes()
{
	FUNCTIONSETUP;

	QMap<int,QString>::ConstIterator i = fP->fNotes.begin();
	while (i != fP->fNotes.end())
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": "
			<< i.key()
			<< "->"
			<< i.data()
			<< (fP->fKNotes->isNew("kpilot",i.key()) ?
				" (new)" : "" )
			<< endl;
#endif
		i++;
	}

	emit syncDone(this);
}

/* slot */ void KNotesAction::process()
{
	switch(fStatus)
	{
	case Init: 
		getAppInfo(); 
		getConfigInfo();
		break;
	case ModifiedNotesToPilot : 
		modifyNoteOnPilot(); 
		break;
	case NewNotesToPilot : 
		addNewNoteToPilot(); 
		break;
	case Cleanup : 
		cleanupMemos(); 
		break;
	default : 
		fP->fTimer->stop();
		emit syncDone(this);
	}
}


void KNotesAction::getConfigInfo()
{
	FUNCTIONSETUP;

	if (fConfig)
	{
		KConfigGroupSaver g(fConfig,KNotesConduitFactory::group);

		QValueList<int> notes;
		QValueList<int> memos;


		notes=fConfig->readIntListEntry(noteIdsKey);
		memos=fConfig->readIntListEntry(memoIdsKey);

		if (notes.count() != memos.count())
		{
			kdWarning() << k_funcinfo
				<< ": Notes and memo id lists don't match ("
				<< notes.count()
				<< ","
				<< memos.count()
				<< ")"
				<< endl;
		}

		QValueList<int>::ConstIterator iNotes = notes.begin();
		QValueList<int>::ConstIterator iMemos = memos.begin();

		while((iNotes != notes.end()) && (iMemos != memos.end()))
		{
			fP->fIdList.append(NoteAndMemo(*iNotes,*iMemos));
			++iNotes;
			++iMemos;
		}
	}
}

void KNotesAction::getAppInfo()
{
	FUNCTIONSETUP;


	unsigned char buffer[PilotDatabase::MAX_APPINFO_SIZE];
	int appInfoSize = fP->fDatabase->readAppBlock(buffer,PilotDatabase::MAX_APPINFO_SIZE);
	struct MemoAppInfo memoInfo;

	if (appInfoSize<0)
	{
		fStatus=Error;
		return;
	}

	unpack_MemoAppInfo(&memoInfo,buffer,appInfoSize);
	PilotDatabase::listAppInfo(&memoInfo.category);

	resetIndexes();
	fStatus=ModifiedNotesToPilot;
}


void KNotesAction::modifyNoteOnPilot()
{
	FUNCTIONSETUP;

	if (fP->fIndex == fP->fNotes.end())
	{
		if (fP->fCounter)
		{
			addSyncLogEntry(i18n("Modified one memo.",
				"Modified %n memos.",
				fP->fCounter));
		}

		resetIndexes();
		fStatus = NewNotesToPilot;
		return;
	}

	if (fP->fKNotes->isModified("kpilot",fP->fIndex.key()))
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": The note #"
			<< fP->fIndex.key()
			<< " with name "
			<< fP->fIndex.data()
			<< " is modified in KNotes."
			<< endl;
#endif

		NoteAndMemo nm = NoteAndMemo::findNote(fP->fIdList,
			fP->fIndex.key());

		if (nm.valid())
		{
			QString text = fP->fIndex.data() + "\n" ;
			text.append(fP->fKNotes->text(fP->fIndex.key()));

			const char *c = text.latin1();
			PilotMemo *a = new PilotMemo((void *)(const_cast<char *>(c)));
			PilotRecord *r = a->pack();
			r->setID(nm.memo());

			int newid = fP->fDatabase->writeRecord(r);

			if (newid != nm.memo())
			{
				kdWarning() << k_funcinfo
					<< ": Memo id changed during write? "
					<< "From "
					<< nm.memo()
					<< " to "
					<< newid
					<< endl;
			}
		}
	}

	++(fP->fIndex);
}

void KNotesAction::addNewNoteToPilot()
{
	FUNCTIONSETUP;

	if (fP->fIndex == fP->fNotes.end())
	{
		if (fP->fCounter)
		{
			addSyncLogEntry(i18n("Added one new memo.",
				"Added %n new memos.",
				fP->fCounter));
		}

		resetIndexes();
		fStatus = Cleanup;
		return;
	}

	if (fP->fKNotes->isNew("kpilot",fP->fIndex.key()))
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": The note #"
			<< fP->fIndex.key()
			<< " with name "
			<< fP->fIndex.data()
			<< " is new to the Pilot."
			<< endl;
#endif

		QString text = fP->fIndex.data() + "\n" ;
		text.append(fP->fKNotes->text(fP->fIndex.key()));

		const char *c = text.latin1();
		PilotMemo *a = new PilotMemo((void *)(const_cast<char *>(c)));
		PilotRecord *r = a->pack();

		int newid = fP->fDatabase->writeRecord(r);

		fP->fIdList.append(NoteAndMemo(fP->fIndex.key(),newid));

		delete r;
		delete a;
		
		fP->fCounter++;
	}

	++(fP->fIndex);
}

void KNotesAction::cleanupMemos()
{
	FUNCTIONSETUP;

	// Tell KNotes we're up-to-date
	fP->fKNotes->sync("kpilot");

	if (fConfig)
	{
		KConfigGroupSaver g(fConfig,KNotesConduitFactory::group);

		QValueList<int> notes;
		QValueList<int> memos;

		for (QValueList<NoteAndMemo>::ConstIterator i =
			fP->fIdList.begin();
			i!=fP->fIdList.end();
			++i)
		{
			notes.append((*i).note());
			memos.append((*i).memo());
		}

		fConfig->writeEntry(noteIdsKey,notes);
		fConfig->writeEntry(memoIdsKey,memos);
	}

	fStatus=Done;
}


/* virtual */ QString KNotesAction::statusString() const
{
	switch(fStatus)
	{
	case Init : return QString("Init");
	case NewNotesToPilot :
		return QString("NewNotesToPilot key=%1")
			.arg(fP->fIndex.key());
	case Done :
		return QString("Done");
	default :
		return QString("Unknown (%1)").arg(fStatus);
	}
}


// $Log$
// Revision 1.8  2002/02/23 20:57:40  adridg
// #ifdef DEBUG stuff
//
// Revision 1.7  2002/01/20 22:42:43  adridg
// CVS_SILENT: Administrative
//
// Revision 1.6  2001/12/31 09:24:25  adridg
// Cleanup, various fixes for runtime loading
//
// Revision 1.5  2001/12/20 22:55:44  adridg
// Making conduits save their configuration and doing syncs
//
// Revision 1.4  2001/12/02 22:08:24  adridg
// CVS_SILENT: I forget
//
// Revision 1.3  2001/10/31 23:46:51  adridg
// CVS_SILENT: Ongoing conduits ports
//
// Revision 1.2  2001/10/29 09:45:19  cschumac
// Make it compile.
//
// Revision 1.1  2001/10/16 21:44:53  adridg
// Split up some files, added behavior
//
// Revision 1.4  2001/10/10 22:39:49  adridg
// Some UI/Credits/About page patches
//
// Revision 1.3  2001/10/10 21:42:09  adridg
// Actually do part of a sync now
//
// Revision 1.2  2001/10/10 13:40:07  cschumac
// Compile fixes.
//
// Revision 1.1  2001/10/08 22:27:42  adridg
// New ui, moved to lib-based conduit
//
//

