/* KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This is a C++ class for the DOC bookmark record structure
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
#include "pilotDOCBookmark.h"



PilotDOCBookmark::PilotDOCBookmark():PilotRecordBase(), pos(0)
{
	FUNCTIONSETUP;
	memset(&bookmarkName[0], 0, 16);
}



/* initialize the entry from another one. If rec==NULL, this constructor does the same as PilotDOCBookmark()
*/
PilotDOCBookmark::PilotDOCBookmark(PilotRecord * rec):PilotRecordBase(rec)
{
	if (rec)
	{
		const pi_buffer_t *b = rec->buffer();
		unsigned int offset = 0;
		Pilot::dlp<char *>::read(b,offset,bookmarkName,16);
		bookmarkName[16]='\0';
		pos = Pilot::dlp<long>::read(b,offset);
	}
}



PilotDOCBookmark::PilotDOCBookmark(const PilotDOCBookmark & e):PilotRecordBase(e)
{
	FUNCTIONSETUP;
	*this = e;
}



PilotDOCBookmark & PilotDOCBookmark::operator =(const PilotDOCBookmark & e)
{
	if (this != &e)
	{
		strncpy(&bookmarkName[0], &e.bookmarkName[0], 16);
		bookmarkName[16]='\0';
		pos = e.pos;
	}
	return *this;
}



PilotRecord *PilotDOCBookmark::pack() const
{
	pi_buffer_t *b = pi_buffer_new( 16 + Pilot::dlp<long>::size );
	pi_buffer_append(b, bookmarkName, 16);
	b->data[16] = 0;
	Pilot::dlp<long>::append(b,pos);
	PilotRecord* rec =  new PilotRecord(b, this);
	return rec;
}
