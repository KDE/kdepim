/* knotes-action.cc                      KPilot
**
** Copyright (C) 2001,2002 by Dan Pilone
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
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


typedef QString KNoteID_t;
typedef const QString &KNoteID_pt;

class NoteAndMemo
{
public:
	NoteAndMemo() : noteId(),memoId(-1) { } ;
	NoteAndMemo(KNoteID_pt noteid,int memoid) : noteId(noteid),memoId(memoid) { } ;

	int memo() const { return memoId; } ;
	KNoteID_t note() const { return noteId; } ;
	bool valid() const { return (!noteId.isEmpty()) && (memoId>0); } ;
	QString toString() const { return CSL1("<%1,%2>").arg(noteId).arg(memoId); } ;

	static NoteAndMemo findNote(const QValueList<NoteAndMemo> &,KNoteID_pt note);
	static NoteAndMemo findMemo(const QValueList<NoteAndMemo> &,int memo);

protected:
	KNoteID_t noteId;
	int memoId;
} ;

NoteAndMemo NoteAndMemo::findNote(const QValueList<NoteAndMemo> &l ,KNoteID_pt note)
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
		// fDatabase(0L),
		fCounter(0)
	{ } ;

	// This is the collection of  notes held by KNotes and
        // returned by the notes() DCOP call.
	QMap <KNoteID_t,QString> fNotes;
	// This iterates through that list; it's in here because
	// we use slots to process one item at a time and need
	// to keep track of where we are between slot calls.
	QMap <KNoteID_t,QString>::ConstIterator fIndex;
	// The DCOP client for this application, and the KNotes stub.
	DCOPClient *fDCOP;
	KNotesIface_stub *fKNotes;
	// The timer for invoking process() to do some more work.
	QTimer *fTimer;
	// The database we're working with (MemoDB)
	// PilotSerialDatabase *fDatabase;
	// Some counter that needs to be preserved between calls to
	// process(). Typically used to note hom much work is done.
	int fCounter;

	// We need to translate between the ids that KNotes uses and
	// Pilot id's, so we make a list of pairs.
	//
	QValueList<NoteAndMemo> fIdList;
} ;


/* static */ const char * const KNotesAction::noteIdsKey="KNoteIds";
/* static */ const char * const KNotesAction::memoIdsKey="MemoIds";


KNotesAction::KNotesAction(KPilotDeviceLink *o,
	const char *n, const QStringList &a) :
	ConduitAction(o,!n ? "knotes-conduit" : n,a),
	fP(new KNotesActionPrivate)
{
	FUNCTIONSETUP;


	if (fP) fP->fDCOP = KApplication::kApplication()->dcopClient();

	if (fP && !fP->fDCOP)
	{
		kdWarning() << k_funcinfo
			<< ": Can't get DCOP client."
			<< endl;
	}
}

/* virtual */ KNotesAction::~KNotesAction()
{
	FUNCTIONSETUP;

	if (fP)
	{
		KPILOT_DELETE(fP->fTimer);
		KPILOT_DELETE(fP->fKNotes);
		KPILOT_DELETE(fP);
	}
}

/* virtual */ bool KNotesAction::exec()
{
	FUNCTIONSETUP;

	QString e;
	if (!fP || !fP->fDCOP)
	{
		emit logError(i18n("No DCOP connection could be made. The "
			"conduit cannot function like this."));
		return false;

	}
	if (!PluginUtility::isRunning("knotes"))
	{
		emit logError(i18n("KNotes is not running. The conduit must "
			"be able to make a DCOP connection to KNotes "
			"for synchronization to take place. "
			"Please start KNotes and try again."));
		return false;
	}

	if (!fConfig) return false;

	fP->fKNotes = new KNotesIface_stub("knotes","KNotesIface");

	fP->fNotes = fP->fKNotes->notes();

	// Database names seem to be latin1
	openDatabases(QString::fromLatin1("MemoDB"));

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

	return true;
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

	QMap<KNoteID_t,QString>::ConstIterator i = fP->fNotes.begin();
	while (i != fP->fNotes.end())
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": "
			<< i.key()
			<< "->"
			<< i.data()
			<< (fP->fKNotes->isNew(CSL1("kpilot"),i.key()) ?
				" (new)" : "" )
			<< endl;
#endif
		i++;
	}

	emit syncDone(this);
}

/* slot */ void KNotesAction::process()
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Now in state " << fStatus << endl;
#endif

	switch(fStatus)
	{
	case Init:
		getAppInfo();
		getConfigInfo();
		break;
	case ModifiedNotesToPilot :
		if (modifyNoteOnPilot())
		{
			resetIndexes();
			fStatus = NewNotesToPilot;
		}
		break;
	case NewNotesToPilot :
		if (addNewNoteToPilot())
		{
			resetIndexes();
			fStatus = MemosToKNotes;
			fDatabase->resetDBIndex();
		}
		break;
	case MemosToKNotes :
		if (syncMemoToKNotes())
		{
			fStatus=Cleanup;
		}
		break;
	case Cleanup :
		cleanupMemos();
		break;
	default :
		if (fP->fTimer) fP->fTimer->stop();
		emit syncDone(this);
	}
}


