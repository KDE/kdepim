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
#include <dcopclient.h>
#include <kdebug.h>

#include "conduitApp.h"
#include "kpilotlink.h"
#include "knotes-conduit.h"
#include "setupDialog.h"
#include "pilotMemo.h"


#include	"md5wrap.h"

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
	return a.exec(true /* with DCOP support */);
}


NotesSettings::NotesSettings(const QString &configPath,
	const QString &notesdir,
	KConfig& c) :
	cP(configPath)
{
	EFUNCTIONSETUP;

	DEBUGCONDUIT << fname
		<< ": Reading note from "
		<< configPath
		<< " in dir "
		<< notesdir
		<< endl;

	c.setGroup("KPilot");
	id = c.readNumEntry("pilotID",0);
	checksum = c.readEntry("checksum",QString::null);
	csValid = !(checksum.isNull());

	c.setGroup("Data");
	nN = c.readEntry("name");

	QString notedata = notesdir + "/." +  nN + "_data";
	if (QFile::exists(notedata))
	{
		DEBUGCONDUIT << fname 
			<< ": Data for note in " 
			<< notedata 
			<< endl;
		dP = notedata;
	}
	else
	{
		kdWarning() << fname
			<< ": No data file for note?"
			<< endl;
	}

}

QString NotesSettings::computeCheckSum() const
{
	EFUNCTIONSETUP;

	if (dP.isEmpty()) return 0;

	QFile f(dP);
	if (!f.open(IO_ReadOnly))
	{
		kdWarning() << fname
			<< ": Couldn't open file."
			<< endl;
		return QString::null;
	}

	unsigned char data[PilotMemo::MAX_MEMO_LEN];
	int r = f.readBlock((char *)data,(int) PilotMemo::MAX_MEMO_LEN);
	if (r<1)
	{
		kdWarning() << fname
			<< ": Couldn't read notes file."
			<< endl;
		return QString::null;
	}

	QString s;
	MD5Context md5context;
	md5context.update(data,r);
	s=md5context.finalize();

	return s;
}

int NotesSettings::readNotesData(char *text)
{
	EFUNCTIONSETUP;

	// Check that the data can actually fit into a memo.
	// If it does, read it all.
	//
	QFile f(dataPath());
	int filesize = f.size();

	if (filesize > PilotMemo::MAX_MEMO_LEN) 
	{
		kdWarning() << fname
			<< ": Notes file is too large ("
			<< filesize
			<< " bytes) -- truncated to "
			<< (int) PilotMemo::MAX_MEMO_LEN
			<< endl;
		filesize = PilotMemo::MAX_MEMO_LEN;
	}
		
	if (!f.open(IO_ReadOnly)) 
	{
		kdWarning() << fname
			<< ": Couldn't read notes file."
			<< endl;
		return 0;
	}

	memset(text,0,PilotMemo::MAX_MEMO_LEN+1);
	int len = f.readBlock(text,filesize);

	DEBUGCONDUIT << fname
		<< ": Read "
		<< len
		<< " bytes from note "
		<< dataPath()
		<< endl;

	return len;
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
		QString notename,notedata ;
		KSimpleConfig *c = 0L;
		int version ;

#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname << ": Reading note " << *i << endl;
		}
#endif
		c = new KSimpleConfig( notedir.absFilePath(*i));
		
		c->setGroup("General");
		version = c->readNumEntry("version",1);
		if (version<2)
		{
			kdWarning() << fname
				<< ": Skipping old-style KNote"
				<< *i
				<< endl;
			goto EndNote;
		}

		NotesSettings n(notedir.absFilePath(*i),
			notedir.absPath(),
			*c);
		m.insert(*i,n);
		
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
	EFUNCTIONSETUP;

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

		DEBUGCONDUIT << fname
			<< ": "
			<< msg
			<< endl;
	}

	DCOPClient *dcopptr = KApplication::kApplication()->dcopClient();
	if (!dcopptr)
	{
		kdWarning() << fname
			<< ": Can't get DCOP client."
			<< endl;
		return;
	}

	QByteArray data;
	if (dcopptr -> send("knotes",
		"KNotesIface",
		"rereadNotesDir()",
		data))
	{
		kdWarning() << fname
			<< ": Couldn't tell KNotes to re-read notes."
			<< endl;
	}
}


