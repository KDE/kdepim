/* KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This is a C++ class dealing with PalmDOC text records
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
#include "options.h"
#include "pilotDOCHead.h"

#include "makedoc9.h"



const int PilotDOCHead::textRecordSize = 4096;

PilotDOCHead::PilotDOCHead():PilotRecordBase(),
version(0),
spare(0), storyLen(0), numRecords(0), recordSize(textRecordSize), position(0)
{
	FUNCTIONSETUP;
}



/* initialize the entry from another one. If rec==NULL, this constructor does the same as PilotDOCHead()
*/
PilotDOCHead::PilotDOCHead(PilotRecord * rec):PilotRecordBase(rec)
{
	const unsigned char *b = (const unsigned char *) rec->data();
	unsigned int offset = 0;

	version = Pilot::dlp<short>::read(b,offset);
	spare = Pilot::dlp<short>::read(b,offset);
	storyLen = Pilot::dlp<long>::read(b,offset);
	numRecords = Pilot::dlp<short>::read(b,offset);
	recordSize = Pilot::dlp<short>::read(b,offset);
	position = Pilot::dlp<long>::read(b,offset);
}


PilotDOCHead::PilotDOCHead(const PilotDOCHead & e):PilotRecordBase(e)
{
	FUNCTIONSETUP;
	*this = e;
}



PilotDOCHead & PilotDOCHead::operator =(const PilotDOCHead & e)
{
	if (this != &e)
	{
		version = e.version;
		spare = e.spare;
		storyLen = e.storyLen;
		numRecords = e.numRecords;
		recordSize = e.recordSize;
		position = e.position;
	}
	return *this;
}




PilotRecord *PilotDOCHead::pack() const
{
	pi_buffer_t *b = pi_buffer_new(16);

	Pilot::dlp<short>::append(b,version);
	Pilot::dlp<short>::append(b,spare);
	Pilot::dlp<long>::append(b,storyLen);
	Pilot::dlp<short>::append(b,numRecords);
	Pilot::dlp<short>::append(b,recordSize);
	Pilot::dlp<long>::append(b,position);

	PilotRecord *rec =  new PilotRecord(b, this);
	return rec;
}

