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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"
#include "pilotDOCBookmark.h"



static const char *pilotDOCBookmark_id =
	"$Id$";



PilotDOCBookmark::PilotDOCBookmark():PilotAppCategory(), pos(0)
{
	FUNCTIONSETUP;
	memset(&bookmarkName[0], 0, 16);
}



/* initialize the entry from another one. If rec==NULL, this constructor does the same as PilotDOCBookmark()
*/
PilotDOCBookmark::PilotDOCBookmark(PilotRecord * rec):PilotAppCategory(rec)
{
	if (rec)
	{
		strncpy(&bookmarkName[0], (char *) rec->getData(), 16);
		bookmarkName[16]='\0';
		pos = get_long(&rec->getData()[16]);// << 8 + (rec->getData())[17];
	}
	(void) pilotDOCBookmark_id;
}



PilotDOCBookmark::
PilotDOCBookmark(const PilotDOCBookmark & e):PilotAppCategory(e)
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



void *PilotDOCBookmark::pack(void *buf, int *len)
{
	char *tmp = (char *) buf;

//  buf=malloc(16*sizeof(char)+sizeof(long int));
	strncpy(tmp, &bookmarkName[0], 16);
	//*(long int *) (tmp + 16) = pos;
	set_long(tmp + 16, pos);
	*len = 20;
	return buf;
}

