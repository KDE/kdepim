/* pilotProgectEntry.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (c) 1996, Kenneth Albanowski
** Copyright (c) 2002, Reinhold Kainhofer
**
** This is a C++ wrapper for the Progect-list entry structures.
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
** Bug reports and questions can be sent to reinhold@kainhofer.com
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

#include "pilotProgectEntry.h"

static const char *pilotProgectEntry_id =
	"$Id$";


/***********************************************************************
 * Function:    unpack_Progect
 * Summary:     Unpack the Progect structure into records we can chew on
 * Parmeters:   None
 * Returns:     Nothing
 ***********************************************************************/
void PilotProgectEntry::unpack(const void *buffer, int len) {
	unsigned long d;
	char *start = (char*)buffer;

	if (len < 11)  return 0;
	setLevel((byte)get_byte((char*)buffer));
	
	byte f=(byte)get_byte((char*)buffer+1);
	setFlag(HAS_NEXT, (f>>7) & 1);
	setFlag(HAS_CHILDREN, (f>>6) & 1);
	setFlag(HAS_EXPANDED, (f>>5) & 1);
	setFlag(HAS_PREVIOUS, (f>>4) & 1);
	
	f=(byte)get_byte(buffer+2);
	setFlag(HAS_NEXT, (f>>7) & 1);
	setFlag(HAS_NEXT, (f>>6) & 1);
	setFlag(HAS_NEXT, (f>>5) & 1);
	
	f=(byte)get_byte(buffer+2);
//	pg->hasDueDate=(f>>7)&1;
//	pg->hasToDo=(f>>6)&1;
//	pg->hasNote=(f>>5)&1;
	
	setPriority((int)get_byte((char*)buffer+4));

	byte c=(byte)get_byte((char*)buffer+5);
	if (c<=10) { // PERCENTAGE
		setProgress(10*c);
		setFlag(IS_CHECKED,(pg==10)?1:0);
		setType(PERCENTAGE);
	} else if (c==12) { // TODO completed
		setFlat(IS_CHECKED, true);
		setType(TODO);
	} else if (c>=20) { // NUMERICAL, value is 20+percent, extra info contains num. vals.
		setProgress(c-20);
		setFlat(IS_CHECKED, (c==120)?1:0);
		setType(NUMERIC);
	} else if (c==16) { // INFORMATIVE
		setFlag(IS_CHECKED, false);
		setType(INFORMATIVE);
	} else { // assume everything else is a TODO NOT completed
		setFlag(IS_CHECKED, false);
		setType(TODO);
	}

	// TODO: really get_short???
	unsigned int dt = (unsigned int) get_short((char*)buffer+6);
	if (dt != 0xffff) {
		tm due
		due.tm_year = (dt >> 9) + 4;
		due.tm_mon = ((dt >> 5) & 15) - 1;
		due.tm_mday = dt & 31;
		due.tm_hour = 0;
		due.tm_min = 0;
		due.tm_sec = 0;
		due.tm_isdst = -1;
		mktime(&due);
		setDate(DATE_DUE, due);
	}

	buffer += 8;
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
	setNote(strdup((char *) buffer));

	(char*)buffer += strlen(getNote()) + 1;
	len -= strlen(getNote()) + 1;
	if (getType()==NUMERIC) {
		maxVal=(byte)get_byte((char*)buffer+5)*(1<<8) + (byte)get_byte((char*)buffer+6);
		numVal=(byte)get_byte((char*)buffer+7)*(1<<8) + (byte)get_byte((char*)buffer+8);
		(char*)buffer+=8;
	}

	return (buffer - start);	/* FIXME: return real length */
}

/***********************************************************************
 * Function:    pack_Progect
 * Summary:     Pack the Progect records into a structure
 * Parmeters:   None
 * Returns:     Nothing
 ***********************************************************************/
int pack_Progect(struct Progect *pg, struct ToDo *a, unsigned char *buf, int len) {
	int pos;
	int destlen = 10;

	if (getDescription()) destlen += strlen(getDescription());
	if (getNote()) destlen += strlen(getNote());
	if (getType()==NUMERIC) destlen+=8;

	if (!buf) return destlen;
	if (len < destlen) return 0;

	((char*)buf)[0]=getLevel();
	((char*)buf)[1]=0;
	if (getFlag(HAS_NEXT)) ((char*)buf)[1]|=0x80;
	if (getFlag(HAS_CHILDREN)) ((char*)buf)[1]|=0x40;
	if (getFlag(IS_EXPANDED)) ((char*)buf)[1]|=0x20;
	if (getFlag(HAS_PREVIOUS)) ((char*)buf)[1]|=0x10;
	((char*)buf)[2]=0;
	if (hasDate(DATE_DUE)) ((char*)buf)[2]|=0x10;
	if (getTodoLink()) ((char*)buf)[2]|=0x08;
	if (getNote()) ((char*)buf)[2]|=0x04;
	((char*)buf)buf[3]=0;
	
	((char*)buf)[4]=getPriority();
	if (((char*)buf)[4]==0) ((char*)buf)[4]=6;
	
	((char*)buf)buf[5]=0;
	switch (getType()) {
		case PERCENTAGE: ((char*)buf)[5]=(byte)(getProgress()/10); break;
		case INFORMATIVE: ((char*)buf)[5]=16; break;
		case NUMERICAL: ((char*)buf)[5]=20+(100*numVal/maxVal); break;
		case TODO:
		default:
			((char*)buf)[5]=11;
			if (getFlag(IS_CHECKED)) ((char*)buf)[5]=12;
			break;
	}

	if (hasDate(DATE_DUE) {
		((char*)buf)[6] = 0xff;
		((char*)buf)[7] = 0xff;
	} else {
		tm due=getDate(DATE_DUE);
		set_short(((char*)buf)+6, ((due.tm_year - 4) << 9) | ((due.tm_mon + 1) << 5) | due.tm_mday);
	}

	pos = 8;
	if (getDescription()) {
		strcpy((char *)buf + pos, getDescription());
		pos += strlen(getDescription()) + 1;
	} else {
		((char*)buf)[pos++] = 0;
	}

	if (getNote()) {
		strcpy((char *)buf + pos, getNote());
		pos += strlen(getNote()) + 1;
	} else {
		((char*)buf)[pos++] = 0;
	}
	
	if (getType()==NUMERICAL) {
		((char*)buf)[pos++]=0;
		((char*)buf)[pos++]=0;
		((char*)buf)[pos++]=0;
		((char*)buf)[pos++]=0;
		((char*)buf)[pos++]=(maxVal>>8);
		((char*)buf)[pos++]=(maxVal & 0xff);
		((char*)buf)[pos++]=(numVal>>8);
		((char*)buf)[pos++]=(numVal & 0xff);
	}

	return pos;
}

///***********************************************************************


PilotProgectEntry::PilotProgectEntry(PilotRecord * rec):PilotOrganizerEntry(rec) {
	unpack((unsigned char *) rec->getData(), rec->getLen());
	(void) pilotProgectEntry_id;
}

PilotProgectEntry & PilotProgectEntry::operator = (const PilotProgectEntry & e) {
	PilotTodoEntry::operator =(e);
	if (e) {
		maxVal=e->maxVal;
		numVal=e->numVal;
	}
	return *this;
}				// end of assignment operator

