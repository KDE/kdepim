/* KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2002,2003,2004 by Adriaan de Groot
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

#undef DEBUGAREA
#define DEBUGAREA	DEBUGAREA_CONDUIT

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
#include "knotesconduitSettings.h"

extern "C"
{

long version_conduit_knotes = KPILOT_PLUGIN_API;
const char *id_conduit_knotes = "$Id$" ;

}

typedef QString KNoteID_t;
typedef const QString &KNoteID_pt;

class NoteAndMemo
{
public:
	NoteAndMemo() : noteId(),memoId(-1) { } ;
	NoteAndMemo(KNoteID_pt noteid,int memoid) : noteId(noteid),memoId(memoid) { } ;
	bool operator ==(const NoteAndMemo &p) const
	{
		return (p.memo()==memoId) && (p.note()==noteId);
	}

	int memo() const { return memoId; } ;
	KNoteID_t note() const { return noteId; } ;
	inline bool valid() const { return (memoId>0) && (!noteId.isEmpty()) ; } ;
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

	for (QValueList<NoteAndMemo>::ConstIterator it = l.begin();
		it != l.end();
		++it)
	{
		if ((*it).note()==note) return *it;
	}

	return NoteAndMemo();
}

NoteAndMemo NoteAndMemo::findMemo(const QValueList<NoteAndMemo> &l , int memo)
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
		fDeleteCounter(0),
		fModifiedNotesCounter(0),
		fModifiedMemosCounter(0),
		fAddedNotesCounter(0),
		fAddedMemosCounter(0),
		fDeletedNotesCounter(0),
		fDeletedMemosCounter(0),
		fDeleteNoteForMemo(false)
	{ } ;
	~KNotesActionPrivate()
	{
		KPILOT_DELETE(fKNotes);
		KPILOT_DELETE(fTimer);
	}

	// This is the collection of  notes held by KNotes and
        // returned by the notes() DCOP call.
	QMap <KNoteID_t,QString> fNotes;

	// This iterates through that list; it's in here because
	// we use slots to process one item at a time and need
	// to keep track of where we are between slot calls.
	QMap <KNoteID_t,QString>::ConstIterator fIndex;

	// The record index we're dealing with. Used by
	// CopyHHToPC sync only.
	int fRecordIndex;

	// The DCOP client for this application, and the KNotes stub.
	DCOPClient *fDCOP;
	KNotesIface_stub *fKNotes;

	// The timer for invoking process() to do some more work.
	QTimer *fTimer;

	// The database we're working with (MemoDB)
	// PilotSerialDatabase *fDatabase;
	// Some counter that needs to be preserved between calls to
	// process(). Typically used to note how much work is done.
	int fDeleteCounter; // Count deleted memos as well.
	unsigned int fModifiedNotesCounter; // Count modified KNotes.
	unsigned int fModifiedMemosCounter;
	unsigned int fAddedNotesCounter;
	unsigned int fAddedMemosCounter;
	unsigned int fDeletedNotesCounter;
	unsigned int fDeletedMemosCounter;

	// We need to translate between the ids that KNotes uses and
	// Pilot id's, so we make a list of pairs.
	//
	QValueList<NoteAndMemo> fIdList;

	// Setting to delete a KNote when the corresponding memo
	// has been deleted.
	//
	bool fDeleteNoteForMemo;
} ;



KNotesAction::KNotesAction(KPilotDeviceLink *o,
	const char *n, const QStringList &a) :
	ConduitAction(o,n ? n : "knotes-conduit",a),
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

	KPILOT_DELETE(fP);
}

/* virtual */ bool KNotesAction::exec()
{
	FUNCTIONSETUP;

	QString e;
	if (!fP || !fP->fDCOP)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< "No DCOP connection." << endl;
#endif
		emit logError(i18n("No DCOP connection could be made. The "
			"conduit cannot function without DCOP."));
		return false;

	}


	QCString knotesAppname = "knotes" ;
	if (!PluginUtility::isRunning(knotesAppname))
	{
		knotesAppname = "kontact" ;
		if (!PluginUtility::isRunning(knotesAppname))
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": KNotes not running." << endl;
#endif
			emit logError(i18n("KNotes is not running. The conduit must "
				"be able to make a DCOP connection to KNotes "
				"for synchronization to take place. "
				"Please start KNotes and try again."));
			return false;
		}
	}

	fP->fKNotes = new KNotesIface_stub(knotesAppname,"KNotesIface");

	fP->fNotes = fP->fKNotes->notes();
	if (fP->fKNotes->status() != DCOPStub::CallSucceeded)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< "Can not get list of notes from KNotes.." << endl;
