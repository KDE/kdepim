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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qmap.h>
#include <qtimer.h>

#include <kapplication.h>

#include <kurl.h>
#include <libkcal/calendarlocal.h>
#include <kstandarddirs.h>


#include <kconfig.h>
//#include <dcopclient.h>

#include <time.h>  // required by pilot-link includes

#include <pi-memo.h>

#include "pilotMemo.h"
#include "pilotSerialDatabase.h"

//#include "KNotesIface_stub.h"

#include "knotes-factory.h"

#include "knotes-action.moc"
#include "knotesconduitSettings.h"

extern "C"
{

unsigned long version_conduit_knotes = Pilot::PLUGIN_API;

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
		fNotesResource(0L),
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
		fNotesResource->save();
	
		KPILOT_DELETE(fNotesResource);
		KPILOT_DELETE(fTimer);
	}

	// The record index we're dealing with. Used by
	// CopyHHToPC sync only.
	int fRecordIndex;

	KCal::CalendarLocal *fNotesResource;
	// This is the collection of  notes held by KNotes and
	KCal::Journal::List fNotes;
	
	// This iterates through that list; it's in here because
	// we use slots to process one item at a time and need
	// to keep track of where we are between slot calls.
	KCal::Journal::List::ConstIterator fIndex;
	
	// The DCOP client for this application, and the KNotes stub.
	// DCOPClient *fDCOP;
	//KNotesIface_stub *fKNotes;

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
	bool fDeleteNoteForMemo;
};



KNotesAction::KNotesAction(KPilotLink *o,
	const char *n, const QStringList &a) :
	ConduitAction(o,n ? n : "knotes-conduit",a),
	fP(new KNotesActionPrivate)
{
	FUNCTIONSETUP;

/*
	if (fP) fP->fDCOP = KApplication::kApplication()->dcopClient();

	if (fP && !fP->fDCOP)
	{
		WARNINGKPILOT << "Can't get DCOP client." << endl;
	}
*/
}

/* virtual */ KNotesAction::~KNotesAction()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fP);
}

/* virtual */ bool KNotesAction::exec()
{
	FUNCTIONSETUP;
	DEBUGKPILOT << fname << ": Starting knotes conduit." << endl;

	if (syncMode().isTest())
	{
		test();
		delayDone();
		return true;
	}

	QString e;
	if (!openKNotesResource()) return false;

	// Database names seem to be latin1
	if (!openDatabases(CSL1("MemoDB")))
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << "Can not open databases." << endl;
#endif
		emit logError(i18n("Could not open MemoDB on the handheld."));
		return false;
	}

	fP->fTimer = new QTimer(this);
	fActionStatus = Init;
	
	// this is not needed. As it is done in the initstate in process();
	// resetIndexes();

	connect(fP->fTimer,SIGNAL(timeout()),SLOT(process()));
	fP->fTimer->start(0,false);

	return true;
}

void KNotesAction::test()
{
	if (!openKNotesResource()) return;
	listNotes();
}

bool KNotesAction::openKNotesResource()
{
	FUNCTIONSETUP;
	
	KConfig korgcfg( locate( "config", CSL1("korganizerrc") ) );
	korgcfg.setGroup( "Time & Date" );
	QString tz(korgcfg.readEntry( "TimeZoneId" ) );
	
	fP->fNotesResource = new KCal::CalendarLocal(tz);
	KURL mURL = KGlobal::dirs()->saveLocation( "data", "knotes/" ) + "notes.ics";
	
	if( fP->fNotesResource->load( mURL.path() ) )
	{
		fP->fNotes = fP->fNotesResource->journals();
		return true;
	}
	else
	{
		emit logError( i18n("Could not load the resource at: %1").arg(mURL.path()) );
		return false;
	}
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
	
	KCal::Journal::List notes = fP->fNotesResource->journals();
	DEBUGKPILOT << fname << ": the resource contains " << notes.size()
		<< " note(s)." << endl;
		
	KCal::Journal::List::ConstIterator it;
	int i = 1;
	for ( it = notes.begin(); it != notes.end(); ++it )
	{
		DEBUGKPILOT << fname << ": note " << i << " has id " << (*it)->uid() 
			<< endl;
		i++;
	}

	DEBUGKPILOT << fname << ": "
		<< "Sync direction: " << syncMode().name() << endl;
}

