/* pilotDateEntry.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is a C++ wrapper for the Pilot's datebook structures.
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

#include "options.h"

#include <stdlib.h>

#include <qtextcodec.h>

#ifndef _KDEBUG_H_
#include <kdebug.h>
#endif

#include "pilotDateEntry.h"

static const char *pilotDateEntry_id = "$Id$";


PilotDateEntry::PilotDateEntry(void):PilotAppCategory()
{
	::memset(&fAppointmentInfo, 0, sizeof(struct Appointment));
}

/* initialize the entry from another one. If rec==NULL, this constructor does the same as PilotDateEntry()
*/
PilotDateEntry::PilotDateEntry(PilotRecord * rec):PilotAppCategory(rec)
{
	::memset(&fAppointmentInfo, 0, sizeof(fAppointmentInfo));
	if (rec)
	{
		 unpack_Appointment(&fAppointmentInfo,
			(unsigned char *) rec->getData(), rec->getLen());
	}
	return;

	/* NOTREACHED */
	/* Included to avoid warning that id isn't used. */
	(void) pilotDateEntry_id;
}

void PilotDateEntry::_copyExceptions(const PilotDateEntry & e)
{
	if (e.fAppointmentInfo.exceptions > 0)
	{
		size_t blocksize = e.fAppointmentInfo.exceptions * 
			sizeof(struct tm);

		fAppointmentInfo.exception = (struct tm *)::malloc(blocksize);

		if (fAppointmentInfo.exception)
		{
			fAppointmentInfo.exceptions = 
				e.fAppointmentInfo.exceptions;
			::memcpy(fAppointmentInfo.exception,
				e.fAppointmentInfo.exception, blocksize);
		}
		else
		{
			kdError(LIBPILOTDB_AREA) << __FUNCTION__
				<< ": malloc() failed, exceptions not copied"
				<< endl;
			fAppointmentInfo.exceptions = 0;
		}
	}
	else
	{
		fAppointmentInfo.exceptions = 0;
		fAppointmentInfo.exception = 0L;
	}
}


PilotDateEntry::PilotDateEntry(const PilotDateEntry & e):PilotAppCategory(e)
{
	::memcpy(&fAppointmentInfo, &e.fAppointmentInfo,
		sizeof(struct Appointment));
	// See operator = for explanation
	fAppointmentInfo.exception = 0L;
	fAppointmentInfo.description = 0L;
	fAppointmentInfo.note = 0L;

	_copyExceptions(e);
	setDescriptionP(e.fAppointmentInfo.description);
	setNoteP(e.fAppointmentInfo.note);
}


PilotDateEntry & PilotDateEntry::operator = (const PilotDateEntry & e)
{
	if (this != &e)		// Pointer equality!
	{
		KPILOT_FREE(fAppointmentInfo.exception);
		KPILOT_FREE(fAppointmentInfo.description);
		KPILOT_FREE(fAppointmentInfo.note);
		::memcpy(&fAppointmentInfo, &e.fAppointmentInfo,
			sizeof(fAppointmentInfo));

		// The original pointers were already freed; since we're now 
		// got the pointers from the new structure and we're going
		// to use the standard set functions make sure that
		// we don't free() the copies-of-pointers from e, which
		// would be disastrous.
		//
		//
		fAppointmentInfo.exception = 0L;
		fAppointmentInfo.description = 0L;
		fAppointmentInfo.note = 0L;

		_copyExceptions(e);
		setDescriptionP(e.fAppointmentInfo.description);
		setNoteP(e.fAppointmentInfo.note);
	}

	return *this;
}				// end of assignment operator


void *PilotDateEntry::pack(void *buf, int *len)
{
	int i;

	i = pack_Appointment(&fAppointmentInfo, (unsigned char *) buf, *len);
	*len = i;
	return buf;
}

/* setExceptions sets a new set of exceptions. Note that 
	PilotDateEntry assumes ownership of the array and will
	delete the old one. */
void PilotDateEntry::setExceptions(struct tm *e) {
	if (fAppointmentInfo.exception != e)
	{
		KPILOT_FREE(fAppointmentInfo.exception);
	}
	fAppointmentInfo.exception=e;
}


void PilotDateEntry::setDescriptionP(const char *desc, int l)
{
	FUNCTIONSETUP;
	KPILOT_FREE(fAppointmentInfo.description);

	if (desc && *desc)
	{
		if (-1 == l) l=::strlen(desc);
		fAppointmentInfo.description =
			(char *) ::malloc(l + 1);
		if (fAppointmentInfo.description)
		{
			::strcpy(fAppointmentInfo.description, desc);
		}
		else
		{
			kdError(LIBPILOTDB_AREA) << __FUNCTION__
				<< ": malloc() failed, description not set"
				<< endl;
		}
	}
	else
	{
		fAppointmentInfo.description = 0L;
	}
}

void PilotDateEntry::setNoteP(const char *note, int l)
{
	FUNCTIONSETUP;
	KPILOT_FREE(fAppointmentInfo.note);

	if (note && *note)
	{
		if (-1 == l) l=::strlen(note);
		fAppointmentInfo.note = (char *)::malloc(l + 1);
		if (fAppointmentInfo.note)
		{
			strcpy(fAppointmentInfo.note, note);
		}
		else
		{
			kdError(LIBPILOTDB_AREA) << __FUNCTION__
				<< ": malloc() failed, note not set" << endl;
		}
	}
	else
	{
		fAppointmentInfo.note = 0L;
	}
}

void PilotDateEntry::setNote(const QString &s)
{
	setNoteP(codec()->fromUnicode(s),s.length());
}

void PilotDateEntry::setDescription(const QString &s)
{
	setDescriptionP(codec()->fromUnicode(s),s.length());
}

QString PilotDateEntry::getNote() const
{
	return codec()->toUnicode(getNoteP());
}

QString PilotDateEntry::getDescription() const
{
	return codec()->toUnicode(getDescriptionP());
}

