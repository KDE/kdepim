// knotes-conduit.h
//
// Copyright (C) 2000 Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$


#ifndef _KNOTES_CONDUIT_H
#define _KNOTES_CONDUIT_H

#include "baseConduit.h"

class PilotRecord;
class PilotMemo;

#include <qstring.h>
#include <qmap.h>


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
		unsigned long pilotID) :
		cP(configPath),dP(dataPath),id(pilotID) {} ;

	const QString& configPath() const { return cP; } ;
	const QString& dataPath() const { return dP; } ;
	unsigned long pilotID() const { return id; } ;

	bool isNew() const { return id == 0L ; } ;

private:
	QString cP,dP;
	unsigned long id;
} ;

typedef QMap<QString,NotesSettings> NotesMap;

class KNotesConduit : public BaseConduit
{
public:
  KNotesConduit(eConduitMode mode);
  virtual ~KNotesConduit();
  
  virtual void doSync();
  virtual QWidget* aboutAndSetup();

  virtual const char* dbInfo() ; // { return NULL; }
  virtual void doTest();

protected:
	int notesToPilot(NotesMap&);
	int pilotToNotes(NotesMap&);

	bool changeMemo(NotesMap&,NotesMap::Iterator,PilotMemo *);
	bool newMemo(NotesMap&,unsigned long id,PilotMemo *);
	/**
	* Delete a KNote associated with a particular
	* Pilot memo. The note to be deleted is the
	* one the iterator points to. The note is also
	* removed from the map.
	*/
	bool deleteNote(NotesMap&,NotesMap::Iterator *,unsigned long);

private:
	/**
	* Read the global KPilot config file for settings
	* particular to the KNotes conduit.
	* @ref fDeleteNoteForMemo
	*/
	void readConfig();

	/**
	* This reflects the setting in the KNotes conduit
	* setup whether or not to delete the KNote associated
	* with a particular Pilot memo when the memo itself
	* is deleted.
	*/
	bool fDeleteNoteForMemo;
};

// $Log$
// Revision 1.3  2000/11/26 01:43:21  adridg
// Two-way syncs
//
// Revision 1.2  2000/11/24 17:54:28  adridg
// Two-way sync
//
// Revision 1.1  2000/11/20 00:22:28  adridg
// New KNotes conduit
//
#endif