#endif
		emit logError(i18n("Could not retrieve list of notes from KNotes. "
			"The KNotes conduit will not be run."));
		return false;

	}

	// Database names seem to be latin1
	if (!openDatabases(CSL1("MemoDB")))
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< "Can not open databases." << endl;
#endif
		emit logError(i18n("Could not open MemoDB on the Handheld."));
		return false;
	}

	if (isTest())
	{
		listNotes();
		return delayDone();
	}

	fP->fTimer = new QTimer(this);
	fActionStatus = Init;
	resetIndexes();

	connect(fP->fTimer,SIGNAL(timeout()),SLOT(process()));
	fP->fTimer->start(0,false);

	return true;
}

void KNotesAction::resetIndexes()
{
	FUNCTIONSETUP;

	fP->fRecordIndex = 0;
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


#ifdef DEBUG
	DEBUGCONDUIT << fname << ": "
		<< "Sync direction: " << getSyncDirection() << endl;
#endif
}

/* slot */ void KNotesAction::process()
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Now in state " << fActionStatus << endl;
#endif

	switch(fActionStatus)
	{
	case Init:
		resetIndexes();
		getAppInfo();
		getConfigInfo();
		switch(getSyncDirection())
		{
		case SyncAction::eDefaultSync:
		case SyncAction::eTest:
		case SyncAction::eBackup:
		case SyncAction::eRestore:
			// Impossible!
			fActionStatus = Done;
			break;
		case SyncAction::eCopyHHToPC :
			listNotes(); // Debugging
			fActionStatus = MemosToKNotes;
			break;
		case SyncAction::eFastSync:
		case SyncAction::eHotSync:
		case SyncAction::eFullSync:
		case SyncAction::eCopyPCToHH:
			fActionStatus = ModifiedNotesToPilot;
			break;
		}
		break;
	case ModifiedNotesToPilot :
		if (modifyNoteOnPilot())
		{
			resetIndexes();
			fActionStatus = DeleteNotesOnPilot;
		}
		break;
	case DeleteNotesOnPilot:
		if (deleteNoteOnPilot())
		{
			resetIndexes();
			fActionStatus = NewNotesToPilot;
		}
		break;
	case NewNotesToPilot :
		if (addNewNoteToPilot())
		{
			resetIndexes();
			fDatabase->resetDBIndex();
			switch(getSyncDirection())
			{
			case SyncAction::eDefaultSync:
			case SyncAction::eTest:
			case SyncAction::eBackup:
			case SyncAction::eRestore:
			case SyncAction::eCopyHHToPC :
				// Impossible!
				fActionStatus = Done;
				break;
			case SyncAction::eFastSync:
			case SyncAction::eHotSync:
			case SyncAction::eFullSync:
				fActionStatus = MemosToKNotes;
				break;
			case SyncAction::eCopyPCToHH:
				fActionStatus = Cleanup;
				break;
			}
		}
		break;
	case MemosToKNotes :
		if (syncMemoToKNotes())
		{
			fActionStatus=Cleanup;
		}
		break;
	case Cleanup :
		cleanupMemos();
		break;
	default :
		if (fP->fTimer) fP->fTimer->stop();
		delayDone();
	}
}