void KNotesAction::getConfigInfo()
{
	FUNCTIONSETUP;

	if (fConfig)
	{
		KConfigGroupSaver g(fConfig,KNotesConduitFactory::group);

		QValueList<KNoteID_t> notes;
		QValueList<int> memos;


                // Make this match the type of KNoteID_t !
		notes=fConfig->readListEntry(noteIdsKey);
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
			notes.clear();
			memos.clear();
			fFirstSync = true;
		}

		QValueList<KNoteID_t>::ConstIterator iNotes = notes.begin();
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
	int appInfoSize = fDatabase->readAppBlock(buffer,PilotDatabase::MAX_APPINFO_SIZE);
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

	addSyncLogEntry(i18n("[KNotes conduit: "));
}


bool KNotesAction::modifyNoteOnPilot()
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
		else
		{
			addSyncLogEntry(TODO_I18N("No memos were changed."));
		}
		return true;
	}

	if (fP->fKNotes->isModified(CSL1("kpilot"),fP->fIndex.key()))
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
			QString text = fP->fIndex.data() + CSL1("\n") ;
			text.append(fP->fKNotes->text(fP->fIndex.key()));

			PilotMemo *a = new PilotMemo(text);
			PilotRecord *r = a->pack();
			r->setID(nm.memo());

			int newid = fDatabase->writeRecord(r);

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
		else
		{
			kdWarning() << ": Modified note unknown to Pilot" << endl;
		}

		fP->fCounter++;
	}

	++(fP->fIndex);
	return false;
}

bool KNotesAction::addNewNoteToPilot()
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
		else
		{
			addSyncLogEntry(TODO_I18N("No memos were added."));
		}
		return true;
	}

	if (fP->fKNotes->isNew(CSL1("kpilot"),fP->fIndex.key()))
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

		QString text = fP->fIndex.data() + CSL1("\n") ;
		text.append(fP->fKNotes->text(fP->fIndex.key()));

		PilotMemo *a = new PilotMemo(text);
		PilotRecord *r = a->pack();

		int newid = fDatabase->writeRecord(r);

		fP->fIdList.append(NoteAndMemo(fP->fIndex.key(),newid));

		delete r;
		delete a;

		fP->fCounter++;
	}

	++(fP->fIndex);
	return false;
}

bool KNotesAction::syncMemoToKNotes()
{
	FUNCTIONSETUP;

	PilotRecord *rec = fDatabase->readNextModifiedRec();
	if (!rec)
	{
		if (fP->fCounter)
		{
			addSyncLogEntry(i18n("Added one memo to KNotes.",
				"Added %n memos to KNotes.",fP->fCounter));
		}
		else
		{
			addSyncLogEntry(TODO_I18N("No memos added to KNotes."));
		}
		return true;
	}

	fP->fCounter++;

	PilotMemo *memo = new PilotMemo(rec);
	NoteAndMemo m = NoteAndMemo::findMemo(fP->fIdList,memo->id());

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Looking at memo "
		<< memo->id()
		<< " which was found "
		<< m.toString()
		<< endl;
#endif

	if (m.valid())
	{
		// We knew about the note already, but it
		// has changed on the Pilot.
		//
		//
		if (memo->isDeleted())
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": It's been deleted." << endl;
#endif
			fP->fKNotes->killNote(m.note());
		}
		else
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": It's just modified." << endl;
			DEBUGCONDUIT << fname << ": <"
				<< fP->fNotes[m.note()]
				<< "> <"
				<< memo->shortTitle()
				<< ">"
				<< endl;
#endif
			if (fP->fNotes[m.note()] != memo->shortTitle())
			{
				// Name changed. KNotes might complain though.
				fP->fKNotes->setName(m.note(),memo->shortTitle());
			}
			fP->fKNotes->setText(m.note(),memo->text());
		}
	}
	else
	{
		if (memo->isDeleted())
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": It's new and deleted." << endl;
#endif
			// Do nothing, it's new and deleted at the same time
		}
		else
		{
			KNoteID_t i = fP->fKNotes->newNote(memo->shortTitle(),memo->text());
			fP->fIdList.append(NoteAndMemo(i,memo->id()));
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": It's new with knote id " << i << endl;
#endif
		}
	}

	if (memo) delete memo;
	if (rec) delete rec;

	return false;
}


void KNotesAction::cleanupMemos()
{
	FUNCTIONSETUP;

	// Tell KNotes we're up-to-date
	fP->fKNotes->sync(CSL1("kpilot"));

	if (fConfig)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": Writing "
			<< fP->fIdList.count()
			<< " pairs to the config file."
			<< endl;
		DEBUGCONDUIT << fname
			<< ": The config file is read-only: "
			<< fConfig->isReadOnly()
			<< endl;
#endif

		KConfigGroupSaver g(fConfig,KNotesConduitFactory::group);

		QValueList<KNoteID_t> notes;
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
		fConfig->sync();
	}

	fStatus=Done;
	fDatabase->cleanup();
	fDatabase->resetSyncFlags();

	addSyncLogEntry(CSL1("]\n"));
}


/* virtual */ QString KNotesAction::statusString() const
{
	switch(fStatus)
	{
	case Init : return CSL1("Init");
	case NewNotesToPilot :
		return CSL1("NewNotesToPilot key=%1")
			.arg(fP->fIndex.key());
	case Done :
		return CSL1("Done");
	default :
		return CSL1("Unknown (%1)").arg(fStatus);
	}
}
