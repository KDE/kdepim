/* pilotTodoEntry.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is a C++ wrapper for the todo-list entry structures.
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
** Bug reports and questions can be sent to groot@kde.org
*/
#include <stdlib.h>

#ifndef _KDEBUG_H_
#include <kdebug.h>
#endif

#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#include "pilotTodoEntry.h"

static const char *pilotTodoEntry_id =
	"$Id$";


PilotTodoEntry::PilotTodoEntry(void):PilotAppCategory()
{
	::memset(&fTodoInfo, 0, sizeof(struct ToDo));
}


/* initialize the entry from another one. If rec==NULL, this constructor does the same as PilotTodoEntry()
*/
PilotTodoEntry::PilotTodoEntry(PilotRecord * rec):PilotAppCategory(rec)
{
	::memset(&fTodoInfo, 0, sizeof(struct ToDo));
	if (rec) 
	{
		unpack_ToDo(&fTodoInfo, (unsigned char *) rec->getData(),
			rec->getLen());
	}
	(void) pilotTodoEntry_id;
}


PilotTodoEntry::PilotTodoEntry(const PilotTodoEntry & e):PilotAppCategory(e)
{
	::memcpy(&fTodoInfo, &e.fTodoInfo, sizeof(fTodoInfo));
	// See PilotDateEntry::operator = for details
	fTodoInfo.description = 0L;
	fTodoInfo.note = 0L;

	setDescription(e.fTodoInfo.description);
	setNote(e.fTodoInfo.note);

}				// end of copy constructor


PilotTodoEntry & PilotTodoEntry::operator = (const PilotTodoEntry & e)
{
	if (this != &e)
	{
		KPILOT_FREE(fTodoInfo.description);
		KPILOT_FREE(fTodoInfo.note);

		::memcpy(&fTodoInfo, &e.fTodoInfo, sizeof(fTodoInfo));
		// See PilotDateEntry::operator = for details
		fTodoInfo.description = 0L;
		fTodoInfo.note = 0L;

		setDescription(e.fTodoInfo.description);
		setNote(e.fTodoInfo.note);

	}

	return *this;
}				// end of assignment operator

void *PilotTodoEntry::pack(void *buf, int *len)
{
	int i;

	i = pack_ToDo(&fTodoInfo, (unsigned char *) buf, *len);
	*len = i;
	return buf;
}

void PilotTodoEntry::setDescription(const char *desc)
{
	KPILOT_FREE(fTodoInfo.description);
	if (desc)
	{
	  if (::strlen(desc) > 0) {
		fTodoInfo.description = (char *)::malloc(::strlen(desc) + 1);
		if (fTodoInfo.description)
		{
			::strcpy(fTodoInfo.description, desc);
		}
		else
		{
			kdError(LIBPILOTDB_AREA) << __FUNCTION__
				<< ": malloc() failed, description not set"
				<< endl;
		}
	  } else
		fTodoInfo.description = 0L;
	}
	else
	{
		fTodoInfo.description = 0L;
	}
}

void PilotTodoEntry::setNote(const char *note)
{
	KPILOT_FREE(fTodoInfo.note);
	if (note)
	  {
	    if (::strlen(note) > 0) {
		fTodoInfo.note = (char *)::malloc(::strlen(note) + 1);
		if (fTodoInfo.note)
		{
		    ::strcpy(fTodoInfo.note, note);
		}
		else
		{
			kdError(LIBPILOTDB_AREA) << __FUNCTION__
				<< ": malloc() failed, note not set" << endl;
		}
	    } else
	      fTodoInfo.note = 0;
	}
	else
	{
		fTodoInfo.note = 0L;
	}
}



// $Log$
// Revision 1.2  2001/12/28 12:55:24  adridg
// Fixed email addresses; added isBackup() to interface
//
// Revision 1.1  2001/12/27 23:08:30  adridg
// Restored some deleted wrapper files
//
// Revision 1.9  2001/06/05 22:50:56  adridg
// Avoid allocating empty notes and descriptions
//
// Revision 1.8  2001/05/24 10:31:38  adridg
// Philipp Hullmann's extensive memory-leak hunting patches
//
// Revision 1.7  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.6  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.5  2001/02/05 20:58:48  adridg
// Fixed copyright headers for source releases. No code changed
//
