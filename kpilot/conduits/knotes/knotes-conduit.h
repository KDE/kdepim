/* knotes-conduit.h			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This file is part of the KNotes conduit, a conduit for KPilot that
** synchronises the Pilot's memo pad application with KNotes.
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

#ifndef _KPILOT_KNOTES_CONDUIT_H
#define _KPILOT_KNOTES_CONDUIT_H

#ifndef _KPILOT_BASECONDUIT_H
#include "baseConduit.h"
#endif


class PilotRecord;
class PilotMemo;

#ifndef QSTRING_H
#include <qstring.h>
#endif

#ifndef QMAP_H
#include <qmap.h>
#endif



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
	// Both of these constructores are deprecated.
	//
	//
	NotesSettings() {} ;
	NotesSettings(const QString& name,
		const QString& configPath,
		const QString& dataPath,
		unsigned long pilotID) :
		nN(name),
		cP(configPath),dP(dataPath),id(pilotID),
		checksum(0L),csValid(false) {} ;

	NotesSettings(const QString &configPath,
	        const QString &notesdir,
		KConfig& c);

	const QString& name() const { return nN; } ;
	const QString& configPath() const { return cP; } ;
	const QString& dataPath() const { return dP; } ;
	unsigned long pilotID() const { return id; } ;

	bool isNew() const { return id == 0L ; } ;
	void setId(unsigned long i) { id=i; } ;

	QString checkSum() const { return checksum; } ;
	bool isCheckSumValid() const { return csValid; } ;
	/**
	* Is the file changed? If we have no valid checksum,
	* then it must be changed. Otherwise, recompute the
	* checksum and if the new checksum is different from
	* the old one, then the note has changed.
	*/
	bool isChanged() const 
	{ 
		QString newCS = computeCheckSum();
		return !csValid || (newCS != checksum) ; 
	}

	QString updateCheckSum() 
	{ 
		checksum=computeCheckSum(); 
		csValid = !checksum.isNull();
		return checksum; 
	} ;
	QString computeCheckSum() const;

	/**
	* Reads a note file into memory. This reads at most
	* PilotMemo::MAX_MEMO_LEN bytes into the buffer text,
	* which must be at least PilotMemo::MAX_MEMO_LEN+1
	* bytes large (1 extra byte to accommodate trailing 0).
	*/
	int readNotesData(char *text);

private:
	QString nN,cP,dP;
	unsigned long id;
	/**
	* Since KNotes (re)writes the notes files all the time,
	* we need a different way of detecting when a note has
	* changed. We don't want to keep copying all the notes
	* to the Pilot if they haven't really changed, so we
	* add a checksum when the note is copied to the Pilot.
	* The checksum is just the (16-byte) md5 digest of the note.
	*/
	QString checksum;
	bool csValid;		// Checksum valid?
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

	/**
	* KNotes can be new or changed relative to the Pilot.
	* Each requires special handling.
	*/
	bool addNewNote(NotesSettings&);
	bool changeNote(NotesSettings&);

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

#endif