void KNotesAction::getConfigInfo()
{
	FUNCTIONSETUP;

	KNotesConduitSettings::self()->readConfig();

	fP->fDeleteNoteForMemo = KNotesConduitSettings::deleteNoteForMemo();

	QValueList<KNoteID_t> notes;
	QValueList<int> memos;

	// Make this match the type of KNoteID_t !
	notes=KNotesConduitSettings::noteIds();
	memos=KNotesConduitSettings::memoIds();

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
		setFirstSync( true );
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

void KNotesAction::getAppInfo()
{
	FUNCTIONSETUP;


	unsigned char buffer[PilotDatabase::MAX_APPINFO_SIZE];
	int appInfoSize = fDatabase->readAppBlock(buffer,PilotDatabase::MAX_APPINFO_SIZE);
	struct MemoAppInfo memoInfo;

	if (appInfoSize<0)
	{
		fActionStatus=Error;
		return;
	}

	unpack_MemoAppInfo(&memoInfo,buffer,appInfoSize);
	PilotAppCategory::dumpCategories(memoInfo.category);

	resetIndexes();
}


bool KNotesAction::modifyNoteOnPilot()
{
	FUNCTIONSETUP;

	if (fP->fIndex == fP->fNotes.end())
	{
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
			QString text,title,body;
			title = fP->fIndex.data();
			body = fP->fKNotes->text(fP->fIndex.key());
			if (body.startsWith(title))
			{
				text = body;
			}
			else
			{
				text = title + CSL1("\n") + body;
			}

			PilotMemo *a = new PilotMemo(text);
			PilotRecord *r = a->pack();
			r->setID(nm.memo());

			int newid = fDatabase->writeRecord(r);
			fLocalDatabase->writeRecord(r);

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
			// Add it anyway, with new PilotID.
			int newid = addNoteToPilot();
			fP->fIdList.remove(nm);
			fP->fIdList.append(NoteAndMemo(fP->fIndex.key(),newid));
		}

		++(fP->fModifiedMemosCounter);
	}

	++(fP->fIndex);
	return false;
}

bool KNotesAction::deleteNoteOnPilot()
{
	FUNCTIONSETUP;

	QValueList<NoteAndMemo>::Iterator i = fP->fIdList.begin();
	while ( i != fP->fIdList.end() )
	{
		if (fP->fNotes.contains((*i).note()))
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": Note " << (*i).note() << " still exists." << endl;
#endif
		}
		else
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": Note " << (*i).note() << " is deleted." << endl;
#endif
			fDatabase->deleteRecord((*i).memo());
			fLocalDatabase->deleteRecord((*i).memo());
			i = fP->fIdList.remove(i);
			fP->fDeletedMemosCounter++;
			continue;
		}
		++i;
	}

	return true;
}

bool KNotesAction::addNewNoteToPilot()
{
	FUNCTIONSETUP;

	if (fP->fIndex == fP->fNotes.end())
	{
		return true;
	}

	if (fP->fKNotes->isNew(CSL1("kpilot"),fP->fIndex.key()))
	{
		int newid = addNoteToPilot();
		fP->fIdList.append(NoteAndMemo(fP->fIndex.key(),newid));
		++(fP->fAddedMemosCounter);
	}

	++(fP->fIndex);
	return false;
}

bool KNotesAction::syncMemoToKNotes()
{
	FUNCTIONSETUP;

	PilotRecord *rec = 0L;

	if (SyncAction::eCopyHHToPC == getSyncDirection())
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Read record " << fP->fRecordIndex << endl;
#endif
		rec = fDatabase->readRecordByIndex(fP->fRecordIndex);
		fP->fRecordIndex++;
	}
	else
	{
		rec = fDatabase->readNextModifiedRec();
	}

	if (!rec)
	{
		return true;
	}

	PilotMemo *memo = new PilotMemo(rec);
	NoteAndMemo m = NoteAndMemo::findMemo(fP->fIdList,memo->id());

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Looking at memo "
		<< memo->id()
		<< " which was found "
		<< m.toString()
		<< endl;
#endif

	if (memo->isDeleted())
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": It's been deleted." << endl;
#endif
		if (m.valid())
		{
			// We knew about the note already, but it
			// has changed on the Pilot.
			//
			//
			if (fP->fDeleteNoteForMemo)
			{
				fP->fKNotes->killNote(m.note(),KNotesConduitSettings::suppressKNotesConfirm());
				fP->fDeletedNotesCounter++;
			}
		}
		else
		{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": It's new and deleted." << endl;
#endif
		}

		fLocalDatabase->deleteRecord(rec->id());
	}
	else
	{
		if (m.valid())
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
			// Check if KNotes still knows about this note
			if (!(fP->fKNotes->name(m.note()).isEmpty()))
			{
				updateNote(m,memo);
			}
			else
			{
				uint c = fP->fIdList.remove(m);
				if (!c)
				{
					kdWarning() << k_funcinfo
						<< ": Tried to remove valid note "
						"and failed."
						<< endl;
				}
				addMemoToKNotes(memo);
			}
		}
		else
		{
			addMemoToKNotes(memo);
		}
		fLocalDatabase->writeRecord(rec);
	}

	KPILOT_DELETE(memo);
	KPILOT_DELETE(rec);

	return false;
}