/* slot */ void KNotesAction::process()
{
	FUNCTIONSETUP;

	DEBUGKPILOT << fname << ": Now in state " << fActionStatus << endl;

	switch(fActionStatus)
	{
	case Init:
		resetIndexes();
		getAppInfo();
		getConfigInfo();
		switch(syncMode().mode())
		{
		case SyncAction::SyncMode::eBackup:
		case SyncAction::SyncMode::eRestore:
			// Impossible!
			fActionStatus = Done;
			break;
		case SyncAction::SyncMode::eCopyHHToPC :
			listNotes(); // Debugging
			fActionStatus = MemosToKNotes;
			break;
		case SyncAction::SyncMode::eHotSync:
		case SyncAction::SyncMode::eFullSync:
		case SyncAction::SyncMode::eCopyPCToHH:
			fActionStatus = ModifiedNotesToPilot;
			break;
		}
		break;
	case ModifiedNotesToPilot:
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
			switch(syncMode().mode())
			{
			case SyncAction::SyncMode::eBackup:
			case SyncAction::SyncMode::eRestore:
			case SyncAction::SyncMode::eCopyHHToPC :
				// Impossible!
				fActionStatus = Done;
				break;
			case SyncAction::SyncMode::eHotSync:
			case SyncAction::SyncMode::eFullSync:
				fActionStatus = MemosToKNotes;
				break;
			case SyncAction::SyncMode::eCopyPCToHH:
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
		WARNINGKPILOT
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

	resetIndexes();
}


bool KNotesAction::modifyNoteOnPilot()
{
	FUNCTIONSETUP;
	return true;
	/*
	if (fP->fIndex == fP->fNotes.end())
	{
		return true;
	}
	*/

	//TODO DCOP_REMOVAL
	/*
	if (fP->fKNotes->isModified(CSL1("kpilot"),fP->fIndex.key()))
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
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
				WARNINGKPILOT
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
			WARNINGKPILOT << "Modified note unknown to Pilot" << endl;
			// Add it anyway, with new PilotID.
			int newid = addNoteToPilot();
			fP->fIdList.remove(nm);
			fP->fIdList.append(NoteAndMemo(fP->fIndex.key(),newid));
		}

		++(fP->fModifiedMemosCounter);
	}
	*/

	//++(fP->fIndex);
	//return false;
}

bool KNotesAction::deleteNoteOnPilot()
{
	FUNCTIONSETUP;

	/*
	QValueList<NoteAndMemo>::Iterator i = fP->fIdList.begin();
	while ( i != fP->fIdList.end() )
	{
		// TODO DCOP_REMOVE
		if (fP->fNotes.contains((*i).note()))
		{
#ifdef DEBUG
			DEBUGKPILOT << fname << ": Note " << (*i).note() << " still exists." << endl;
#endif
		}
		else
		{
#ifdef DEBUG
			DEBUGKPILOT << fname << ": Note " << (*i).note() << " is deleted." << endl;
#endif
			fDatabase->deleteRecord((*i).memo());
			fLocalDatabase->deleteRecord((*i).memo());
			i = fP->fIdList.remove(i);
			fP->fDeletedMemosCounter++;
			continue;
		}
		++i;
	}
	*/
	return true;
}

bool KNotesAction::addNewNoteToPilot()
{
	FUNCTIONSETUP;

	if (fP->fIndex == fP->fNotes.end())
	{
		return true;
	}

	KCal::Journal *j = (*fP->fIndex);

	if( j->pilotId() == 0 )
	{
		DEBUGKPILOT << fname << ": Adding note with id " << j->uid() 
			<< " to pilot." << endl;
		
		int newid = addNoteToPilot();
		
		++(fP->fAddedMemosCounter);
	}
	//TODO DCOP_REMOVAL
	/*
	if (fP->fKNotes->isNew(CSL1("kpilot"),fP->fIndex.key()))
	{
		int newid = addNoteToPilot();
		fP->fIdList.append(NoteAndMemo(fP->fIndex.key(),newid));
		++(fP->fAddedMemosCounter);
	}
	*/

	++(fP->fIndex);
	return false;
}

bool KNotesAction::syncMemoToKNotes()
{
	FUNCTIONSETUP;

	PilotRecord *rec = 0L;

	if ( syncMode() == SyncAction::SyncMode::eCopyHHToPC )
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Read record " << fP->fRecordIndex << endl;
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
	DEBUGKPILOT << fname << ": Looking at memo "
		<< memo->id()
		<< " which was found "
		<< m.toString()
		<< endl;
#endif

	if (memo->isDeleted())
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": It's been deleted." << endl;
#endif
		if (m.valid())
		{
			// We knew about the note already, but it
			// has changed on the Pilot.
			//
			//
			if (fP->fDeleteNoteForMemo)
			{
		//TODO DCOP_REMOVAL
	//fP->fKNotes->killNote(m.note(),KNotesConduitSettings::suppressKNotesConfirm()
	//) ;
				fP->fDeletedNotesCounter++;
			}
		}
		else
		{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": It's new and deleted." << endl;
#endif
		}

		fLocalDatabase->deleteRecord(rec->id());
	}
	else
	{
		if (m.valid())
		{
	#ifdef DEBUG
			DEBUGKPILOT << fname << ": It's just modified." << endl;
			DEBUGKPILOT << fname << ": <"
//				<< fP->fNotes[m.note()]
				<< "> <"
				<< memo->shortTitle()
				<< ">"
				<< endl;
	#endif
			// Check if KNotes still knows about this note
			//TODO DCOP_REMOVAL
			/*
			if (!(fP->fKNotes->name(m.note()).isEmpty()))
			{
				updateNote(m,memo);
			}
			else
			{
				uint c = fP->fIdList.remove(m);
				if (!c)
				{
					WARNINGKPILOT
						<< "Tried to remove valid note and failed."
						<< endl;
				}
				addMemoToKNotes(memo);
			}
			*/
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
	//TODO DCOP_REMOVAL
	if (true/*fP->fNotes[m.note()] != memo->shortTitle()*/)
	{
		// Name changed. KNotes might complain though.
		//TODO DCOP_REMOVAL
		//fP->fKNotes->setName(m.note(), memo->shortTitle());
	}
	//TODO DCOP_REMOVAL
	//fP->fKNotes->setText(m.note(),memo->text());
	fP->fModifiedNotesCounter++;
}

void KNotesAction::addMemoToKNotes(const PilotMemo *memo)
{
	FUNCTIONSETUP;
	// This note is new to KNotes
	//TODO DCOP_REMOVAL
	//KNoteID_t i = fP->fKNotes->newNote(memo->shortTitle(), memo->text());
	//fP->fIdList.append(NoteAndMemo(i,memo->id()));
	//fP->fAddedNotesCounter++;

#ifdef DEBUG
	//TODO DCOP_REMOVAL
	//DEBUGKPILOT << fname << ": It's new with knote id " << i << endl;
#endif
}
int KNotesAction::addNoteToPilot()
{
	FUNCTIONSETUP;
	
	KCal::Journal *j = (*fP->fIndex);
	
#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": The note #"
		<< j->uid()
		<< " with name "
		<< j->summary()
		<< " is new to the Pilot."
		<< endl;
#endif

	QString text = j->summary() + CSL1("\n");
	text.append( j->description() );
	//TODO DCOP_REMOVAL
	//text.append(fP->fKNotes->text(fP->fIndex.key()));

	PilotMemo *a = new PilotMemo(text);
	PilotRecord *r = a->pack();

	int newid = fDatabase->writeRecord(r);
	fLocalDatabase->writeRecord(r);
	
	j->setPilotId( newid );

	delete r;
	delete a;
	delete j;

	fP->fAddedMemosCounter++;

	return newid;
}


void KNotesAction::cleanupMemos()
{
	FUNCTIONSETUP;

	// Tell KNotes we're up-to-date
	//TODO DCOP_REMOVAL
	//fP->fKNotes->sync(CSL1("kpilot"));

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Writing "
		<< fP->fIdList.count()
		<< " pairs to the config file."
		<< endl;
	DEBUGKPILOT << fname
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
			"Added %n notes to KNotes.",fP->fAddedNotesCounter));
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
			"Deleted %n notes from KNotes.",fP->fDeletedNotesCounter));
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
		return CSL1("NewNotesToPilot key=%1");
			// TODO DCOP_REMOVAL .arg(fP->fIndex.key());
	case ModifiedNotesToPilot :
		return CSL1("ModifiedNotesToPilot key=%1");
			//TODO DCOP_REMOVAL .arg(fP->fIndex.key());
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