bool KNotesConduit::addNewNote(NotesSettings& s)
{
	FUNCTIONSETUP;

	// We know that this is a new memo (no pilot id)
	char text[PilotMemo::MAX_MEMO_LEN+1];
	if (!s.readNotesData(text))
	{
		return false;
	}

	PilotMemo *memo = new PilotMemo(text,0,0,0);
	PilotRecord *rec = memo->pack();
	unsigned long id = writeRecord(rec);

	KSimpleConfig *c = new KSimpleConfig(s.configPath());
	c->setGroup("KPilot");
	c->writeEntry("pilotID",id);
	c->writeEntry("checksum",s.computeCheckSum());

	s.setId(id);

	delete c;
	delete rec;
	delete memo;

	return true;
}

bool KNotesConduit::changeNote(NotesSettings& s)
{
	FUNCTIONSETUP;

	char text[PilotMemo::MAX_MEMO_LEN+1];
	if (!s.readNotesData(text))
	{
		return false;
	}

	return false;
}


int KNotesConduit::notesToPilot(NotesMap& m)
{
	FUNCTIONSETUP;
	NotesMap::Iterator i;
	int count=0;

	DEBUGCONDUIT << fname
		<< ": Adding new memos to pilot"
		<< endl;


	for (i=m.begin(); i!=m.end(); ++i)
	{
		NotesSettings& s=(*i);

		if (s.isNew()) 
		{
			addNewNote(s);
			count++;
		}
		if (s.isChanged()) 
		{
			changeNote(s);
			count++;
		}
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
		NotesSettings n(noteName,
			notedir.absFilePath(noteName),
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
	DCOPClient *dcopptr = KApplication::kApplication()->dcopClient();
	if (!dcopptr)
	{
		kdWarning() << fname
			<< ": Can't get DCOP client."
			<< endl;
		return count;
	}

	QByteArray data;
	if (dcopptr -> send("knotes",
		"KNotesIface",
		"rereadNotesDir()",
		data))
	{
		DEBUGCONDUIT << fname
			<< ": DCOP to KNotes succesful."
			<< endl;
	}
	else
	{
		kdWarning() << fname
			<< ": Couln't tell KNotes about new notes."
			<< endl;
	}

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
	EFUNCTIONSETUP;
	DCOPClient *dcopptr = KApplication::kApplication()->dcopClient();
	if (!dcopptr)
	{
		kdWarning() << fname
			<< ": Can't get DCOP client."
			<< endl;
		return;
	}

	NotesMap m = collectNotes();


	NotesMap::Iterator i;
	for (i=m.begin(); i!=m.end(); ++i)
	{
		if ((*i).pilotID())
		{
			DEBUGCONDUIT << fname
				<< ": Showing note " 
				<< (*i).name()
				<< endl;

			QByteArray data;
			QDataStream arg(data,IO_WriteOnly);
			arg << (*i).name() ;
			if (dcopptr -> send("knotes",
				"KNotesIface",
				"showNote(QString)",
				data))
			{
				DEBUGCONDUIT << fname
					<< ": DCOP send succesful"
					<< endl;
			}
			else
			{
				kdWarning() << fname
					<< ": DCOP send failed."
					<< endl;
			}
		}
		else
		{
			DEBUGCONDUIT << fname
				<< ": Note "
				<< (*i).name()
				<< " not in Pilot"
				<< endl;
		}
		DEBUGCONDUIT << fname
			<< ": Checksum = "
			<< (*i).computeCheckSum()
			<< endl;
	}

	(void) id;
}

// $Log$
// Revision 1.6  2000/12/29 14:17:51  adridg
// Added checksumming to KNotes conduit
//
// Revision 1.5  2000/12/22 07:47:04  adridg
// Added DCOP support to conduitApp. Breaks binary compatibility.
//
// Revision 1.4  2000/12/05 07:44:01  adridg
// Cleanup
//
// Revision 1.1  2000/11/20 00:22:28  adridg
// New KNotes conduit
//
