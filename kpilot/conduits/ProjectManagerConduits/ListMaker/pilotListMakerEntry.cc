/* pilotListMakerEntry.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (c) 1996, Kenneth Albanowski
** Copyright (c) 2002, Reinhold Kainhofer
**
** This is a C++ wrapper for the ListMaker-list entry structures.
** it is based on the pilotToDoEntry.cc by Dan Pilone,
** the pack/unpack functions are based on pilot-link.
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

//#include <pi-source.h>
//#include <pi-dlp.h>
//#include <pi-todo.h>

#ifndef _KDEBUG_H_
#include <kdebug.h>
#endif

#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#include "pilotListMakerEntry.h"

static const char *pilotListMakerEntry_id =
	"$Id$";


/***********************************************************************
 * Function:    unpack_ListMaker
 * Summary:     Unpack the ListMaker structure into records we can chew on
 * Parmeters:   None
 * Returns:     Nothing
 ***********************************************************************/
void PilotListMakerEntry::unpack(const void *buffer, int len) {
	unsigned long d;
	char *start = (char*)buffer;

	/* Each record had the following byte structure:
	   XXXX | FF DD | TTTTT...  0 NNNN... 0 CC... 0 XX
	
	   The X denote an unknown byte, | does not have a meaning.
	   FF is a bit flag with the bits (highest bit first) having the following meaning:
	      1.. the item is checked (1)
	      2.. if the 3rd bit is set (meaning it is a subgroup) the group is open (1) / closed (0)
	      3.. the item has subitems (1)
	      4 .. the item is visible (i.e. not a subitem of a closed group)
	      5.. unknown (set if item or subitem is checked)
	      6 .. " normal item", i.e. not a common item
	      7.. unknown
	      8.. the item is a common item (1)
	      9-13.. unknown
	      14-16 .. level of the item
	   DD is the due date of the item (or 0xff, if no date is set)
	   TTTTT is the text of the item
	   NNNN is the text of the note
	   CCC is the custom field
	*/
	
	if (len < 11)  return;// 0;
	char f = (char) get_byte((char*)buffer+4);
	setFlag(IS_CHECKED, (f>>7) & 1);
	setFlag(IS_EXPANDED, (f>>6) & 1);
	setFlag(HAS_CHILDREN, (f>>5) & 1);
	setFlag(IS_VISIBLE, (f>>4) & 1);
	setFlag(FLAG_CUSTOM1, f & 1);
	f=(char) get_byte((char*)buffer+5);
	setLevel(f & 7);
	
	d = (unsigned short int) get_short((char*)buffer+6);
	setDate(DATE_DUE, d);

	setPriority(0);

	(char*)buffer += 8;
	len -= 8;

	if (len < 1) return;
	setDescription(strdup((char *) buffer));

	(char*)buffer += strlen(getDescription()) + 1;
	len -= strlen(getDescription()) + 1;

	if (len < 1) {
//		free(fData.description);
//		fData.description = 0;
		return;
	}
	setNote( strdup((char *) buffer) );

	(char*)buffer += strlen(getNote()) + 1;
	len -= strlen(getNote()) + 1;

//	return (buffer - start);	/* FIXME: return real length */
}

/***********************************************************************
 * Function:    pack_ListMaker
 * Summary:     Pack the ListMaker records into a structure
 * Parmeters:   None
 * Returns:     Nothing
 ***********************************************************************/
void *PilotListMakerEntry::pack(void *buf, int *len) {
	int pos;
	int destlen = 13;

	if (getDescription()) destlen += strlen(getDescription());
	if (getNote()) destlen += strlen(getNote());

	if (!buf) return NULL;// destlen;
	if (*len < destlen) return NULL;// 0;

	((char*)buf)[0]=0;
	((char*)buf)[1]=0;
	((char*)buf)[2]=0;
	((char*)buf)[3]=0;
	((char*)buf)[4]=0;
	if (getFlag(IS_CHECKED)) ((char*)buf)[4]|=0x80;
	if (getFlag(IS_EXPANDED)) ((char*)buf)[4]|=0x40;
	if (getFlag(HAS_CHILDREN)) ((char*)buf)[4]|=0x20;
	if (getFlag(IS_VISIBLE)) ((char*)buf)[4]|=0x10;
//	if(lm->normal) buf[4]|=0x04;
	if (getFlag(FLAG_CUSTOM1)) ((char*)buf)[4]|=0x01; else ((char*)buf)[4]|=0x04;
	((char*)buf)[5]=getLevel();
	
	if (hasDate(DATE_DUE)) {
		QDate due=getDate(DATE_DUE).date();
		set_short((char*)buf+6, ((due.year() - 4) << 9) | ((due.month() + 1) << 5) | due.day());
	} else {
		((char*)buf)[6] = 0xff;
		((char*)buf)[7] = 0xff;
	}
	pos = 8;
	if (getDescription()) {
		strcpy((char *) buf + pos, getDescription());
		pos += strlen(getDescription()) + 1;
	} else {
		((char*)buf)[pos++] = 0;
	}

	if (getNote()) {
		strcpy((char *) buf + pos, getNote());
		pos += strlen(getNote()) + 1;
	} else {
		((char*)buf)[pos++] = 0;
	}
	((char*)buf)[pos++]=0;
	// don't know what the last two bytes are, so just set them 0
	((char*)buf)[pos++]=0;
	((char*)buf)[pos++]=0;
	*len=pos;
	return &pos;
}


PilotListMakerEntry::PilotListMakerEntry(PilotRecord * rec):PilotOrganizerEntry(rec) {
	unpack((unsigned char *) rec->getData(), rec->getLen());
	(void) pilotListMakerEntry_id;
}

