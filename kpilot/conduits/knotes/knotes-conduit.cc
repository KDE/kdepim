// knotes-conduit.cc
//
// Copyright (C) 2000 by Dan Pilone, Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
//
//
// The KNotes conduit copies memos from the Pilot's memo pad to KNotes
// and vice-versa. It complements or replaces the builtin memo conduit
// in KPilot.
//
//



#include "options.h"

// Only include what we really need:
// First UNIX system stuff, then std C++, 
// then Qt, then KDE, then local includes.
//
//
#include <stream.h>
#include <qdir.h>
#include <qmap.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kconfig.h>
#include <kdebug.h>

#include "conduitApp.h"
#include "kpilotlink.h"
#include "knotes-conduit.h"
#include "setupDialog.h"
#include "pilotMemo.h"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static const char *id=
	"$Id$";


// This is a generic main() function, all
// conduits look basically the same,
// except for the name of the conduit.
//
//
int main(int argc, char* argv[])
{
	ConduitApp a(argc,argv,"knotes",
		I18N_NOOP("KNotes Conduit"),
		"0.1");

	a.addAuthor("Adriaan de Groot",
		"KNotes Conduit author",
		"adridg@sci.kun.nl");

	KNotesConduit conduit(a.getMode());
	a.setConduit(&conduit);
	return a.exec();
}



static NotesMap
collectNotes()
{
	FUNCTIONSETUP;
	NotesMap m;

	// This is code taken directly from KNotes
	//
	//
	QString str_notedir = KGlobal::dirs()->
		saveLocation( "appdata", "notes/" );

#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname << ": Notes dir = " << str_notedir << endl;
	}
#endif

	QDir notedir( str_notedir );
	QStringList notes = notedir.entryList( QDir::Files, QDir::Name );
	QStringList::ConstIterator  i;

	for (i=notes.begin() ; i !=notes.end(); ++i)
	{
		QString notedata ;
		KSimpleConfig *c = 0L;
		int version ;
		unsigned long pilotID ;

#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname << ": Reading note " << *i << endl;
		}
#endif
		c = new KSimpleConfig( notedir.absFilePath(*i));
		
		c->setGroup("General");
		version = c->readNumEntry("version",1);
		c->setGroup("KPilot");
		pilotID = c->readNumEntry("pilotID",0);

		if (version<2) goto EndNote;
#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
		kdDebug() << fname << ": Note has version " << version << endl;
		kdDebug() << fname << ": Note has pilotID " << pilotID << endl;
		}
#endif

		c->setGroup("Data");
		notedata = "." + c->readEntry("name") + "_data";
		if (notedir.exists(notedata))
		{
#ifdef DEBUG
			if (debug_level & SYNC_TEDIOUS)
			{
			kdDebug() << fname << ": Data for note in " 
				<< notedata << endl;
			}
#endif
			NotesSettings n(notedir.absFilePath(*i),
				notedir.absFilePath(notedata),
				pilotID);
			m.insert(*i,n);
		}
		
EndNote:
		delete c;
	}

	return m;
}


static NotesMap::Iterator *
findID(NotesMap& m,unsigned long id)
{
	FUNCTIONSETUP;

	NotesMap::Iterator *i = new NotesMap::Iterator;

	for ((*i)=m.begin(); (*i)!=m.end(); ++(*i))
	{
		const NotesSettings& r = (*(*i));

		if (r.pilotID()==id)
		{
#ifdef DEBUG
			if (debug_level & SYNC_TEDIOUS)
			{
				kdDebug() << fname
					<< ": Found ID "
					<< id
					<< " in note "
					<< r.configPath()
					<< endl;
			}
#endif
			return i;
		}
	}

	delete i;

#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname
			<< ": ID "
			<< id
			<< " not found."
			<< endl;
	}
#endif
		
	return 0L;
}



KNotesConduit::KNotesConduit(eConduitMode mode) : 
	BaseConduit(mode),
	fDeleteNoteForMemo(false)
{
	FUNCTIONSETUP;

}

KNotesConduit::~KNotesConduit()
{
	FUNCTIONSETUP;

}

