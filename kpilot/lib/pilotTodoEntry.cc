/* pilotTodoEntry.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is a C++ wrapper for the todo-list entry structures.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
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

static const char *pilotTodoEntry_id = "$Id$";
const int PilotTodoEntry::APP_BUFFER_SIZE = 0xffff;


PilotTodoEntry::PilotTodoEntry(struct ToDoAppInfo &appInfo):PilotAppCategory(), fAppInfo(appInfo)
{
	FUNCTIONSETUP;
	::memset(&fTodoInfo, 0, sizeof(struct ToDo));
}

/* initialize the entry from another one. If rec==NULL, this constructor does the same as PilotTodoEntry()
*/
PilotTodoEntry::PilotTodoEntry(struct ToDoAppInfo &appInfo, PilotRecord * rec):PilotAppCategory(rec), fAppInfo(appInfo)
{
	::memset(&fTodoInfo, 0, sizeof(struct ToDo));
	if (rec) 
	{
		unpack_ToDo(&fTodoInfo, (unsigned char *) rec->getData(),
			rec->getLen());
	}
	(void) pilotTodoEntry_id;
}


PilotTodoEntry::PilotTodoEntry(const PilotTodoEntry & e):PilotAppCategory(e), fAppInfo(e.fAppInfo)
{
	FUNCTIONSETUP;
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

bool PilotTodoEntry::setCategory(const char *label)
{
	FUNCTIONSETUP;
	for (int catId = 0; catId < 16; catId++)
	{
		QString aCat = fAppInfo.category.name[catId];

		if (label == aCat)
		{
			setCat(catId);
			return true;
		}
		else
			// if empty, then no more labels; add it 
		if (aCat.isEmpty())
		{
			qstrncpy(fAppInfo.category.name[catId], label, 16);
			setCat(catId);
			return true;
		}
	}
	// if got here, the category slots were full
	return false;
}

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
	if (desc && (::strlen(desc) > 0)) 
	{
		fTodoInfo.description = strdup(desc);
	}
	else
	{
		fTodoInfo.description = 0L;
	}
}

void PilotTodoEntry::setNote(const char *note)
{
	KPILOT_FREE(fTodoInfo.note);
	if (note && (::strlen(note) > 0))
	{
		fTodoInfo.note = strdup(note);
	}
	else
	{
		fTodoInfo.note = 0L;
	}
}
