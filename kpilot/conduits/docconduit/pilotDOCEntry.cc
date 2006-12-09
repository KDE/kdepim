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
#include "pilotDOCEntry.h"



const int PilotDOCEntry::TEXT_SIZE = 4096;


PilotDOCEntry::PilotDOCEntry():PilotRecordBase()
{
	FUNCTIONSETUP;
	compress = false;
}



/* initialize the entry from another one. If rec==NULL, this constructor does the same as PilotDOCEntry()
*/
PilotDOCEntry::PilotDOCEntry(PilotRecord * rec, bool compressed):PilotRecordBase(rec)
{
	if (rec) fText.setText((unsigned char *) rec->data(), rec->size(), compressed);
	compress = compressed;
}



PilotDOCEntry::PilotDOCEntry(const PilotDOCEntry & e):PilotRecordBase(e)
{
	FUNCTIONSETUP;
	// See PilotDateEntry::operator = for details
	fText.setText(e.fText.text(), e.fText.Len(), e.fText.compressed());
	compress = e.compress;
}



PilotDOCEntry & PilotDOCEntry::operator =(const PilotDOCEntry & e)
{
	if (this != &e)
	{
		fText.setText(e.fText.text(), e.fText.Len(), e.fText.compressed());
		compress = e.compress;
	}
	return *this;
}




PilotRecord *PilotDOCEntry::pack()
{
	int len = compress ? fText.Compress() : fText.Decompress();

	if (len<0)
	{
		return 0L;
	}

	pi_buffer_t *b = pi_buffer_new( len + 4 ); // +4 for safety
	memcpy( b->data, (const char *) fText.text(), len );
	b->used = len;
	PilotRecord* rec =  new PilotRecord(b, this);
	return rec;
}
