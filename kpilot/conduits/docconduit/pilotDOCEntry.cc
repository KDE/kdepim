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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"
#include "pilotDOCEntry.h"



static const char *pilotDOCEntry_id =
	"$Id$";
const int PilotDOCEntry::TEXT_SIZE = 4096;


PilotDOCEntry::PilotDOCEntry():PilotAppCategory()
{
	FUNCTIONSETUP;
	compress = false;
}



/* initialize the entry from another one. If rec==NULL, this constructor does the same as PilotDOCEntry()
*/
PilotDOCEntry::PilotDOCEntry(PilotRecord * rec, bool compressed):PilotAppCategory(rec)
{
	if (rec) fText.setText((unsigned char *) rec->getData(), rec->getLen(), compressed);
	compress = compressed;
	(void) pilotDOCEntry_id;
}



PilotDOCEntry::PilotDOCEntry(const PilotDOCEntry & e):PilotAppCategory(e)
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



void *PilotDOCEntry::pack(void *buf, int *len)
{
//      int len;
	if (compress)
	{
		*len = fText.Compress();
	}
	else
	{
		*len = fText.Decompress();
	}
	if (len > 0)
	{
//              char*out=new char[len];
		memcpy(buf, (const char *) fText.text(), *len);
		return buf;
	}
	return 0L;
}


