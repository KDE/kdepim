// knotes-conduit.cc
//
// Copyright (C) 2000 by Dan Pilone, Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// This is the version of null-conduit.cc for KDE 2 / KPilot 4


// The NULL conduit is a conduit that does nothing.
// It's just a programming example, and maybe sometime
// we can attach it to databases as a means of *not*
// doing anything with those databases.
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
		"NULL Conduit author",
		"adridg@sci.kun.nl");

	KNotesConduit conduit(a.getMode());
	a.setConduit(&conduit);
	return a.exec();
}


// This class stores information about notes.
// We construct a map that associates note names
// with NotesSettings so that we can quickly 
// find out which notes are new / changed / old
// or deleted without having to re-read all those
// KNotes config files every time.
//
//
class NotesSettings
{
public:
	NotesSettings() {} ;
	NotesSettings(QString configPath,
		QString dataPath,
		int pilotID) :
		cP(configPath),dP(dataPath),id(pilotID) {} ;

	const QString& configPath() const { return cP; } ;
	const QString& dataPath() const { return dP; } ;
	int pilotID() const { return id; } ;

	bool isNew() const { return id == 0 ; } ;

private:
	QString cP,dP;
	int id;
} ;

typedef QMap<QString,NotesSettings> NotesMap;

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
		int pilotID ;

#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << ": Reading note " << *i << endl;
		}
#endif
		c = new KSimpleConfig( notedir.absFilePath(*i));
		
		c->setGroup("General");
		version = c->readNumEntry("version",1);
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











// A conduit that does nothing has a very
// simple constructor and destructor.
//
//
KNotesConduit::KNotesConduit(eConduitMode mode)
	: BaseConduit(mode)
{
	FUNCTIONSETUP;

}

KNotesConduit::~KNotesConduit()
{
	FUNCTIONSETUP;

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

	KConfig& c = KPilotLink::getConfig("knotesOptions");
	getDebugLevel(c);

	NotesMap m = collectNotes();
	NotesMap::ConstIterator i;
	PilotRecord *rec;
	PilotMemo *memo;
	int newCount=0;

	// First add all the new KNotes to the Pilot
	//
	//
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
		int id;

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
		c->setGroup("General");
		c->writeEntry("pilotID",id);

		delete c;
		delete rec;
		delete memo;
	}

	if (newCount)
	{
		QString msg = i18n("Added %1 new memos.");
		msg.arg(newCount);

		addSyncLogMessage(msg.local8Bit());
#ifdef DEBUG
		if (debug_level & SYNC_MAJOR)
		{
			kdDebug() << fname
				<< msg
				<< endl;
		}
#endif
	}
}

// aboutAndSetup is pretty much the same
// on all conduits as well.
//
//
QWidget*
KNotesConduit::aboutAndSetup()
{
	FUNCTIONSETUP;

	cerr << fname << ": Running!" << endl;

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
