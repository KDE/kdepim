/* pilotOrganizerEntry.cc			KPilot
**
** Copyright (c) 2002, Reinhold Kainhofer
**
** This is a C++ wrapper for the Organizer entry structures.
** it is based on the pilotToDoEntry.cc by Dan Pilone,
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

#include <pi-source.h>
#include <pi-dlp.h>
#include <pi-todo.h>

#ifndef _KDEBUG_H_
#include <kdebug.h>
#endif

#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#include "pilotOrganizerEntry.h"
#include <calendar.h>

using namespace KCal;


static const char *pilotOrganizerEntry_id =
	"$Id$";

PilotOrganizerEntry::PilotOrganizerEntry(void):PilotAppCategory() {
	::memset(&fData, 0, sizeof(OrganizerEntry));
}


PilotOrganizerEntry::PilotOrganizerEntry(PilotRecord * rec):PilotAppCategory(rec) {
//	unpack( (unsigned char *) rec->getData(), rec->getLen());
	(void) pilotOrganizerEntry_id;
}


PilotOrganizerEntry::PilotOrganizerEntry(const PilotOrganizerEntry & e):PilotAppCategory(e) {
	::memcpy(&fData, &e.fData, sizeof fData );
}				// end of copy constructor

PilotOrganizerEntry::PilotOrganizerEntry(KCal::Todo*todo):PilotAppCategory() {
	// TODO: assign values
}

PilotOrganizerEntry& PilotOrganizerEntry::operator=(const KCal::Todo &todo) {
	// TODO: set this entry from the todo
}
KCal::Todo* PilotOrganizerEntry::getTodo(){
	KCal::Todo*todo=new KCal::Todo();
	// TODO: now set the todo entry from the this entry
}


PilotOrganizerEntry & PilotOrganizerEntry::operator = (const PilotOrganizerEntry & e) {
	if (this != &e) {
		PilotAppCategory::operator =(e);
		::memcpy(&fData, &e.fData, sizeof(fData));
	}
	return *this;
}				// end of assignment operator

void PilotOrganizerEntry::setDate(const po_date_type tp, unsigned short int d) {
	QTime tm;
	if (d == 0xffff) { QDate dt; fData.dates[tp].setDate(dt); fData.dates[tp].setTime(tm); return;}
	QDate dt( (d >> 9) + 4, ((d >> 5) & 15) - 1, d & 31);
		// TODO: possible buffer overflow if tp>=4...
	fData.dates[tp].setDate(dt);
	fData.dates[tp].setTime(tm);
};

void PilotOrganizerEntry::setDescription(const char *desc) {
	KPILOT_FREE(fData.descr);
	if (desc) {
		if (::strlen(desc) > 0) {
			fData.descr = (char *)::malloc(::strlen(desc) + 1);
			if (fData.descr) {
				::strcpy(fData.descr, desc);
			} else {
				kdError(LIBPILOTDB_AREA) << __FUNCTION__
					<< ": malloc() failed, description not set"
					<< endl;
			}
		} else fData.descr = 0L;
	}
	else {
		fData.descr = 0L;
	}
}

void PilotOrganizerEntry::setNote(const char *note) {
	KPILOT_FREE(fData.note);
	if (note) {
		if (::strlen(note) > 0) {
			fData.note = (char *)::malloc(::strlen(note) + 1);
			if (fData.note) {
				::strcpy(fData.note, note);
			} else {
				kdError(LIBPILOTDB_AREA) << __FUNCTION__
					<< ": malloc() failed, note not set" << endl;
			}
		} else fData.note = 0;
	}
	else
	{
		fData.note = 0L;
	}
}

void PilotOrganizerEntry::setCustStr(const char *note) {
	KPILOT_FREE(fData.customstr);
	if (note) {
		if (::strlen(note) > 0) {
			fData.customstr = (char *)::malloc(::strlen(note) + 1);
			if (fData.customstr) {
				::strcpy(fData.customstr, note);
			} else {
				kdError(LIBPILOTDB_AREA) << __FUNCTION__
					<< ": malloc() failed, note not set" << endl;
			}
		} else fData.customstr = 0;
	}
	else
	{
		fData.customstr = 0L;
	}
}

void PilotOrganizerEntry::free_OrganizerEntry(OrganizerEntry*entry) {
	if (entry->descr) free(entry->descr);
	if (entry->note) free(entry->note);
	if (entry->customstr) free(entry->customstr);
}