void KNotesAction::updateNote(const NoteAndMemo &m, const PilotMemo *memo)
{
	FUNCTIONSETUP;
	if (fP->fNotes[m.note()] != memo->shortTitle())
	{
		// Name changed. KNotes might complain though.
		fP->fKNotes->setName(m.note(), memo->shortTitle());
	}
	fP->fKNotes->setText(m.note(),memo->text());
	fP->fModifiedNotesCounter++;
}

void KNotesAction::addMemoToKNotes(const PilotMemo *memo)
{
	FUNCTIONSETUP;
	// This note is new to KNotes
	KNoteID_t i = fP->fKNotes->newNote(memo->shortTitle(), memo->text());
	fP->fIdList.append(NoteAndMemo(i,memo->id()));
	fP->fAddedNotesCounter++;

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": It's new with knote id " << i << endl;
#endif
}
int KNotesAction::addNoteToPilot()
{
	FUNCTIONSETUP;
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
	fLocalDatabase->writeRecord(r);

	delete r;
	delete a;

	fP->fAddedMemosCounter++;

	return newid;
}


void KNotesAction::cleanupMemos()
{
	FUNCTIONSETUP;

	// Tell KNotes we're up-to-date
	fP->fKNotes->sync(CSL1("kpilot"));

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Writing "
		<< fP->fIdList.count()
		<< " pairs to the config file."
		<< endl;
	DEBUGCONDUIT << fname
		<< ": The config file is read-only: "
		<< KNotesConduitSettings::self()->config()->isReadOnly()
		<< endl;
#endif

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

	KNotesConduitSettings::setNoteIds(notes);
	KNotesConduitSettings::setMemoIds(memos);
	KNotesConduitSettings::self()->writeConfig();

	fActionStatus=Done;
	fDatabase->cleanup();
	fDatabase->resetSyncFlags();
	fLocalDatabase->cleanup();
	fLocalDatabase->resetSyncFlags();

	// Tell the user what happened. If no changes were
	// made, spoke remains false and we'll tack a
	// message on to the end saying so, so that
	// the user always gets at least one message.
	bool spoke = false;
	if (fP->fAddedMemosCounter)
	{
		addSyncLogEntry(i18n("Added one new memo.",
			"Added %n new memos.",
			fP->fAddedMemosCounter));
	}
	if (fP->fModifiedMemosCounter)
	{
		addSyncLogEntry(i18n("Modified one memo.",
			"Modified %n memos.",
			fP->fModifiedMemosCounter));
		spoke = true;
	}
	if (fP->fDeletedMemosCounter)
	{
		addSyncLogEntry(i18n("Deleted one memo.",
			"Deleted %n memos.",fP->fDeletedMemosCounter));
		spoke = true;
	}
	if (fP->fAddedNotesCounter)
	{
		addSyncLogEntry(i18n("Added one note to KNotes.",
			"Added %n notess to KNotes.",fP->fAddedNotesCounter));
		spoke = true;
	}
	if (fP->fModifiedNotesCounter)
	{
		addSyncLogEntry(i18n("Modified one note in KNotes.",
			"Modified %n notes in KNotes.",fP->fModifiedNotesCounter));
		spoke = true;
	}
	if (fP->fDeletedNotesCounter)
	{
		addSyncLogEntry(i18n("Deleted one note from KNotes.",
			"Deleted %n note from KNotes.",fP->fDeletedNotesCounter));
		spoke = true;
	}
	if (!spoke)
	{
		addSyncLogEntry(i18n("No change to KNotes."));
	}
}


/* virtual */ QString KNotesAction::statusString() const
{
	switch(fActionStatus)
	{
	case Init : return CSL1("Init");
	case NewNotesToPilot :
		return CSL1("NewNotesToPilot key=%1")
			.arg(fP->fIndex.key());
	case ModifiedNotesToPilot :
		return CSL1("ModifiedNotesToPilot key=%1")
			.arg(fP->fIndex.key());
	case MemosToKNotes :
		return CSL1("MemosToKNotes rec=%1")
			.arg(fP->fRecordIndex);
	case Cleanup : return CSL1("Cleanup");
	case Done :
		return CSL1("Done");
	default :
		return CSL1("Unknown (%1)").arg(fActionStatus);
	}
}



