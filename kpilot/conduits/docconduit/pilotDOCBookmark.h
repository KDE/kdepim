/* pilotDOCBookmark.h	-*- C++ -*-		KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** See the .cc file for an explanation of what this file is for.
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
#ifndef _KPILOT_PILOTDOCBOOKMARK_H
#define _KPILOT_PILOTDOCBOOKMARK_H

#include <string.h>

#ifndef _KPILOT_PILOTAPPCATEGORY_H
#include "pilotAppCategory.h"
#endif

#ifndef _KPILOT_PILOTRECORD_H
#include "pilotRecord.h"
#endif



class PilotDOCBookmark:public PilotAppCategory {
public:
	PilotDOCBookmark();
	PilotDOCBookmark(PilotRecord * rec);
	PilotDOCBookmark(const PilotDOCBookmark & e);
	~PilotDOCBookmark() {
	} PilotDOCBookmark & operator=(const PilotDOCBookmark & e);
	PilotRecord *pack() {
		return PilotAppCategory::pack();
	}

protected:
	void *pack(void *, int *);
	void unpack(const void *, int = 0) {}

// private:
public:
	char bookmarkName[17];
	long int pos;
};




#else
#ifdef DEBUG
#warning "File doubly included"
#endif
#endif


// $Log$
//
