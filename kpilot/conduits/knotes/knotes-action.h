#ifndef _KPILOT_KNOTES_ACTION_H
#define _KPILOT_KNOTES_ACTION_H
/* knotes-action.h                      KPilot
**
** Copyright (C) 2001,2003 by Dan Pilone
**
** This file defines the SyncAction that the KNotes conduit performs.
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

#include "plugin.h"


class NoteAndMemo;
class PilotMemo;

class KNotesAction : public ConduitAction
{
Q_OBJECT
public:
	KNotesAction(
		KPilotDeviceLink *o,
		const char *n = 0L,
		const QStringList &a = QStringList() );
	virtual ~KNotesAction();

	enum Status { Init,
		ModifiedNotesToPilot,
		DeleteNotesOnPilot,
		NewNotesToPilot,
		MemosToKNotes,
		Cleanup,
		Done } ;
	virtual QString statusString() const;

protected:
	virtual bool exec();

protected:
	/**
	* For test mode -- just list the notes KNotes has.
	*/
	void listNotes();

	/**
	* For actual processing. These are called by process
	* and it is critical that fP->fIndex is set properly.
	*
	* Each returns true when it is completely finished processing,
	* if it returns a bool. Void functions need only be called once.
	*/
	void getAppInfo();
	void getConfigInfo();
	bool modifyNoteOnPilot();
	bool deleteNoteOnPilot();
	bool addNewNoteToPilot();
	bool syncMemoToKNotes();
	void cleanupMemos();

	void updateNote(const NoteAndMemo &,const PilotMemo *);
	
	/**
	* Add the Memo to KNotes.
	*/
	void addMemoToKNotes(const PilotMemo *);
	/**
	* Add the Note currently being processed to the
	* pilot as a new memo. Returns the id of the record.
	*/
	int addNoteToPilot();


	void resetIndexes();

protected slots:
	void process();

private:
	class KNotesActionPrivate;
	KNotesActionPrivate *fP;
} ;

#endif