void
KNotesConduit::readConfig()
{
	FUNCTIONSETUP;

	KConfig& c = KPilotLink::getConfig(KNotesOptions::KNotesGroup);
	getDebugLevel(c);
	fDeleteNoteForMemo = c.readBoolEntry("DeleteNoteForMemo",false);
#ifdef DEBUG
	if (debug_level & SYNC_MINOR)
	{
		kdDebug() << fname
			<< ": Settings "
			<< "DeleteNoteForMemo="
			<< fDeleteNoteForMemo
			<< endl;
	}
#endif
}

// doSync should add a line to the logfile
// with the indicated text, but the 
// addSyncLogEntry() doesn't work (not
// in any other conduit, either).
//
// Just print the message to error.
//
//
void
KNotesConduit::doSync()
{
	FUNCTIONSETUP;

	readConfig();

	NotesMap m = collectNotes();

	// First add all the new KNotes to the Pilot
	//
	//
	int newCount = notesToPilot(m);
	int oldCount = pilotToNotes(m);

	if (newCount || oldCount)
	{
		QString msg = i18n("Changed %1/%2 Memos/Notes")
			.arg(newCount)
			.arg(oldCount);
		addSyncLogMessage(msg.local8Bit());
#ifdef DEBUG
		if (debug_level & SYNC_MAJOR)
		{
			kdDebug() << fname
				<< ": "
				<< msg
				<< endl;
		}
#endif
	}

}

int KNotesConduit::notesToPilot(NotesMap& m)
{
	FUNCTIONSETUP;
	NotesMap::ConstIterator i;
	PilotRecord *rec;
	PilotMemo *memo;
	int count=0;

#ifdef DEBUG
	if (debug_level & SYNC_MAJOR)
	{
		kdDebug() << fname
			<< ": Adding new memos to pilot"
			<< endl;
	}
#endif


	for (i=m.begin(); i!=m.end(); ++i)
	{
		const NotesSettings& s=(*i);
		unsigned long id;

		if (!s.isNew()) continue;

		// We know that this is a new memo (no pilot id)
		// Check that the data can actually fit into a memo.
		// If it does, read it all.
		//
		QFile f(s.dataPath());

		if (f.size()>PilotMemo::MAX_MEMO_LEN) continue;
		if (!f.open(IO_ReadOnly)) continue;

		char *text = new char[f.size()+16];
		int len = f.readBlock(text,f.size());
		text[len]=0;

#ifdef DEBUG
		if (debug_level & SYNC_MINOR)
		{
			kdDebug() << fname
				<< ": Read "
				<< len
				<< " bytes from note "
				<< s.dataPath()
				<< endl;
		}
#endif
		memo = new PilotMemo(text,0,0,0);
		rec = memo->pack();
		id = writeRecord(rec);

		KSimpleConfig *c = new KSimpleConfig(s.configPath());
		c->setGroup("KPilot");
		c->writeEntry("pilotID",id);

		delete c;
		delete rec;
		delete memo;

		count++;
	}

	return count;
}

bool KNotesConduit::newMemo(NotesMap& m,unsigned long id,PilotMemo *memo)
{
	FUNCTIONSETUP;

	QString noteName = memo->sensibleTitle();

	// This is code taken directly from KNotes
	//
	//
	QString str_notedir = KGlobal::dirs()->
		saveLocation( "appdata", "notes/" );

#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname << ": Notes dir = " << str_notedir << endl;
	}
#endif

	QDir notedir( str_notedir );

	if (notedir.exists(noteName))
	{
		noteName += QString::number(id);
	}

	if (notedir.exists(noteName))
	{
		kdDebug() << fname
			<< ": Note " << noteName
			<< " already exists!"
			<< endl;
		return false;
	}

	QString dataName = "." + noteName + "_data" ;
	bool success = false;
#ifdef DEBUG
	if (debug_level & SYNC_MAJOR)
	{
		kdDebug() << fname
			<< ": Creating note "
			<< noteName
			<< endl;
	}
#endif

	{
	QFile file(notedir.absFilePath(dataName));
	file.open(IO_WriteOnly | IO_Truncate);
	if (file.isOpen())
	{
		file.writeBlock(memo->text(),strlen(memo->text()));
		success = true;
	}
	file.close();
	}

	// Write out the KNotes config file 
	if (success)
	{
	KConfig *c = new KSimpleConfig( notedir.absFilePath(noteName));
	
	c->setGroup("General");
	c->writeEntry("version",2);
	c->setGroup("KPilot");
	c->writeEntry("pilotID",id);
	c->setGroup("Data");
	c->writeEntry("name",noteName);
	c->sync();

	delete c;
	}

	if (success)
	{
		NotesSettings n(notedir.absFilePath(noteName),
			notedir.absFilePath(dataName),
			id);
		m.insert(noteName,n);
	}

	return success;
}

