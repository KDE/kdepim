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
#include "pilotDOCHead.h"

#include "makedoc9.h"



static const char *pilotDOCHead_id =
	"$Id$";
const int PilotDOCHead::textRecordSize = 4096;



PilotDOCHead::PilotDOCHead():PilotAppCategory(),
version(0),
spare(0), storyLen(0), numRecords(0), recordSize(textRecordSize), position(0)
{
	FUNCTIONSETUP;
	(void) pilotDOCHead_id;
}



/* initialize the entry from another one. If rec==NULL, this constructor does the same as PilotDOCHead()
*/
PilotDOCHead::PilotDOCHead(PilotRecord * rec):PilotAppCategory(rec)
{
	unpack((const void *) rec->getData());
}


PilotDOCHead::PilotDOCHead(const PilotDOCHead & e):PilotAppCategory(e)
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



void *PilotDOCHead::pack(void *buf, int *len)
{
	char *tmp = (char *) buf;

	*len = 16;
	set_short(tmp, version);
	tmp+=2;
	set_short(tmp, spare);
	tmp+=2;
	set_long(tmp, storyLen);
	tmp+=4;
	set_short(tmp, numRecords);
	tmp+=2;
	set_short(tmp, recordSize);
	tmp+=2;
	set_short(tmp, position);
	return buf;
}


void PilotDOCHead::unpack(const void *buf, int)
{
	char *tmp = (char *) buf;

	version = get_short(tmp);
	tmp+=2;
	spare = get_short(tmp);
	tmp+=2;
	storyLen = get_long(tmp);
	tmp+=4;
	numRecords = get_short(tmp);
	tmp+=2;
	recordSize = get_short(tmp);
	tmp+=2;
	position = get_long(tmp);
}