bool KNotesConduit::changeMemo(NotesMap& m,NotesMap::Iterator i,PilotMemo *memo)
{
	FUNCTIONSETUP;


// handle deleted memos (deleted on the pilot) here.
// since they're just copies of what's in knotes,
// delete the files.

	(void) m;
	NotesSettings n = *i;
#ifdef DEBUG
	if (debug_level & SYNC_MAJOR)
	{
		kdDebug() << fname
			<< ": Updating note "
			<< n.configPath()
			<< endl;
	}
#endif
	QFile file(n.dataPath());
	file.open(IO_WriteOnly | IO_Truncate);
	if (file.isOpen())
	{
		file.writeBlock(memo->text(),strlen(memo->text()));
#ifdef DEBUG
		if (debug_level & SYNC_MINOR)
		{
			kdDebug() << fname
				<< ": Succesfully updated memo "
				<< n.configPath()
				<< endl;
		}
#endif

		return true;
	}
	else
	{
		return false;
	}
}

bool KNotesConduit::deleteNote(NotesMap& m,NotesMap::Iterator *i,
	unsigned long id)
{
	FUNCTIONSETUP;

	if (!i)
	{
#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname
				<< ": Unknown Pilot memo "
				<< id
				<< " has been deleted."
				<< endl;
		}
#endif
		return false;
	}

#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
		if (fDeleteNoteForMemo)
		{
			kdDebug() << fname
				<< ": Deleting pilot memo "
				<< id
				<< endl;
		}
		else
		{
			kdDebug() << fname
				<< ": Pilot memo "
				<< id
				<< " has been deleted, note remains."
				<< endl;
			return false;
		}
	}
#endif

	NotesSettings n = *(*i);

#ifdef DEBUG
	if (debug_level & SYNC_MAJOR)
	{
		kdDebug() << fname
			<< ": Deleting note "
			<< n.configPath()
			<< endl;
	}
#endif

	QFile::remove(n.dataPath());
	QFile::remove(n.configPath());

	m.remove(*i);

	return true;
}


int KNotesConduit::pilotToNotes(NotesMap& m)
{
	FUNCTIONSETUP;

	PilotRecord *rec;
	int count=0;
	NotesMap::Iterator *i;

	while ((rec=readNextModifiedRecord()))
	{
		unsigned long id = rec->getID();
		bool success;

		success=false;

		kdDebug() << fname 
			<< ": Read Pilot record with ID "
			<< id
			<< endl;

		i = findID(m,id);
		if (rec->getAttrib() & dlpRecAttrDeleted)
		{
			success=deleteNote(m,i,id);
		}
		else
		{
			PilotMemo *memo = new PilotMemo(rec);
			if (i)
			{
				success=changeMemo(m,*i,memo);
			}
			else
			{
				success=newMemo(m,id,memo);
			}
			delete memo;
		}
		delete rec;

		if (success) 
		{ 
			count++;
		}
	}

// we also need to tell knotes that things have changed in the dirs it owns

	return count;
}

// aboutAndSetup is pretty much the same
// on all conduits as well.
//
//
QWidget*
KNotesConduit::aboutAndSetup()
{
	FUNCTIONSETUP;

	return new KNotesOptions(0L);
}

const char *
KNotesConduit::dbInfo()
{
	return "MemoDB";
}



/* virtual */ void
KNotesConduit::doTest()
{
	FUNCTIONSETUP;

	NotesMap m = collectNotes();


	NotesMap::Iterator i;
	for (i=m.begin(); i!=m.end(); ++i)
	{
		kdDebug() << fname 
			<< ": Note "
			<< (*i).configPath()
			<< " "
			<< (*i).dataPath()
			<< " "
			<< (*i).pilotID()
			<< endl;
	}

	(void) id;
}

// $Log$
// Revision 1.1  2000/11/20 00:22:28  adridg
// New KNotes conduit
//
